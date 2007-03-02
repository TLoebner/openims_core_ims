/*
 * $Id$
 *
 * Copyright (C) 2006 iptelorg GmbH
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
 * send commands using binrpc
 *
 * History:
 * --------
 *  2006-02-14  created by andrei
 */


#include <stdlib.h> /* exit, abort */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h> /* isprint */
#include <sys/socket.h>
#include <sys/un.h> /* unix sock*/
#include <netinet/in.h> /* udp sock */
#include <sys/uio.h> /* writev */
#include <netdb.h> /* gethostbyname */
#include <time.h> /* time */

#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "parse_listen_id.h"
#include "license.h"

#include "../../modules/ctl/ctl_defaults.h" /* default socket & port */
#include "../../modules/ctl/binrpc.h"
#include "../../modules/ctl/binrpc.c" /* ugly hack */


#ifndef NAME
#define NAME    "sercmd"
#endif
#ifndef VERSION
#define VERSION "0.1"
#endif

#define IOVEC_CNT 20
#define BUF_SIZE  65535
#define MAX_LINE_SIZE 1024 /* for non readline mode */
#define MAX_REPLY_SIZE  4096
#define MAX_BODY_SIZE   4096
#define MAX_BINRPC_ARGS  128


#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

static char id[]="$Id$";
static char version[]= NAME " " VERSION;
static char compiled[]= __TIME__ " " __DATE__;
static char help_msg[]="\
Usage: " NAME " [options][-s address] [ cmd ]\n\
Options:\n\
    -s address  unix socket name or host name to send the commands on\n\
    -R name     force reply socket name, for the unix datagram socket mode\n\
    -D dir      create the reply socket in the directory <dir> if no reply \n\
                socket is forced (-R) and a unix datagram socket is selected\n\
                as the transport\n\
    -f format   print the result using format. Format is a string containing\n\
                %v at the places where values read from the reply should be\n\
                substituted. To print '%v', escape it using '%': %%v.\n\
    -v          Verbose       \n\
    -V          Version number\n\
    -h          This help message\n\
address:\n\
    [proto:]name[:port]   where proto is one of tcp, udp, unixs or unixd\n\
                          e.g.:  tcp:localhost:2048 , unixs:/tmp/ser_ctl\n\
cmd:\n\
    method  [arg1 [arg2...]]\n\
arg:\n\
     string or number; to force a number to be interpreted as string \n\
     prefix it by \"s:\", e.g. s:1\n\
Examples:\n\
        " NAME " -s unixs:/tmp/ser_unix system.listMethods\n\
        " NAME " -f \"pid: %v  desc: %v\\n\" -s udp:localhost:2047 core.ps \n\
        " NAME " ps  # uses default ctl socket \n\
        " NAME "     # enters interactive mode on the default socket \n\
        " NAME " -s tcp:localhost # interactive mode, default port \n\
";


int verbose=0;
char* reply_socket=0; /* unix datagram reply socket name */
char* sock_dir=0;     /* same as above, but only the directory */
char* unix_socket=0;
struct sockaddr_un mysun;
int quit; /* used only in interactive mode */

struct binrpc_val* rpc_array;
int rpc_no=0;




#define IOV_SET(vect, str) \
	do{\
		(vect).iov_base=(str); \
		(vect).iov_len=strlen((str)); \
	}while(0)


#define INT2STR_MAX_LEN  (19+1+1) /* 2^64~= 16*10^18 => 19+1 digits + \0 */

/* returns a pointer to a static buffer containing l in asciiz & sets len */
static inline char* int2str(unsigned int l, int* len)
{
	static char r[INT2STR_MAX_LEN];
	int i;
	
	i=INT2STR_MAX_LEN-2;
	r[INT2STR_MAX_LEN-1]=0; /* null terminate */
	do{
		r[i]=l%10+'0';
		i--;
		l/=10;
	}while(l && (i>=0));
	if (l && (i<0)){
		fprintf(stderr, "BUG: int2str: overflow\n");
	}
	if (len) *len=(INT2STR_MAX_LEN-2)-i;
	return &r[i+1];
}



static char* trim_ws(char* l)
{
	char* ret;
	
	for(;*l && ((*l==' ')||(*l=='\t')||(*l=='\n')||(*l=='\r')); l++);
	ret=l;
	if (*ret==0) return ret;
	for(l=l+strlen(l)-1; (l>ret) && 
			((*l==' ')||(*l=='\t')||(*l=='\n')||(*l=='\r')); l--);
	*(l+1)=0;
	return ret;
}



int gen_cookie()
{
	return rand();
}



struct binrpc_cmd{
	char* method;
	int argc;
	struct binrpc_val argv[MAX_BINRPC_ARGS];
};


struct cmd_alias{
	char* name;
	char* method;
	char* format; /* reply print format */
};


struct sercmd_builtin{
	char* name;
	int (*f)(int, struct binrpc_cmd*);
	char* doc;
};


static int sercmd_help(int s, struct binrpc_cmd* cmd);
static int sercmd_ver(int s, struct binrpc_cmd* cmd);
static int sercmd_quit(int s, struct binrpc_cmd* cmd);
static int sercmd_warranty(int s, struct binrpc_cmd* cmd);


static struct cmd_alias cmd_aliases[]={
	{	"ps",			"core.ps",				"%v\t%v\n"	},
	{	"list",			"system.listMethods",	0			},
	{	"ls",			"system.listMethods",	0			},
	{	"server",		"core.version",			0			},
	{	"serversion",	"core.version",			0			},
	{	"who",			"ctl.who",				"[%v] %v: %v %v -> %v %v\n"},
	{	"listen",		"ctl.listen",			"[%v] %v: %v %v\n"},
	{	"dns_mem_info",		"dns.mem_info",			"%v / %v\n"},
	{	"dns_debug",	"dns.debug",			
					"%v (%v): size=%v ref=%v expire=%vs last=%vs ago f=%v\n"},
	{	"dns_debug_all",	"dns.debug_all",			
			"%v (%v) [%v]: size=%v ref=%v expire=%vs last=%vs ago f=%v\n"
			"\t\t%v:%v expire=%vs f=%v\n"},
	{	"dst_blacklist_mem_info",	"dst_blacklist.mem_info",	"%v / %v\n"},
	{	"dst_blacklist_debug",		"dst_blacklist.debug",	
		"%v:%v:%v expire:%v flags: %v\n"},
	{0,0,0}
};


static struct sercmd_builtin builtins[]={
	{	"?",		sercmd_help, "help"},
	{	"help",		sercmd_help, "displays help for a command"},
	{	"version",	sercmd_ver,  "displays " NAME "version"},
	{	"quit",		sercmd_quit, "exits " NAME },
	{	"exit",		sercmd_quit, "exits " NAME },
	{	"warranty",		sercmd_warranty, "displays " NAME "'s warranty info"},
	{	"license",		sercmd_warranty, "displays " NAME "'s license"},
	{0,0}
};



#ifdef USE_READLINE

/* instead of rl_attempted_completion_over which is not present in
   some readline emulations */
static int attempted_completion_over=0; 

/* commands for which we complete the params to other command names */
char* complete_params[]={
	"?",
	"h",
	"help",
	"system.methodSignature",
	"system.methodHelp",
	0
};
#endif



static int parse_arg(struct binrpc_val* v, char* arg)
{
	int i;
	double f;
	char* tmp;
	int len;
	
	i=strtol(arg, &tmp, 10);
	if ((tmp==0) || (*tmp)){
		f=strtod(arg, &tmp);
		if ((tmp==0) || (*tmp)){
			/* not an int or a float => string */
			len=strlen(arg);
			if ((len>=2) && (arg[0]=='s') && (arg[1]==':')){
				tmp=&arg[2];
				len-=2;
			}else{
				tmp=arg;
			}
			v->type=BINRPC_T_STR;
			v->u.strval.s=tmp;
			v->u.strval.len=len;
		}else{ /* float */
			v->type=BINRPC_T_DOUBLE;
			v->u.fval=f;
		}
	}else{ /* int */
		v->type=BINRPC_T_INT;
		v->u.intval=i;
	}
	return 0;
}



static int parse_cmd(struct binrpc_cmd* cmd, char** argv, int count)
{
	int r;
	
	cmd->method=argv[0];
	if ((count-1)>MAX_BINRPC_ARGS){
		fprintf(stderr,  "ERROR: too many args %d, only %d allowed\n",
					count-1, MAX_BINRPC_ARGS);
		return -1;
	}
	for (r=1; r<count; r++){
		if (parse_arg(&cmd->argv[r-1], argv[r])<0)
			return -1;
	}
	cmd->argc=r-1;
	return 0;
}


void print_binrpc_val(struct binrpc_val* v, int ident)
{
	int r;

	if ((v->type==BINRPC_T_STRUCT) && !v->u.end)
		ident--; /* fix to have strut beg. idented differently */
	for (r=0; r<ident; r++) putchar('	');
	if (v->name.s){
		printf("%.*s: ", v->name.len, v->name.s);
	}
	switch(v->type){
		case BINRPC_T_INT:
			printf("%d", v->u.intval);
			break;
		case BINRPC_T_STR:
		case BINRPC_T_BYTES:
			printf("%.*s", v->u.strval.len, v->u.strval.s);
			break;
		case BINRPC_T_ARRAY:
			printf("%c", (v->u.end)?']':'[');
			break;
		case BINRPC_T_STRUCT:
			printf("%c", (v->u.end)?'}':'{');
			break;
		default:
			printf("ERROR: unknown type %d\n", v->type);
	};
}



/* opens,  and  connects on a STREAM unix socket
 * returns socket fd or -1 on error */
int connect_unix_sock(char* name, int type)
{
	struct sockaddr_un ifsun;
	int s;
	int len;
	int ret;
	int retries;
	
	retries=0;
	s=-1;
	memset(&ifsun, 0, sizeof (struct sockaddr_un));
	len=strlen(name);
	if (len>UNIX_PATH_MAX){
		fprintf(stderr, "ERROR: connect_unix_sock: name too long "
				"(%d > %d): %s\n", len, UNIX_PATH_MAX, name);
		goto error;
	}
	ifsun.sun_family=AF_UNIX;
	memcpy(ifsun.sun_path, name, len);
#ifdef HAVE_SOCKADDR_SA_LEN
	ifsun.sun_len=len;
#endif
	s=socket(PF_UNIX, type, 0);
	if (s==-1){
		fprintf(stderr, "ERROR: connect_unix_sock: cannot create unix socket"
				" %s: %s [%d]\n", name, strerror(errno), errno);
		goto error;
	}
	if (type==SOCK_DGRAM){
		/* we must bind so that we can receive replies */
		if (reply_socket==0){
			if (sock_dir==0)
				sock_dir="/tmp";
retry:
			ret=snprintf(mysun.sun_path, UNIX_PATH_MAX, "%s/" NAME "_%d",
							sock_dir, rand()); 
			if ((ret<0) ||(ret>=UNIX_PATH_MAX)){
				fprintf(stderr, "ERROR: buffer overflow while trying to"
							"generate unix datagram socket name");
				goto error;
			}
		}else{
			if (strlen(reply_socket)>UNIX_PATH_MAX){
				fprintf(stderr, "ERROR: buffer overflow while trying to"
							"use the provided unix datagram socket name (%s)",
							reply_socket);
				goto error;
			}
			strcpy(mysun.sun_path, reply_socket);
		}
		mysun.sun_family=AF_UNIX;
		if (bind(s, (struct sockaddr*)&mysun, sizeof(mysun))==-1){
			if (errno==EADDRINUSE && (reply_socket==0) && (retries < 10)){
				retries++;
				/* try another one */
				goto retry;
			}
			fprintf(stderr, "ERROR: could not bind the unix socket to"
					" %s: %s (%d)\n",
					mysun.sun_path, strerror(errno), errno);
			goto error;
		}
		unix_socket=mysun.sun_path;
	}
	if (connect(s, (struct sockaddr *)&ifsun, sizeof(ifsun))==-1){
		fprintf(stderr, "ERROR: connect_unix_sock: connect(%s): %s [%d]\n",
				name, strerror(errno), errno);
		goto error;
	}
	return s;
error:
	if (s!=-1) close(s);
	return -1;
}



int connect_tcpudp_socket(char* address, int port, int type)
{
	struct sockaddr_in addr;
	struct hostent* he;
	int sock;
	
	sock=-1;
	/* resolve destination */
	he=gethostbyname(address);
	if (he==0){
		fprintf(stderr, "ERROR: could not resolve %s\n", address);
		goto error;
	}
	/* open socket*/
	addr.sin_family=he->h_addrtype;
	addr.sin_port=htons(port);
	memcpy(&addr.sin_addr.s_addr, he->h_addr_list[0], he->h_length);
	
	sock = socket(he->h_addrtype, type, 0);
	if (sock==-1){
		fprintf(stderr, "ERROR: socket: %s\n", strerror(errno));
		goto error;
	}
	if (connect(sock, (struct sockaddr*) &addr, sizeof(struct sockaddr))!=0){
		fprintf(stderr, "ERROR: connect: %s\n", strerror(errno));
		goto error;
	}
	return sock;
error:
	if (sock!=-1) close(sock);
	return -1;
}



static void hexdump(unsigned char* buf, int len, int ascii)
{
	int r, i;
	
	/* dump it in hex */
	for (r=0; r<len; r++){
		if ((r) && ((r%16)==0)){
			if (ascii){
				putchar(' ');
				for (i=r-16; i<r; i++){
					if (isprint(buf[i]))
						putchar(buf[i]);
					else
						putchar('.');
				}
			}
			putchar('\n');
		}
		printf("%02x ", buf[r]);
	};
	if (ascii){
		for (i=r;i%16; i++)
			printf("   ");
		putchar(' ');
		for (i=16*(r/16); i<r; i++){
			if (isprint(buf[i]))
				putchar(buf[i]);
			else
				putchar('.');
		}
	}
	putchar('\n');
}



/* returns: -1 on error, number of bytes written on success */
static int send_binrpc_cmd(int s, struct binrpc_cmd* cmd, int cookie)
{
	struct iovec v[IOVEC_CNT];
	int r;
	unsigned char msg_body[MAX_BODY_SIZE];
	unsigned char msg_hdr[BINRPC_MAX_HDR_SIZE];
	struct binrpc_pkt body;
	int ret;
	int n;
	
	ret=binrpc_init_pkt(&body, msg_body, MAX_BODY_SIZE);
	if (ret<0) goto binrpc_err;
	ret=binrpc_addstr(&body, cmd->method, strlen(cmd->method));
	if (ret<0) goto binrpc_err;
	for (r=0; r<cmd->argc; r++){
		switch(cmd->argv[r].type){
			case BINRPC_T_STR:
				ret=binrpc_addstr(&body, cmd->argv[r].u.strval.s,
										cmd->argv[r].u.strval.len);
				break;
			case BINRPC_T_INT:
				ret=binrpc_addint(&body, cmd->argv[r].u.intval);
				break;
			case BINRPC_T_DOUBLE:
				ret=binrpc_adddouble(&body, cmd->argv[r].u.fval);
				break;
			default:
				fprintf(stderr, "ERROR: unsupported type %d\n",
								cmd->argv[r].type);
		}
		if (ret<0) goto binrpc_err;
	}
	ret=binrpc_build_hdr(BINRPC_REQ, binrpc_pkt_len(&body), cookie, msg_hdr,
							BINRPC_MAX_HDR_SIZE);
	if (ret<0) goto binrpc_err;
	v[0].iov_base=msg_hdr;
	v[0].iov_len=ret;
	v[1].iov_base=msg_body;
	v[1].iov_len=binrpc_pkt_len(&body);
write_again:
	if ((n=writev(s, v, 2))<0){
		if (errno==EINTR)
			goto write_again;
		goto error_send;
	}
	
	return n;
error_send:
	return -1;
binrpc_err:
	return -2;
}


static int binrpc_errno=0;

/* reads the whole reply
 * returns < 0 on error, reply size on success + initializes in_pkt
 * if ret==-2 (parse error), sets binrpc_errno to the binrpc error
 * error returns: -1 - read error (check errno)
 *                -2 - binrpc parse error (chekc binrpc_errno) 
 *                -3 - cookie error (the cookied doesn't match)
 *                -4 - message too big */
static int get_reply(int s, unsigned char* reply_buf, int max_reply_size,
						int cookie, struct binrpc_parse_ctx* in_pkt,
						unsigned char** body)
{
	unsigned char* crt;
	unsigned char* hdr_end;
	unsigned char* msg_end;
	int n;
	int ret;
	
	
	hdr_end=crt=reply_buf;
	msg_end=reply_buf+max_reply_size;
	binrpc_errno=0;
	do{
		n=read(s, crt, (int)(msg_end-crt));
		if (n<0){
			if (errno==EINTR)
				continue;
			goto error_read;
		}
		if (verbose >= 3){
			/* dump it in hex */
			printf("received %d bytes in reply (@offset %d):\n",
					n, (int)(crt-reply_buf));
			hexdump(crt, n, 1);
		}
		crt+=n;
		/* parse header if not parsed yet */
		if (hdr_end==reply_buf){
			hdr_end=binrpc_parse_init(in_pkt, reply_buf, n, &ret);
			if (ret<0){
				if (ret==E_BINRPC_MORE_DATA)
					continue;
				goto error_parse;
			}
			if (verbose>1){
				printf("new packet: type %02x, len %d, cookie %02x\n",
						in_pkt->type, in_pkt->tlen, in_pkt->cookie);
			}
			if (in_pkt->cookie!=cookie){
				fprintf(stderr, "bad reply, cookie doesn't match: sent %02x "
						"and received  %02x\n",
						cookie, in_pkt->cookie);
				goto error;
			}
			msg_end=hdr_end+in_pkt->tlen;
			if ((int)(msg_end-reply_buf)>max_reply_size)
				goto error_toolong;
		}
	}while(crt<msg_end);
	
	*body=hdr_end;
	return (int)(msg_end-reply_buf);
error_read:
	return -1;
error_parse:
	binrpc_errno=ret;
	return -2;
error:
	return -3;
error_toolong:
	return -4;
}



/* returns a malloced copy of str, with all the escapes ('\') resolved */
static char* str_escape(char* str)
{
	char* n;
	char* ret;
	
	ret=n=malloc(strlen(str)+1);
	if (n==0)
		goto end;
	
	for(;*str;str++){
		*n=*str;
		if (*str=='\\'){
			switch(*(str+1)){
				case 'n':
					*n='\n';
					str++;
					break;
				case 'r':
					*n='\r';
					str++;
					break;
				case 't':
					*n='\t';
					str++;
					break;
				case '\\':
					str++;
					break;
			}
		}
		n++;
	}
	*n=*str; /* terminating 0 */
end:
	return ret;
}



/* parses strings like "bla bla %v 10%% %v\n test=%v",
 * and stops at each %v,  returning  a pointer after the %v, setting *size
 * to the string length (not including %v) and *type to the corresponding
 * BINRPC type (for now only BINRPC_T_ALL).
 * To escape a '%', use "%%", and check for type==-1 (which means skip an call
 *  again parse_fmt).
 * Usage:
 *        n="test: %v,%v,%v\n";
 *        while(*n){
 *          s=n;
 *          n=parse_fmt(n, &type, &size);
 *          printf("%.*s", size, s);
 *          if (type==-1)
 *            continue;
 *          else 
 *             printf("now we should get & print an object of type %d\n", type)
 *        }
 */
static char* parse_fmt(char* fmt, int* type, int* size)
{
	char* s;

	s=fmt;
	do{
		for(;*fmt && *fmt!='%'; fmt++);
		if (*fmt=='%'){
			switch(*(fmt+1)){
				case 'v':
					*type=BINRPC_T_ALL;
					*size=(int)(fmt-s);
					return (fmt+2);
					break;
				case '%':
					/* escaped % */
					*size=(int)(fmt-s)+1;
					*type=-1; /* skip */
					return (fmt+2);
					break;
			}
		}
	}while(*fmt);
	*type=-1; /* no value */
	*size=(fmt-s);
	return fmt;
}



static int print_body(struct binrpc_parse_ctx* in_pkt, 
						unsigned char* body, int size, char* fmt)
{
	
	unsigned char* p;
	unsigned char* end;
	struct binrpc_val val;
	int ret;
	int rec;
	char *f;
	char* s;
	int f_size;
	int fmt_has_values;
	
	p=body;
	end=p+size;
	rec=0;
	f=fmt;
	fmt_has_values=0;
	/* read body */
	while(p<end){
		if (f){
			
			do{
				if (*f==0)
					f=fmt; /* reset */
				s=f;
				f=parse_fmt(f, &val.type, &f_size);
				printf("%.*s", f_size, s);
				if (val.type!=-1){
					fmt_has_values=1;
					goto read_value;
				}
			}while(*f || fmt_has_values);
			val.type=BINRPC_T_ALL;
		}else{
			val.type=BINRPC_T_ALL;
		}
read_value:
		val.name.s=0;
		val.name.len=0;
		p=binrpc_read_record(in_pkt, p, end, &val, &ret);
		if (ret<0){
			if (fmt)
				putchar('\n');
			/*if (ret==E_BINRPC_MORE_DATA)
				goto error_read_again;*/
			if (ret==E_BINRPC_EOP){
				printf("end of message detected\n");
				break;
			}
			fprintf(stderr, "ERROR while parsing the record %d,"
					" @%d: %02x : %s\n", rec,
					in_pkt->offset, *p, binrpc_error(ret));
			goto error;
		}
		rec++;
		if (fmt){
			print_binrpc_val(&val, 0);
		}else{
			print_binrpc_val(&val, in_pkt->in_struct+in_pkt->in_array);
			putchar('\n');
		}
	}
	if (fmt && *f){
		/* print the rest, with empty values */
		while(*f){
			s=f;
			f=parse_fmt(f, &val.type, &f_size);
			printf("%.*s", f_size, s);
		}
	}
	return 0;
error:
	return -1;
/*error_read_again:
	fprintf(stderr, "ERROR: more data needed\n");
	return -2;
	*/
}



static int print_fault(struct binrpc_parse_ctx* in_pkt, 
						unsigned char* body, int size)
{
	printf("error: ");
	return print_body(in_pkt, body, size, "%v - %v\n");
}



static int run_binrpc_cmd(int s, struct binrpc_cmd * cmd, char* fmt)
{
	int cookie;
	unsigned char reply_buf[MAX_REPLY_SIZE];
	unsigned char* msg_body;
	struct binrpc_parse_ctx in_pkt;
	int ret;
	
	cookie=gen_cookie();
	if ((ret=send_binrpc_cmd(s, cmd, cookie))<0){
		if (ret==-1) goto error_send;
		else goto binrpc_err;
	}
	/* read reply */
	memset(&in_pkt, 0, sizeof(in_pkt));
	if ((ret=get_reply(s, reply_buf, MAX_REPLY_SIZE, cookie, &in_pkt,
					&msg_body))<0){
		switch(ret){
			case -1:
				goto error_read;
			case -2:
				goto error_parse;
			case -3:
				goto error_cookie;
			case -4:
				goto error_toobig;
		}
		goto error;
	}
	switch(in_pkt.type){
		case BINRPC_FAULT:
			if (print_fault(&in_pkt, msg_body, in_pkt.tlen)<0){
				goto error;
			}
			break;
		case BINRPC_REPL:
			if (print_body(&in_pkt, msg_body, in_pkt.tlen, fmt)<0){
				goto error;
			}
			break;
		default:
			fprintf(stderr, "ERROR: not a reply\n");
			goto error;
	}
	if (verbose) printf(".\n");
	/* normal exit */
	return 0;
binrpc_err:
	fprintf(stderr, "ERROR while building the packet: %s\n", 
				binrpc_error(ret));
	goto error;
error_parse:
	fprintf(stderr, "ERROR while parsing the reply: %s\n", 
				binrpc_error(binrpc_errno));
	goto error;
error_cookie:
	fprintf(stderr, "ERROR: cookie does not match\n");
	goto error;
error_toobig:
	fprintf(stderr, "ERROR: reply too big\n");
	goto error;
error_send:
	fprintf(stderr, "ERROR: send packet failed: %s (%d)\n",
			strerror(errno), errno);
	goto error;
error_read:
	fprintf(stderr, "ERROR: read reply failed: %s (%d)\n",
			strerror(errno), errno);
	goto error;
error:
	return -1;
}



static int parse_line(struct binrpc_cmd* cmd, char* line)
{
	char* p;
	int count;
	
	cmd->method=strtok(line, " \t");
	if (cmd->method==0)
		goto error_no_method;
	count=0;
	for(p=strtok(0, " \t"); p; p=strtok(0, " \t")){
		if (count>=MAX_BINRPC_ARGS)
			goto error_too_many;
		if (parse_arg(&cmd->argv[count], p)<0){
			goto error_arg;
		}
		count++;
	}
	cmd->argc=count;
	return 0;
error_no_method:
	printf( "ERROR: no method name\n");
	return -1;
error_too_many:
	printf("ERROR: too many arguments (%d), no more than %d allowed\n",
			count, MAX_BINRPC_ARGS);
	return -1;
error_arg:
	printf("ERROR: bad argument %d: %s\n", count+1, p);
	return -1;
}



/* resolves builtin aliases */
static void fix_cmd(struct binrpc_cmd* cmd, char** format)
{
	int r;
	
	for (r=0; cmd_aliases[r].name; r++){
		if (strcmp(cmd_aliases[r].name, cmd->method)==0){
			cmd->method=cmd_aliases[r].method;
			if (*format==0)
				*format=cmd_aliases[r].format;
			break;
		}
	}
}



/* intercept builtin commands, returns 1 if intercepted, 0 if not, <0 on error
 */
static int run_builtins(int s, struct binrpc_cmd* cmd)
{
	int r;
	int ret;
	
	for (r=0; builtins[r].name; r++){
		if (strcmp(builtins[r].name, cmd->method)==0){
			ret=builtins[r].f(s, cmd);
			return (ret<0)?ret:1;
		}
	}
	return 0;
}



/* runs command from cmd */
inline static int run_cmd(int s, struct binrpc_cmd* cmd, char* format)
{
	int ret;
	char* fmt;

	fmt=format;
	
	fix_cmd(cmd, &fmt);
	if (!(ret=run_builtins(s, cmd))){
		ret=run_binrpc_cmd(s, cmd, fmt);
	}
	return (ret>0)?0:ret;
}



/* runs a command represented in line */
inline static int run_line(int s, char* l, char* format)
{
	struct binrpc_cmd cmd;
	int ret;
	
	if ((ret=parse_line(&cmd, l))==0){
		return run_cmd(s, &cmd, format);
	}
	return ret;
}



/* parse the body into a malloc allocated,  binrpc_val array */
static struct binrpc_val* parse_reply_body(int* records, 
											struct binrpc_parse_ctx* in_pkt,
											unsigned char* body, int size)
{
	struct binrpc_val* a;
	struct binrpc_val* t;
	unsigned char* p;
	unsigned char* end;
	struct binrpc_val val;
	int ret;
	int rec;

	if (*records==0){
		*records=100; /* start with a reasonable size */
	};
	a=malloc(*records*sizeof(struct binrpc_val));
	if (a==0)
		goto error_mem;
	p=body;
	end=p+size;
	rec=0;
	
	/* read body */
	while(p<end){
		val.type=BINRPC_T_ALL;
		val.name.s=0;
		val.name.len=0;
		p=binrpc_read_record(in_pkt, p, end, &val, &ret);
		if (ret<0){
			if (ret==E_BINRPC_EOP){
				printf("end of message detected\n");
				break;
			}
			fprintf(stderr, "ERROR while parsing the record %d,"
					" @%d: %02x : %s\n", rec,
					in_pkt->offset, *p, binrpc_error(ret));
			goto error;
		}
		if (rec>=*records){
			t=realloc(a, *records*sizeof(struct binrpc_val)*2);
			if (t==0)
				goto error_mem;
			a=t;
			*records*=2;
		}
		a[rec]=val;
		rec++;
	}
	if (rec && (rec<*records)){
		a=realloc(a, rec*sizeof(struct binrpc_val));
	}
	*records=rec;
	return a;
error_mem:
	fprintf(stderr, "ERROR: parse_reply_body: out of memory\n");
error:
	if (a){
		free(a);
	}
	*records=0;
	return 0;
}



static int get_sercmd_list(int s)
{
	struct binrpc_cmd cmd;
	int cookie;
	unsigned char reply_buf[MAX_REPLY_SIZE];
	unsigned char* msg_body;
	struct binrpc_parse_ctx in_pkt;
	int ret;
	
	cmd.method="system.listMethods";
	cmd.argc=0;
	
	cookie=gen_cookie();
	if ((ret=send_binrpc_cmd(s, &cmd, cookie))<0){
		if (ret==-1) goto error_send;
		else goto binrpc_err;
	}
	/* read reply */
	memset(&in_pkt, 0, sizeof(in_pkt));
	if ((ret=get_reply(s, reply_buf, MAX_REPLY_SIZE, cookie, &in_pkt,
					&msg_body))<0){
		goto error;
	}
	switch(in_pkt.type){
		case BINRPC_FAULT:
			if (print_fault(&in_pkt, msg_body, in_pkt.tlen)<0){
				goto error;
			}
			break;
		case BINRPC_REPL:
			rpc_no=10; /* default cmd list */
			if ((rpc_array=parse_reply_body(&rpc_no, &in_pkt, msg_body,
												in_pkt.tlen))==0)
				goto error;
			break;
		default:
			fprintf(stderr, "ERROR: not a reply\n");
			goto error;
	}
	return 0;
binrpc_err:
error_send:
error:
	return -1;
}



static void print_formatting(char* prefix, char* format, char* suffix)
{
	if (format){
		printf("%s", prefix);
		for (;*format;format++){
			switch(*format){
				case '\t':
					printf("\\t");
					break;
				case '\n':
					printf("\\n");
					break;
				case '\r':
					printf("\\r");
					break;
				default:
					putchar(*format);
			}
		}
		printf("%s", suffix);
	}
}



static int sercmd_help(int s, struct binrpc_cmd* cmd)
{
	int r;
	
	if (cmd->argc && (cmd->argv[0].type==BINRPC_T_STR)){
		/* if it has args, try command help */
		for (r=0; cmd_aliases[r].name; r++){
			 if (strcmp(cmd->argv[0].u.strval.s, cmd_aliases[r].name)==0){
				 printf("%s is an alias for %s", cmd->argv[0].u.strval.s,
						 						cmd_aliases[r].method);
				 print_formatting(" with reply formatting: \"",
						 			cmd_aliases[r].format, "\"");
				 putchar('\n');
				 return 0;
			 }
		}
		for(r=0; builtins[r].name; r++){
			 if (strcmp(cmd->argv[0].u.strval.s, builtins[r].name)==0){
				 printf("builtin command: %s\n", 
						 builtins[r].doc?builtins[r].doc:"undocumented");
				 return 0;
			 }
		}
		cmd->method="system.methodHelp";
		if (run_binrpc_cmd(s, cmd, 0)<0){
			printf("error: no such command %s\n", cmd->argv[0].u.strval.s);
		}
		return 0;
	}
		
	if (rpc_no==0){
		if (get_sercmd_list(s)<0)
			goto error;
	}
	for (r=0; r<rpc_no; r++){
		if (rpc_array[r].type==BINRPC_T_STR){
			printf("%s\n", rpc_array[r].u.strval.s);
		}
	}
	for (r=0; cmd_aliases[r].name; r++){
		printf("alias: %s\n", cmd_aliases[r].name);
	}
	for(r=0; builtins[r].name; r++){
		printf("builtin: %s\n", builtins[r].name);
	}
	return 0;
error:
	return -1;
}



static int sercmd_ver(int s, struct binrpc_cmd* cmd)
{
	printf("%s\n", version);
	printf("%s\n", id);
	printf("%s compiled on %s \n", __FILE__, compiled);
#ifdef USE_READLINE
	printf("interactive mode command completion support\n");
#endif
	return 0;
}



static int sercmd_quit(int s, struct binrpc_cmd* cmd)
{
	quit=1;
	return 0;
}



static int sercmd_warranty(int s, struct binrpc_cmd *cmd)
{
	printf("%s %s\n", NAME, VERSION);
	printf("%s\n", COPYRIGHT);
	printf("\n%s\n", LICENSE);
	return 0;
}


#ifdef USE_READLINE

/* readline command generator */
static char* sercmd_generator(const char* text, int state)
{
	static int idx;
	static int list; /* aliases, builtins, rpc_array */
	static int len;
	char* name;
	
	if (attempted_completion_over)
		return 0;
	
	if (state==0){
		/* init */
		idx=list=0;
		len=strlen(text);
	}
	/* return next partial match */
	switch(list){
		case 0: /* aliases*/
			while((name=cmd_aliases[idx].name)){
				idx++;
				if (strncmp(name, text, len)==0)
					return strdup(name);
			}
			list++;
			idx=0;
			/* no break */
		case 1: /* builtins */
			while((name=builtins[idx].name)){
				idx++;
				if (strncmp(name, text, len)==0)
					return strdup(name);
			}
			list++;
			idx=0;
			/* no break */
		case 2: /* rpc_array */
			while(idx < rpc_no){
				if (rpc_array[idx].type==BINRPC_T_STR){
					name=rpc_array[idx].u.strval.s;
					idx++;
					if (strncmp(name, text, len)==0)
						return strdup(name);
				}else{
					idx++;
				}
			}
	}
	/* no matches */
	return 0;
}


char** sercmd_completion(const char* text, int start, int end)
{
	int r;
	int i;
	
	attempted_completion_over=1;
	/* complete only at beginning */
	if (start==0){
		attempted_completion_over=0;
	}else{ /* or if this is a command for which we complete the parameters */
		/* find first whitespace */
		for(r=0; (r<start) && (rl_line_buffer[r]!=' ') && 
				(rl_line_buffer[r]!='\t'); r++);
		for(i=0; complete_params[i]; i++){
			if ((r==strlen(complete_params[i])) &&
					(strncmp(rl_line_buffer, complete_params[i], r)==0)){
					attempted_completion_over=0;
					break;
			}
		}
	}
	return 0; /* let readline call sercmd_generator */
}

#endif /* USE_READLINE */



/* on exit cleanup */
static void  cleanup()
{
	if (unix_socket){
		if (unlink(unix_socket)<0){
			fprintf(stderr, "ERROR: failed to delete %s: %s\n",
					unix_socket, strerror(errno));
		}
	}
}



int main(int argc, char** argv)
{
	int c;
	char* sock_name;
	int port_no;
	int sock_type;
	int s;
	struct binrpc_cmd cmd;
	int rec;
	struct id_list* sock_id;
	char* format;
	int interactive;
	char* line;
	char* l;

	quit=0;
	format=0;
	line=0;
	interactive=0;
	rec=1;
	s=-1;
	sock_name=0;
	port_no=0;
	sock_type=UNIXS_SOCK;
	opterr=0;
	while((c=getopt(argc, argv, "Vhs:D:R:vf:"))!=-1){
		switch(c){
			case 'V':
				printf("version: %s\n", version);
				printf("%s\n", id);
				printf("%s compiled on %s \n", __FILE__,
						compiled);
				exit(0);
				break;
			case 'h':
				printf("version: %s\n", version);
				printf("%s", help_msg);
				exit(0);
				break;
			case 's':
				sock_name=optarg;
				break;
			case 'R':
				reply_socket=optarg;
				break;
			case 'D':
				sock_dir=optarg;
				break;
			case 'U':
				sock_type=UDP_SOCK;
				break;
			case 'v':
				verbose++;
				break;
			case 'f':
				format=str_escape(optarg);
				if (format==0){
					fprintf(stderr, "ERROR: memory allocation failure\n");
					goto error;
				}
				break;
			case '?':
				if (isprint(optopt))
					fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf(stderr, 
							"Unknown option character `\\x%x'.\n",
							optopt);
				goto error;
			case ':':
				fprintf(stderr, 
						"Option `-%c' requires an argument.\n",
						optopt);
				goto error;
			default:
				abort();
		}
	}
	if (sock_name==0){
		sock_name=DEFAULT_CTL_SOCKET;
	}
	
	/* init the random number generator */
	srand(getpid()+time(0)); /* we don't need very strong random numbers */
	
	if (sock_name==0){
		fprintf(stderr, "ERROR: no ser address specified\n");
		goto error;
	}
	sock_id=parse_listen_id(sock_name, strlen(sock_name), sock_type);
	if (sock_id==0){
		fprintf(stderr, "ERROR: error parsing ser adress %s\n", sock_name);
		goto error;
	}
	
	switch(sock_id->proto){
		case UDP_SOCK:
		case TCP_SOCK:
			if (sock_id->port==0){
				sock_id->port=DEFAULT_CTL_PORT;
				/*
				fprintf(stderr, "ERROR: no port specified: %s:<port>\n",
								sock_name);
				goto error;
				*/
			}
			if ((s=connect_tcpudp_socket(sock_id->name, sock_id->port,
						(sock_id->proto==UDP_SOCK)?SOCK_DGRAM:
													SOCK_STREAM))<0){
				goto error;
			}
			break;
		case UNIXS_SOCK:
		case UNIXD_SOCK:
			if ((s=connect_unix_sock(sock_id->name,
						(sock_id->proto==UNIXD_SOCK)?SOCK_DGRAM:
													SOCK_STREAM))<0)
				goto error;
			break;
		case UNKNOWN_SOCK:
			fprintf(stderr, "ERROR: Bad socket type for %s\n", sock_name);
			goto error;
	}
	free(sock_id); /* not needed anymore */
	sock_id=0;
	
	if (optind>=argc){
			interactive=1;
			/*fprintf(stderr, "ERROR: no command specified\n");
			goto error; */
	}else{
		if (parse_cmd(&cmd, &argv[optind], argc-optind)<0)
			goto error;
		if (run_cmd(s, &cmd, format)<0) 
			goto error;
		goto end;
	}
	/* interactive mode */
	get_sercmd_list(s);
	/* banners */
	printf("%s %s\n", NAME, VERSION);
	printf("%s\n", COPYRIGHT);
	printf("%s\n", DISCLAIMER);
#ifdef USE_READLINE
	
	/* initialize readline */
	/* allow conditional parsing of the ~/.inputrc file*/
	rl_readline_name=NAME; 
	rl_completion_entry_function=sercmd_generator;
	rl_attempted_completion_function=sercmd_completion;
	
	while(!quit){
		line=readline(NAME "> ");
		if (line==0) /* EOF */
			break; 
		l=trim_ws(line); /* trim whitespace */
		if (*l){
			add_history(l);
			run_line(s, l, format);
		}
		free(line);
		line=0;
	}
#else
	line=malloc(MAX_LINE_SIZE);
	if (line==0){
		fprintf(stderr, "memory allocation error\n");
		goto error;
	}
	printf(NAME "> "); fflush(stdout); /* prompt */
	while(!quit && fgets(line, MAX_LINE_SIZE, stdin)){
		l=trim_ws(line);
		if (*l){
			run_line(s, l, format);
		}
		printf(NAME "> "); fflush(stdout); /* prompt */
	};
	free(line);
	line=0;
#endif /* USE_READLINE */
end:
	/* normal exit */
	if (line)
		free(line);
	if (format)
		free(format);
	if (rpc_array)
		free(rpc_array);
	cleanup();
	exit(0);
error:
	if (line)
		free(line);
	if (format)
		free(format);
	if (rpc_array)
		free(rpc_array);
	cleanup();
	exit(-1);
}
