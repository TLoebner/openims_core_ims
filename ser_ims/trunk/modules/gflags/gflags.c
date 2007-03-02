/*$Id$
 *
 * gflags module: global flags; it keeps a bitmap of flags
 * in shared memory and may be used to change behaviour
 * of server based on value of the flags. E.g.,
 *    if (is_gflag("1")) { t_relay_to_udp("10.0.0.1","5060"); }
 *    else { t_relay_to_udp("10.0.0.2","5060"); }
 * The benefit of this module is the value of the switch flags
 * can be manipulated by external applications such as web interface
 * or command line tools.
 *
 *
 * Copyright (C) 2004 FhG FOKUS
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
 * History:
 * --------
 *  2004-09-09  initial module created (jiri)
 *  2006-12-04  added xmlrpc command global.reload for reloading of global_attrs
 *              db table (vlada)
 *
 * TODO
 * - flag range checking
 * - named flags (takes a protected name list)
 */


#include <stdio.h>
#include "../../sr_module.h"
#include "../../error.h"
#include "../../ut.h"
#include "../../db/db.h"
#include "../../mem/mem.h"
#include "../../mem/shm_mem.h"
#include "../../usr_avp.h"
#include "../../rpc.h"
#include "../../config.h"


MODULE_VERSION

static int set_gflag(struct sip_msg*, char *, char *);
static int reset_gflag(struct sip_msg*, char *, char *);
static int is_gflag(struct sip_msg*, char *, char *);
static int flush_gflags(struct sip_msg*, char*, char*);

static int mod_init(void);
static void mod_destroy(void);
static int child_init(int rank);
static int reload_global_attributes(void);

static int initial = 0;
static unsigned int *gflags;

static char* db_url = DEFAULT_DB_URL;
static int   load_global_attrs = 0;
static char* attr_table = "global_attrs";
static char* attr_name = "name";
static char* attr_type = "type";
static char* attr_value = "value";
static char* attr_flags = "flags";

static db_con_t* con = 0;
static db_func_t db;

static avp_list_t* active_global_avps;
static avp_list_t avps_1;
static avp_list_t avps_2;
static rpc_export_t rpc_methods[];

static cmd_export_t cmds[]={
	{"set_gflag",   set_gflag,   1, fixup_int_1, REQUEST_ROUTE|FAILURE_ROUTE|ONREPLY_ROUTE},
	{"reset_gflag", reset_gflag, 1, fixup_int_1, REQUEST_ROUTE|FAILURE_ROUTE|ONREPLY_ROUTE},
	{"is_gflag",    is_gflag,    1, fixup_int_1, REQUEST_ROUTE|FAILURE_ROUTE|ONREPLY_ROUTE},
	{"flush_gflags", flush_gflags, 0, 0,         REQUEST_ROUTE|FAILURE_ROUTE|ONREPLY_ROUTE},
	{0, 0, 0, 0, 0}
};

static param_export_t params[]={
	{"initial",            PARAM_INT,    &initial          },
	{"db_url",             PARAM_STRING, &db_url           },
	{"load_global_attrs",  PARAM_INT,    &load_global_attrs},
	{"global_attrs_table", PARAM_STRING, &attr_table       },
	{"global_attrs_name",  PARAM_STRING, &attr_name        },
	{"global_attrs_type",  PARAM_STRING, &attr_type        },
	{"global_attrs_value", PARAM_STRING, &attr_value       },
	{"global_attrs_flags", PARAM_STRING, &attr_flags       },
	{0, 0, 0}
};

struct module_exports exports = {
	"gflags",
	cmds,
	rpc_methods, /* RPC methods */
	params,
	mod_init,    /* module initialization function */
	0,           /* response function*/
	mod_destroy, /* destroy function */
	0,           /* oncancel function */
	child_init   /* per-child init function */
};


static int set_gflag(struct sip_msg *bar, char *flag_par, char *foo)
{
	unsigned long int flag;

	if (!flag_par || ((fparam_t*)flag_par)->type != FPARAM_INT) {
	  LOG(L_ERR, "gflags:set_gflag: Invalid parameter\n");
	  return -1;
	}
	
	flag=((fparam_t*)flag_par)->v.i; 

	(*gflags) |= 1 << flag;
	return 1;
}

static int reset_gflag(struct sip_msg *bar, char *flag_par, char *foo)
{
	unsigned long int flag;

	if (!flag_par || ((fparam_t*)flag_par)->type != FPARAM_INT) {
	  LOG(L_ERR, "gflags:reset_gflag: Invalid parameter\n");
	  return -1;
	}
	
	flag=((fparam_t*)flag_par)->v.i; 
	(*gflags) &= ~ (1 << flag);
	return 1;
}

static int is_gflag(struct sip_msg *bar, char *flag_par, char *foo)
{
	unsigned long int flag;

	if (!flag_par || ((fparam_t*)flag_par)->type != FPARAM_INT) {
	  LOG(L_ERR, "gflags:is_gflag: Invalid parameter\n");
	  return -1;
	}
	
	flag=((fparam_t*)flag_par)->v.i; 
	return ( (*gflags) & (1<<flag)) ? 1 : -1;
}


/*
 * Load attributes from global_attrs table
 */
static int load_attrs(avp_list_t* global_avps)
{
	int_str name, v;

	str avp_name, avp_val;
	int i, type, n;
	db_key_t cols[4];
	db_res_t* res;
	db_val_t *val;
	unsigned short flags;

	if (!con) {
		LOG(L_ERR, "gflags:load_attrs: Invalid database handle\n");
		return -1;
	}

	cols[0] = attr_name;
	cols[1] = attr_type;
	cols[2] = attr_value;
	cols[3] = attr_flags;

	if (db.use_table(con, attr_table) < 0) {
		LOG(L_ERR, "gflags:load_attrs: Error in use_table\n");
		return -1;
	}

	if (db.query(con, 0, 0, 0, cols, 0, 4, 0, &res) < 0) {
		LOG(L_ERR, "gflags:load_attrs: Error while quering database\n");
		return -1;
	}

	n = 0;
	for(i = 0; i < res->n; i++) {
		val = res->rows[i].values;

		if (val[0].nul || val[1].nul || val[3].nul) {
			LOG(L_ERR, "gflags:load_attrs: Skipping row containing NULL entries\n");
			continue;
		}

		if ((val[3].val.int_val & DB_LOAD_SER) == 0) continue;

		n++;
		     /* Get AVP name */
		avp_name.s = (char*)val[0].val.string_val;
		avp_name.len = strlen(avp_name.s);
		name.s = avp_name;

		     /* Get AVP type */
		type = val[1].val.int_val;

		     /* Test for NULL value */
		if (val[2].nul) {
			avp_val.s = 0;
			avp_val.len = 0;
		} else {
			avp_val.s = (char*)val[2].val.string_val;
			avp_val.len = strlen(avp_val.s);
		}

		flags = AVP_CLASS_GLOBAL | AVP_NAME_STR;
		if (type == AVP_VAL_STR) {
			     /* String AVP */
			v.s = avp_val;
			flags |= AVP_VAL_STR;
		} else {
			     /* Integer AVP */
			str2int(&avp_val, (unsigned*)&v.n);
			if (!strcmp(AVP_GFLAGS, avp_name.s)) {
				     /* Restore gflags */
				*gflags = v.n;
			}
		}

		if (add_avp_list(global_avps, flags, name, v) < 0) {
			LOG(L_ERR, "gflags:load_attrs: Error while adding global attribute %.*s, skipping\n",
			    avp_name.len, ZSW(avp_name.s));
			continue;
		}
	}
	DBG("gflags:load_attrs: %d global attributes found, %d loaded\n", res->n, n);
	db.free_result(con, res);
	return 0;
}


static int mod_init(void)
{
	gflags=(unsigned int *) shm_malloc(sizeof(unsigned int));
	if (!gflags) {
		LOG(L_ERR, "Error: gflags/mod_init: no shmem\n");
		return -1;
	}
	*gflags=initial;

	avps_1 = 0;
	avps_2 = 0;
	active_global_avps = &avps_1;
	
	if (load_global_attrs) {
		if (bind_dbmod(db_url, &db) < 0) { /* Find database module */
			LOG(L_ERR, "gflags:mod_init: Can't bind database module\n");
			return -1;
		}
		if (!DB_CAPABILITY(db, DB_CAP_ALL)) {
			LOG(L_ERR, "gflags:mod_init: Database module does not implement"
			    " all functions needed by the module\n");
			return -1;
		}
		con = db.init(db_url); /* Get a new database connection */
		if (!con) {
			LOG(L_ERR, "gflags:mod_init: Error while connecting database\n");
			return -1;
		}

		if (load_attrs(active_global_avps) < 0) {
			db.close(con);
			return -1;
		}

		set_avp_list(AVP_CLASS_GLOBAL, active_global_avps);

		db.close(con);
	}

	return 0;
}

static int child_init(int rank)
{
	if (load_global_attrs) {
		if (rank==PROC_MAIN || rank==PROC_TCP_MAIN)
			return 0; /* do nothing for the main or tcp_main processes */
		con = db.init(db_url);
		if (!con) {
			LOG(L_ERR, "gflags:mod_init: Error while connecting database\n");
			return -1;
		}
	}
	return 0;
}


static void mod_destroy(void)
{
	if (avps_1 != 0) {
		destroy_avp_list(&avps_1);
	}
	if (avps_2 != 0) {
		destroy_avp_list(&avps_2);
	}
	active_global_avps = 0;
}


int save_gflags(unsigned int flags)
{
	db_key_t keys[4];
	db_val_t vals[4];
        str fl;

	if (!load_global_attrs) {
		LOG(L_ERR, "gflags:save_gflags: You must enable load_global_attrs to make flush_gflag work\n");
		return -1;
	}

	if (db.use_table(con,attr_table) < 0) {
		LOG(L_ERR, "gflags:save_gflags: Error in use_table\n");
		return -1;
	}

	keys[0] = attr_name;
	vals[0].type = DB_STRING;
	vals[0].nul = 0;
	vals[0].val.string_val = AVP_GFLAGS;

	if (db.delete(con, keys, 0, vals, 1) < 0) {
		LOG(L_ERR, "gflags:save_gflag: Error while deleting previous value\n");
		return -1;
	}

	keys[1] = attr_type;
	keys[2] = attr_value;
	keys[3] = attr_flags;

	vals[1].type = DB_INT;
	vals[1].nul = 0;
	vals[1].val.int_val = 0; /* Integer */

	fl.s = int2str(flags, &fl.len);

	vals[2].type = DB_STR;
	vals[2].nul = 0;
	vals[2].val.str_val = fl;

	vals[3].type = DB_INT;
	vals[3].nul = 0;
	vals[3].val.int_val = DB_LOAD_SER;

	if (db.insert(con, keys, vals, 4) < 0) {
		LOG(L_ERR, "gflags:save_gflag: Unable to store new value\n");
		return -1;
	}

	DBG("gflags:save_gflags: Successfuly stored in database\n");
	return 0;
}

static int reload_global_attributes(void)
{
	avp_list_t*  new_global_avps;
  
  /* Choose new global AVP list and free its old contents */
  if (active_global_avps == &avps_1) {
  	destroy_avp_list(&avps_2);
		new_global_avps = &avps_2;
	} 
	else {
		destroy_avp_list(&avps_1);
		new_global_avps = &avps_1;
	}
    
  if (load_attrs(new_global_avps) < 0) {
  	goto error;
  }
  
  active_global_avps = new_global_avps;

  return 0;
    
error:
	destroy_avp_list(new_global_avps);
  return -1;
}

/*
 * Flush the state of global flags into database
 */
static int flush_gflags(struct sip_msg* msg, char* s1, char* s2)
{
	if (save_gflags(*gflags) < 0)  return -1;
	else return 1;
}


static const char* rpc_set_doc[] = {
	"Load a CPL script to the server.", /* Documentation string */
	0                                   /* Method signature(s) */
};

static void rpc_set(rpc_t* rpc, void* c)
{
        int flag;

	if (rpc->scan(c, "d", &flag) < 1) {
		rpc->fault(c, 400, "Flag number expected");
		return;
	}
	if (flag < 0 || flag > 31) {
		rpc->fault(c, 400, "Flag number %d out of range", &flag);
	}
	(*gflags) |= 1 << flag;
}


static const char* rpc_is_set_doc[] = {
	"Load a CPL script to the server.", /* Documentation string */
	0                                   /* Method signature(s) */
};

static void rpc_is_set(rpc_t* rpc, void* c)
{
        int flag;

	if (rpc->scan(c, "d", &flag) < 1) {
		rpc->fault(c, 400, "Flag number expected");
		return;
	}
	if (flag < 0 || flag > 31) {
		rpc->fault(c, 400, "Flag number %d out of range", &flag);
	}
	rpc->add(c, "b", (*gflags) & (1 << flag));
}


static const char* rpc_reset_doc[] = {
	"Load a CPL script to the server.", /* Documentation string */
	0                                   /* Method signature(s) */
};

static void rpc_reset(rpc_t* rpc, void* c)
{
        int flag;

	if (rpc->scan(c, "d", &flag) < 1) {
		rpc->fault(c, 400, "Flag number expected");
		return;
	}
	if (flag < 0 || flag > 31) {
		rpc->fault(c, 400, "Flag number %d out of range", &flag);
	}
	(*gflags) &= ~ (1 << flag);
}


static const char* rpc_flush_doc[] = {
	"Load a CPL script to the server.", /* Documentation string */
	0                                   /* Method signature(s) */
};

static void rpc_flush(rpc_t* rpc, void* c)
{
	if (flush_gflags(0, 0, 0) < 0) {
		rpc->fault(c, 400, "Error while saving flags to database");
	}
}


static const char* rpc_dump_doc[] = {
	"Load a CPL script to the server.", /* Documentation string */
	0                                   /* Method signature(s) */
};

static void rpc_dump(rpc_t* rpc, void* c)
{
        int i;
	for(i = 0; i < 32; i++) {
		rpc->add(c, "b", (*gflags >> i) & 1);
	}
}

static const char* rpc_reload_doc[2] = {
	"Reload global attributes from database",
	0
};

/*
 * Fifo function to reload domain table
 */
static void rpc_reload(rpc_t* rpc, void* ctx)
{
	if (reload_global_attributes() < 0) {
		LOG(L_ERR, "ERROR: Reloading of global_attrs table has failed\n");
		rpc->fault(ctx, 400, "Reloading of global attributes failed");
	}
	else {
		/* reload is successful */
		LOG(L_INFO, "INFO: global_attrs table reloaded\n");
	}
}

/*
 * RPC Methods exported by this module
 */
static rpc_export_t rpc_methods[] = {
	{"gflags.set",    rpc_set,    rpc_set_doc,    0},
	{"gflags.is_set", rpc_is_set, rpc_is_set_doc, 0},
	{"gflags.reset",  rpc_reset,  rpc_reset_doc,  0},
	{"gflags.flush",  rpc_flush,  rpc_flush_doc,  0},
	{"gflags.dump",   rpc_dump,   rpc_dump_doc,   0},
	{"global.reload", rpc_reload, rpc_reload_doc, 0},
	{0, 0, 0, 0}
};
