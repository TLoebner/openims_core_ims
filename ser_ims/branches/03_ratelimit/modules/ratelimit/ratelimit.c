/*
 * $Id$
 *
 * ratelimit module
 *
 * Copyright (C) 2006 Hendrik Scholz <hscholz@raisdorf.net>
 *
 * This file is part of ser, a free SIP server.
 *
 * ser is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * For a license to use the ser software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact iptel.org by e-mail at the following addresses:
 *    info@iptel.org
 *
 * ser is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * TODO
 * d move to ratelimit 
 * d rpc fns for static limits
 * - FIFO interface (this will have to wait until unixsock module compiles :-D )
 * - locking 
 * - long long's instead of doubles in get_cpuload
 * - rpc stats
 * d rpc params
 * - remove act_busy
 * - split patches into general modparams + adaptive + rpc
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <math.h>

#include "../../mem/mem.h"
#include "../../mem/shm_mem.h"
#include "../../sr_module.h"
#include "../../dprint.h"
#include "../../timer.h"
#include "../../ut.h"

#include "../sl/sl_funcs.h"
#include "../../data_lump_rpl.h"
#include "../../mem/shm_mem.h"
#include "../../parser/msg_parser.h"

MODULE_VERSION

#define MAX_PIPES	10
#define MAX_QUEUES	10

/* timer interval length in seconds, tunable via modparam */
#define RL_TIMER_INTERVAL 10

#ifndef rpc_lf
#define rpc_lf(rpc, c)  rpc->add(c, "s","")
#endif

int (*sl_reply)(struct sip_msg* _msg, char* _str1, char* _str2); 

static int timer_interval = RL_TIMER_INTERVAL;

/* PIPE_ALGO_FEEDBACK holds cpu usage to a fixed value using 
 * negative feedback according to the PID controller model
 *
 * <http://en.wikipedia.org/wiki/PID_controller>
 */
enum {
	PIPE_ALGO_NOP = 0,
	PIPE_ALGO_RED,
	PIPE_ALGO_TAILDROP,
	PIPE_ALGO_FEEDBACK,		
};

static struct {
	char *	str;
	int		algo;
} algos[] = {
	{"NOP",			PIPE_ALGO_NOP},
	{"RED",			PIPE_ALGO_RED},
	{"TAILDROP",	PIPE_ALGO_TAILDROP},
	{"FEEDBACK",	PIPE_ALGO_FEEDBACK},
};

#define ALGO_SIZE	sizeof(algos) / sizeof(algos[0])

typedef struct pipe {
	/* stuff that gets read as a modparam or set via rpc */
	int *	algo;
	int		algo_mp;
	int *	limit;
	int		limit_mp;

	/* updated values */
	int *	counter;
	int *	load;
} pipe_t;

typedef struct queue {
	int	*	pipe;
	int		pipe_mp;
	str	*	method;
	str		method_mp;
} queue_t;

static double * cpu_load;	/* actual cpu_load, used by PIPE_ALGO_FEEDBACK */
static int cfg_setpoint;	/* desired cpu_load */
static double * pid_kp, * pid_ki, * pid_kd, * pid_setpoint; /* PID tuning params */
static int * drop_rate;		/* updated by PIPE_ALGO_FEEDBACK */

static pipe_t pipes[MAX_PIPES];
static queue_t queues[MAX_QUEUES];

static int nqueues_mp = 0;
static int * nqueues;
static int params_inited = 0;
static regex_t  pipe_params_regex;
static regex_t queue_params_regex;

/** module functions */
static int mod_init(void);
static int child_init(int);
static void timer(unsigned int, void *);
static int rl_check(struct sip_msg*, char *, char *);
static int w_rl_drop(struct sip_msg*, char *, char *);
static int fixup_rl_drop(void **, int);
static int act_busy(struct sip_msg*, char *, char *); 
static int add_queue_params(modparam_t, void *);
static int add_pipe_params(modparam_t, void *);
void destroy(void);

static rpc_export_t rpc_methods[];

static cmd_export_t cmds[]={
	{"rl_check", rl_check, 0, 0, REQUEST_ROUTE},
	{"rl_drop", w_rl_drop, 0, fixup_rl_drop, REQUEST_ROUTE},
	{"rl_drop", w_rl_drop, 1, fixup_rl_drop, REQUEST_ROUTE},
	{"rl_drop", w_rl_drop, 2, fixup_rl_drop, REQUEST_ROUTE},
	{"act_busy", act_busy, 0, 0, REQUEST_ROUTE},
	{0,0,0,0,0}
};

static param_export_t params[]={
	{"timer_interval",	PARAM_INT, &timer_interval},
	{"queue",			PARAM_STRING|PARAM_USE_FUNC, (void *)add_queue_params},
	{"pipe",			PARAM_STRING|PARAM_USE_FUNC, (void *)add_pipe_params},
	{0,0,0}
};

/** module exports */
struct module_exports exports= {
	"ratelimit",
	cmds,
	rpc_methods,
	params,	
	mod_init,   /* module initialization function */
	(response_function) 0,
	(destroy_function) destroy,
	0,
	child_init  /* per-child init function */
};

/* converts an algorithm id to a name
 * \return	0 if found, -1 otherwise
 * */
static int algo_to_str(int algo, char ** ret)
{
	int i;

	for (i=0; i<ALGO_SIZE; i++)
		if (algos[i].algo == algo) {
			*ret = algos[i].str;
			return 0;
		}
	LOG(L_ERR, "ratelimit algo %d not known\n", algo);
	return -1;
}

/**
 * converts an algorithm string to an id
 * \return	0 if found, -1 otherwise
 */
static int str_to_algo(int len, char * str, int * ret)
{
	int i;

	for (i=0; i<ALGO_SIZE; i++)
		if (len == strlen(algos[i].str) && ! strncasecmp(str, algos[i].str, len)) {
			*ret = algos[i].algo;
			return 0;
		}
	LOG(L_ERR, "ratelimit algo '%.*s' not known\n", len, str);
	return -1;
}

/**
 * strcpy for str's (does not allocate the str structure but only the .s member)
 * \return	0 if succeeded, -1 otherwise
 */
static int str_cpy(str * dest, str * src)
{
	dest->len = src->len;
	dest->s = shm_malloc(src->len);
	if (! dest->s)
		return -1;

	memcpy(dest->s, src->s, src->len);
	return 0;
}

/* not using /proc/loadavg because it only works when our_timer_interval == theirs */
static int get_cpuload(double * load)
{
	static 
	double o_user, o_nice, o_sys, o_idle, o_iow, o_irq, o_sirq, o_stl;
	double n_user, n_nice, n_sys, n_idle, n_iow, n_irq, n_sirq, n_stl;
	static int first_time = 1;
	FILE * f = fopen("/proc/stat", "r");

	if (! f) return -1;
	fscanf(f, "cpu  %lf%lf%lf%lf%lf%lf%lf%lf",
			&n_user, &n_nice, &n_sys, &n_idle, &n_iow, &n_irq, &n_sirq, &n_stl);
	fclose(f);

	if (first_time) {
		first_time = 0;
		*load = 0;
	} else {		
		double d_total =(n_user	- o_user)	+ 
						(n_nice	- o_nice)	+ 
						(n_sys	- o_sys)	+ 
						(n_idle	- o_idle)	+ 
						(n_iow	- o_iow)	+ 
						(n_irq	- o_irq)	+ 
						(n_sirq	- o_sirq)	+ 
						(n_stl	- o_stl);
		double d_idle = (n_idle - o_idle);

		*load = 1.0 - d_idle / d_total;
	}

	o_user	= n_user; 
	o_nice	= n_nice; 
	o_sys	= n_sys; 
	o_idle	= n_idle; 
	o_iow	= n_iow; 
	o_irq	= n_irq; 
	o_sirq	= n_sirq; 
	o_stl	= n_stl;
	
	LOG(L_DBG, "load is %lf\n", *load);
	return 0;
}

static double int_err = 0.0;
static double last_err = 0.0;

static void update_load()
{
	static char spcs[51];
	int load;
	double err, dif_err, output;

	if (get_cpuload(cpu_load)) 
		return;

	/* PID update */
	err = *pid_setpoint - *cpu_load;
	dif_err = err - last_err;

	/*
	 * TODO?: the 'if' is needed so low cpu loads for 
	 * long periods (which can't be compensated by 
	 * negative drop rates) don't confuse the controller
	 *
	 * NB: - "err < 0" means "desired_cpuload < actual_cpuload"
	 *     - int_err is integral(err) over time
	 */
	if (int_err < 0 || err < 0)
		int_err += err;

	output =	(*pid_kp) * err + 
				(*pid_ki) * int_err + 
				(*pid_kd) * dif_err;
	last_err = err;

	*drop_rate = (output > 0) ? output  : 0;

	load = 0.5 + 100.0 * *cpu_load; /* round instead of floor */
	memset(spcs, '-', load / 4);
	spcs[load / 4] = 0;

	LOG(L_INFO, "p=% 6.2lf i=% 6.2lf d=% 6.2lf o=% 6.2lf %s|%d%%\n",
			err, int_err, dif_err, output, 
			spcs, load);
}

/* initialize ratelimit module */
static int mod_init(void)
{
	int i;

	DBG("RATELIMIT: initializing ...\n");

	/* register timer to reset counters */
	if (register_timer(timer, 0, timer_interval) < 0) {
		LOG(L_ERR, "RATELIMIT:ERROR: could not register timer function\n");
		return -1;
	}

	sl_reply = find_export("sl_send_reply", 2, 0);
	if (! sl_reply) {
		LOG(L_ERR, "ratelimit: This module requires the sl module\n");
		return -1;
	}

	cpu_load	= shm_malloc(sizeof(double));
	pid_kp		= shm_malloc(sizeof(double));
	pid_ki		= shm_malloc(sizeof(double));
	pid_kd		= shm_malloc(sizeof(double));
	pid_setpoint= shm_malloc(sizeof(double));
	drop_rate	= shm_malloc(sizeof(int));
	nqueues     = shm_malloc(sizeof(int));

	*cpu_load 	= 0.0;
	*pid_kp = 0.0;
	*pid_ki = -25.0;
	*pid_kd = 0.0;
	*pid_setpoint = 0.01 * (double)cfg_setpoint;
	*drop_rate	= 0;
	*nqueues = nqueues_mp;

	for (i=0; i<MAX_PIPES; i++) {
		pipes[i].algo    = shm_malloc(sizeof(int));
		pipes[i].limit   = shm_malloc(sizeof(int));
		pipes[i].load    = shm_malloc(sizeof(int));
		pipes[i].counter = shm_malloc(sizeof(int));
		*pipes[i].algo    = pipes[i].algo_mp;
		*pipes[i].limit   = pipes[i].limit_mp;
		*pipes[i].load    = 0;
		*pipes[i].counter = 0;
	}
	
	for (i=0; i<*nqueues; i++) {
		queues[i].pipe   = shm_malloc(sizeof(int));
		queues[i].method = shm_malloc(sizeof(str));

		*queues[i].pipe   = queues[i].pipe_mp;
		if(str_cpy(queues[i].method, &queues[i].method_mp)) {
			LOG(L_ERR, "out of memory\n");
			return -1;
		}
	}

	return 0;
}

/* generic SER module functions */
static int child_init(int rank)
{
	DBG("RATELIMIT:init_child #%d / pid <%d>\n", rank, getpid());
	return 0;
}

void destroy(void)
{
	int i;
	DBG("RATELIMIT: destroy module ...\n");

	regfree(& pipe_params_regex);
	regfree(&queue_params_regex);

	for (i=0;  i<MAX_PIPES; i++) {
		shm_free(pipes[i].load);
		shm_free(pipes[i].counter);
		shm_free(pipes[i].limit);
		shm_free(pipes[i].algo);
	}
}

static int fixup_rl_drop(void **param, int param_no)
{
	if (param_no == 1) 
		return fix_param(FPARAM_INT, param);
	else
		return fixup_var_int_12(param, 2);
}

#define FP_INT(val) { \
	.orig = "int_value", \
	.type = FPARAM_INT, \
	.v = { \
		.i = val \
	},\
};

#define FP_STRING(val, l) { \
	.orig = val, \
	.type = FPARAM_STR, \
	.v = { \
		.str = { \
			.s = val, \
			.len = l \
		}, \
	},\
};

static fparam_t fp_503 = FP_INT(503);
static fparam_t fp_server_busy = FP_STRING("Server Busy", 11);

static int rl_drop(struct sip_msg * msg, int low, int high)
{
	str hdr = {pkg_malloc(64), 0};
	int ret;

	LOG(L_DBG, "rl_drop(%d, %d sl_reply = %p)\n", low, high, sl_reply);

	if (! hdr.s) {
		LOG(L_ERR, "rl_drop: no memory for hdr\n");
		return 0;
	}

	hdr.len = snprintf(hdr.s, 63, "Retry-After: %d\r\n", 
			low + rand() % (high - low + 1));

	if (add_lump_rpl(msg, hdr.s, hdr.len, LUMP_RPL_HDR)==0) {
		LOG(L_ERR, "Can't add header\n");
 		return 0;
 	}
	
	ret = sl_reply(msg, (char *)&fp_503, (char *)&fp_server_busy);
	pkg_free(hdr.s);
	return ret;
}

static int w_rl_drop(struct sip_msg* msg, char *p1, char *p2) 
{
	int low, high;

	LOG(L_DBG, "w_rl_drop (%p, %p)\n", p1, p2);

	if (!p1 || get_int_fparam(&low, msg, (fparam_t *)p1) < 0) {
		LOG(L_DBG, "using default low retry interval\n");
		low = 5;
	}

	if (!p2 || get_int_fparam(&high, msg, (fparam_t *)p2) < 0) {
		LOG(L_DBG, "using default high retry interval\n");
		high = low;
	}

	return rl_drop(msg, low, high);
}

static int act_busy(struct sip_msg* msg, char *p1, char *p2)
{
	static double x = 0.0;
	int i;

	for (i=0; i<10000; i++) {
		x += (double)i;
		x /= (double)i;
	}
	return (x < 10) ? 1 : -1;
}

static inline int str_i_cmp(str a, str b)
{
	return ! (a.len == b.len && ! memcmp(a.s, b.s, a.len));
}

str queue_other = { "*", 1 };

/**
 * finds the queue associated with the message's method
 * \reture 0 if a nueue was found, -1 otherwise
 */
static int find_queue(struct sip_msg * msg, int * queue)
{
	str method = msg->first_line.u.request.method;
	int i;

	*queue = -1;
	for (i=0; i<*nqueues; i++)
		if (! str_i_cmp(*queues[i].method, method)) {
			*queue = i;
			return 0;
		} else if (! str_i_cmp(*queues[i].method, queue_other)) {
			*queue = i;
		}

	if (*queue >= 0)
		return 0;

	LOG(L_INFO, "no queue matches\n");
	return -1;
}

/* this is here to avoid using rand() ... which doesn't _always_ return
 * exactly what we want (see NOTES section in 'man 3 rand')
 */
int hash[100] = {18, 50, 51, 39, 49, 68, 8, 78, 61, 75, 53, 32, 45, 77, 31, 
	12, 26, 10, 37, 99, 29, 0, 52, 82, 91, 22, 7, 42, 87, 43, 73, 86, 70, 
	69, 13, 60, 24, 25, 6, 93, 96, 97, 84, 47, 79, 64, 90, 81, 4, 15, 63, 
	44, 57, 40, 21, 28, 46, 94, 35, 58, 11, 30, 3, 20, 41, 74, 34, 88, 62, 
	54, 33, 92, 76, 85, 5, 72, 9, 83, 56, 17, 95, 55, 80, 98, 66, 14, 16, 
	38, 71, 23, 2, 67, 36, 65, 27, 1, 19, 59, 89, 48};

/**
 * runs the pipe's algorithm
 * \return -1 if drop needed, 1 if allowed
 */
static int pipe_push(struct sip_msg * msg, int id)
{
	(*pipes[id].counter)++;

	switch (*pipes[id].algo) {
		case PIPE_ALGO_NOP:
			LOG(L_INFO, "warning: queue connected to NOP pipe\n");
			return 1;
		case PIPE_ALGO_TAILDROP:
			return (*pipes[id].counter <= *pipes[id].limit * timer_interval) ? 1 : -1;
		case PIPE_ALGO_RED:
			if (*pipes[id].load == 0)
				return 1;
			return (! (*pipes[id].counter % *pipes[id].load)) ? 1 : -1;
		case PIPE_ALGO_FEEDBACK:
			return (hash[*pipes[id].counter % 100] < *drop_rate) ? -1 : 1;
		default:
			LOG(L_ERR, "unknown ratelimit algorithm: %d\n", *pipes[id].algo);
			return 1;
	}
}

/**
 * runs the current request through the queues
 * \return -1 if drop needed, 1 if allowed
 */
static int rl_check(struct sip_msg * msg, char * a, char * b)
{
	int id, ret;
	str method = msg->first_line.u.request.method;

	if (find_queue(msg, &id))
		return 1;

	ret = pipe_push(msg, *queues[id].pipe);
	LOG(L_DBG,
			"meth=%.*s queue=%d pipe=%d algo=%d limit=%d pkg_load=%d counter=%d "
			"cpu_load=%2.1lf => %s\n",
			method.len,
			method.s, 
			id, 
			*queues[id].pipe,
			*pipes[*queues[id].pipe].algo,
			*pipes[*queues[id].pipe].limit,
			*pipes[*queues[id].pipe].load,
			*pipes[*queues[id].pipe].counter, 
			*cpu_load,
			(ret == 1) ? "ACCEPT" : "DROP");
	
	return ret;
}

typedef struct pipe_params {
	int no;
	int algo;
	int limit;
} pipe_params_t;

typedef struct queue_params {
	int pipe;
	str method;
} queue_params_t;

/**
 * compiles regexes for parsing modparams and clears the pipes and queues
 * \return	0 on success
 */
static int init_params()
{
	if (regcomp(&pipe_params_regex, "^([0-9]+):([^: ]+):([0-9]+)$",
				REG_EXTENDED|REG_ICASE) ||
		regcomp(&queue_params_regex, "^([0-9]+):([^: ]+)$",
				REG_EXTENDED|REG_ICASE)) {
		LOG(L_ERR, "can't compile modparam regexes\n");
		return -1;
	}

	memset(pipes, 0, sizeof(pipes));
	memset(queues, 0, sizeof(queues));

	params_inited = 1;
	return 0;
}

#define RXLS(m, str, i) (m)[i].rm_eo - (m)[i].rm_so, (str) + (m)[i].rm_so
#define RXL(m, str, i) (m)[i].rm_eo - (m)[i].rm_so
#define RXS(m, str, i) (str) + (m)[i].rm_so

/**
 * parses a "pipe_no:algorithm:bandwidth" line
 * \return	0 on success
 */
static int parse_pipe_params(char * line, pipe_params_t * params)
{
	regmatch_t m[4];

	if (! params_inited && init_params())
			return -1;
	if (regexec(&pipe_params_regex, line, 4, m, 0)) {
		LOG(L_ERR, "invalid param tuple: %s\n", line);
		return -1;
	}
	LOG(L_INFO, "pipe: [%.*s|%.*s|%.*s]\n", 
			RXLS(m, line, 1), RXLS(m, line, 2), RXLS(m, line, 3));
	
	params->no = atoi(RXS(m, line, 1));
	params->limit = atoi(RXS(m, line, 3));
	if (str_to_algo(RXLS(m, line, 2), &params->algo))
		return -1;

	return 0;
}

/**
 * parses a "pipe_no:method" line
 * \return	0 on success
 */
static int parse_queue_params(char * line, queue_params_t * params)
{
	regmatch_t m[3];
	int len;

	if (! params_inited && init_params())
			return -1;
	if (regexec(&queue_params_regex, line, 3, m, 0)) {
		LOG(L_ERR, "invalid param tuple: %s\n", line);
		return -1;
	}
	LOG(L_INFO, "queue: [%.*s|%.*s]\n", 
			RXLS(m, line, 1), RXLS(m, line, 2));
	
	params->pipe = atoi(RXS(m, line, 1));

	len = RXL(m, line, 2);
	params->method.s = (char *)pkg_malloc(len+1);
	params->method.len = len;
	memcpy(params->method.s, RXS(m, line, 2), len+1);

	return 0;
}

/**
 * checks that all FEEDBACK pipes use the same setpoint 
 * cpu load. also sets (common) cfg_setpoint value
 * \param modparam	1 to check modparam (static) fields, 0 to use shm ones
 *
 * \return 0 if ok, -1 on error
 */
static int check_feedback_setpoints(int modparam)
{
	int i;

	cfg_setpoint = -1;

	for (i=0; i<MAX_PIPES; i++)
		if (pipes[i].algo_mp == PIPE_ALGO_FEEDBACK) {
			int sp = modparam ? pipes[i].limit_mp : *pipes[i].limit;

			if (sp < 0 || sp > 100) {
				LOG(L_ERR, "FEEDBACK cpu load must be >=0 and <= 100\n");
				return -1;
			} else if (cfg_setpoint == -1) {
				cfg_setpoint = sp;
			} else if (sp != cfg_setpoint) {
				LOG(L_ERR, "pipe %d: FEEDBACK cpu load values must "
						"be equal for all pipes\n", i);
				return -1;
			}
		}

	return 0;
}

static int add_pipe_params(modparam_t type, void * val)
{
	char * param_line = val;
	pipe_params_t params;

	if (parse_pipe_params(param_line, &params))
		return -1;
	
	if (params.no < 0 || params.no >= MAX_PIPES) {
		LOG(L_ERR, "pipe number %d not allowed (MAX_PIPES=%d, 0-based)\n",
				params.no, MAX_PIPES);
		return -1;
	}

	pipes[params.no].algo_mp = params.algo;
	pipes[params.no].limit_mp = params.limit;

	return check_feedback_setpoints(1);
}

static int add_queue_params(modparam_t type, void * val)
{
	char * param_line = val;
	queue_params_t params;

	if (nqueues_mp >= MAX_QUEUES) {
		LOG(L_ERR, "MAX_QUEUES reached (%d)\n", MAX_QUEUES);
		return -1;
	}

	if (parse_queue_params(param_line, &params))
		return -1;

	if (params.pipe >= MAX_PIPES) {
		LOG(L_ERR, "pipe number %d not allowed (MAX_PIPES=%d, 0-based)\n",
				params.pipe, MAX_PIPES);
		return -1;
	}

	queues[nqueues_mp].pipe_mp = params.pipe;
	queues[nqueues_mp].method_mp = params.method;
	nqueues_mp++;

	return 0;
}

/* timer housekeeping, invoked each timer interval to update the load */
static void timer(unsigned int ticks, void *param) {
	int i;

	update_load();

	for (i=0; i<MAX_PIPES; i++) {
		if (*pipes[i].limit && timer_interval)
			*pipes[i].load = *pipes[i].counter / (*pipes[i].limit * timer_interval);
		*pipes[i].counter = 0;
	}
}

/*
 * RPC functions
 *
 * rpc_stats() dumps the current config/statistics
 * rpc_set_param() sets the limits
 * rpc_timer() sets the timer interval length
 *
 */

/* rpc function documentation */
static const char *rpc_stats_doc[2] = {
	"Print ratelimit statistics", 
	0
};

static const char *rpc_timer_doc[2] = {
	"Set the ratelimit timer_interval length",
	0
};

static const char *rpc_set_pid_doc[2] = {
	"Sets the PID controller parameters: KP, KI, KD", 
	0
};

static const char *rpc_get_pid_doc[2] = {
	"Gets the PID controller parameters: KP, KI, KD", 
	0
};

static const char *rpc_set_pipe_doc[2] = {
	"Sets the pipe params (same format as for modparams)", 
	0
};

static const char *rpc_get_pipes_doc[2] = {
	"Gets the pipe params (same format as for modparams)", 
	0
};

static const char *rpc_set_queue_doc[2] = {
	"Sets the queue params (same format as for modparams)", 
	0
};

static const char *rpc_get_queue_doc[2] = {
	"Gets the queue params (same format as for modparams)", 
	0
};

/* rpc function implementations */
static void rpc_stats(rpc_t *rpc, void *c) {
#if 0
#if defined(RL_WITH_RED)
	if (rpc->printf(c, "   INVITE: %d/%d (drop rate: %d)", *invite_counter,
		*invite_limit, *invite_load) < 0) return;
	rpc_lf(rpc, c);
	if (rpc->printf(c, " REGISTER: %d/%d (drop rate: %d)", *register_counter,
		*register_limit, *register_load) < 0) return;
	rpc_lf(rpc, c);
	if (rpc->printf(c, "SUBSCRIBE: %d/%d (drop rate: %d)", *subscribe_counter,
		*subscribe_limit, *subscribe_load) < 0) return;
	rpc_lf(rpc, c);
#else
	if (rpc->printf(c, "   INVITE: %d/%d", *invite_counter,
		*invite_limit) < 0) return;
	rpc_lf(rpc, c);
	if (rpc->printf(c, " REGISTER: %d/%d", *register_counter,
		*register_limit) < 0) return;
	rpc_lf(rpc, c);
	if (rpc->printf(c, "SUBSCRIBE: %d/%d", *subscribe_counter,
		*subscribe_limit) < 0) return;
	rpc_lf(rpc, c);
#endif
#endif
}

static void rpc_timer(rpc_t *rpc, void *c) {
	rpc->fault(c, 400, "Not yet implemented");
}

static void rpc_set_pid(rpc_t *rpc, void *c) 
{

	double kp, ki, kd;

	if (rpc->scan(c, "fff", &kp, &ki, &kd) < 3) {
		rpc->fault(c, 400, "Params expected");
		return;
	}
	LOG(L_INFO, "set_pid: %lf, %lf, %lf\n", kp, ki, kd);
	*pid_ki = ki;
	*pid_kp = kp;
	*pid_kd = kd;
}

static void rpc_get_pid(rpc_t *rpc, void *c) 
{
	if (rpc->add(c, "fff", *pid_kp, *pid_ki, *pid_kd) < 0) return;
}

/* FIXME: if more than one pipe is set up, this can't change the load values for
 * all pipes so changing it for just one will result in an error.
 * possible fix: changing one load should change all of them
 *
 * TODO: see if the string returned by rpc->scan needs to be freed
 */
static void rpc_set_pipe(rpc_t *rpc, void *c) 
{
	int pipe_no, algo_id, limit;
	str algo;

	if (rpc->scan(c, "dSd", &pipe_no, &algo, &limit) < 3) {
		rpc->fault(c, 400, "Params expected");
		return;
	}

	if (str_to_algo(algo.len, algo.s, &algo_id)) {
		rpc->fault(c, 400, "Unknown algo");
		return;
	}

	LOG(L_INFO, "set_pipe: %d:%d:%d\n", pipe_no, algo_id, limit);
	
	if (pipe_no < 0 || pipe_no >= MAX_PIPES) {
		rpc->fault(c, 400, "invalid pipe number");
		return;
	}

	*pipes[pipe_no].algo = algo_id;
	*pipes[pipe_no].limit = limit;

	if (check_feedback_setpoints(0)) {
		rpc->fault(c, 400, "feedback limits don't match");
		return;
	} else {
		*pid_setpoint = 0.01 * (double)cfg_setpoint;
	}
}

static void rpc_get_pipes(rpc_t *rpc, void *c) 
{
	char * algo;
	int i;

	for (i=0; i<MAX_PIPES; i++) {
		if (algo_to_str(*pipes[i].algo, &algo)) {
			rpc->fault(c, 400, "unknown algo");
			return;
		}
	
		if (rpc->add(c, "sd", algo, *pipes[i].limit) < 0)
			return;
	}
}

/* TODO: if nqueues <= queue_no < MAX_QUEUES then 
 * add queue instead of changing it 
 */
static void rpc_set_queue(rpc_t *rpc, void *c) 
{
	int queue_no, pipe_no;
	str method;

	if (rpc->scan(c, "dSd", &queue_no, &method, &pipe_no) < 3) {
		rpc->fault(c, 400, "Params expected");
		return;
	}

	if (queue_no < 0 || queue_no >= *nqueues) {
		rpc->fault(c, 400, "MAX_QUEUES reached");
		return;
	}

	LOG(L_INFO, "set_queue: %d:%.*s:%d\n", queue_no, 
			method.len, method.s,
			pipe_no);

	if (pipe_no >= MAX_PIPES) {
		rpc->fault(c, 400, "invalid pipe number");
		return;
	}
	
	*queues[queue_no].pipe = pipe_no;
	if (str_cpy(queues[queue_no].method, &method)) {
		rpc->fault(c, 400, "out of memory");
		return;
	}
}

static void rpc_get_queue(rpc_t *rpc, void *c) 
{
	int qid;

	if (rpc->scan(c, "d", &qid) < 1) {
		rpc->fault(c, 400, "Params expected");
		return;
	}

	if (qid < 0 || qid >= *nqueues) {
		rpc->fault(c, 400, "invalid queue id");
		return;
	}

	LOG(L_INFO, "get_queue(%d): %.*s, %d\n", 
			qid, 
			queues[qid].method->len,
			queues[qid].method->s,
			*queues[qid].pipe);

	if (rpc->add(c, "Sd", queues[qid].method, *queues[qid].pipe) < 0)
		return;
}

static rpc_export_t rpc_methods[] = {
	{"rl.stats",			rpc_stats,		rpc_stats_doc,		0},
	{"rl.timer_interval",	rpc_timer,		rpc_timer_doc,		0},

	{"rl.set_pid",			rpc_set_pid,	rpc_set_pid_doc,	0},
	{"rl.get_pid",			rpc_get_pid,	rpc_get_pid_doc,	0},

	{"rl.set_pipe",			rpc_set_pipe,	rpc_set_pipe_doc,	0},
	{"rl.get_pipes",		rpc_get_pipes,	rpc_get_pipes_doc,	0},

	{"rl.set_queue",		rpc_set_queue,	rpc_set_queue_doc,	0},
	{"rl.get_queue",		rpc_get_queue,	rpc_get_queue_doc,	0},

	{0, 0, 0, 0}
};
