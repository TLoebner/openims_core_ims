/*
 * $Id$
 *
 * Copyright (C) 2001-2003 FhG Fokus
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
 *  2003-03-19  replaced all mallocs/frees w/ pkg_malloc/pkg_free (andrei)
 *  2006-02-02  named flags support (andrei)
 */


#include <limits.h>
#include "sr_module.h"
#include "dprint.h"
#include "parser/msg_parser.h"
#include "flags.h"
#include "error.h"
#include "stdlib.h"
#include "hashes.h"

int setflag( struct sip_msg* msg, flag_t flag ) {
	msg->flags |= 1 << flag;
	return 1;
}

int resetflag( struct sip_msg* msg, flag_t flag ) {
	msg->flags &= ~ (1 << flag);
	return 1;
}

int isflagset( struct sip_msg* msg, flag_t flag ) {
	return (msg->flags & (1<<flag)) ? 1 : -1;
}

int flag_in_range( flag_t flag ) {
	if (flag > MAX_FLAG ) {
		LOG(L_ERR, "ERROR: message flag %d too high; MAX=%d\n",
			flag, MAX_FLAG );
		return 0;
	}
	if (flag<0) {
		LOG(L_ERR, "ERROR: message flag (%d) must be in range %d..%d\n",
			flag, 0, MAX_FLAG );
		return 0;
	}
	return 1;
}


/* use 2^k */
#define FLAGS_NAME_HASH_ENTRIES		32

struct flag_entry{
	struct flag_entry* next;
	struct flag_entry* prev;
	str name;
	int no;
};


struct flag_hash_head{
	struct flag_entry* next;
	struct flag_entry* prev;
};

static struct flag_hash_head  name2flags[FLAGS_NAME_HASH_ENTRIES];
static unsigned char registered_flags[MAX_FLAG+1];


void init_named_flags()
{
	int r;
	
	for (r=0; r<FLAGS_NAME_HASH_ENTRIES; r++)
		clist_init(&name2flags[r], next, prev);
}



/* returns 0 on success, -1 on error */
int check_flag(int n)
{
	if (!flag_in_range(n))
		return -1;
	if (registered_flags[n]){
		LOG(L_WARN, "WARNING: check_flag: flag %d is already used by "
					" a named flag\n", n);
	}
	return 0;
}


inline static struct flag_entry* flag_search(struct flag_hash_head* lst,
												char* name, int len)
{
	struct flag_entry* fe;
	
	clist_foreach(lst, fe, next){
		if ((fe->name.len==len) && (memcmp(fe->name.s, name, len)==0)){
			/* found */
			return fe;
		}
	}
	return 0;
}



/* returns flag entry or 0 on not found */
inline static struct flag_entry* get_flag_entry(char* name, int len)
{
	int h;
	/* get hash */
	h=get_hash1_raw(name, len) & (FLAGS_NAME_HASH_ENTRIES-1);
	return flag_search(&name2flags[h], name, len);
}



/* returns flag number, or -1 on error */
int get_flag_no(char* name, int len)
{
	struct flag_entry* fe;
	
	fe=get_flag_entry(name, len);
	return (fe)?fe->no:-1;
}



/* resgiter a new flag name and associates it with pos
 * pos== -1 => any position will do 
 * returns flag pos on success (>=0)
 *         -1  flag is an alias for an already existing flag
 *         -2  flag already registered
 *         -3  mem. alloc. failure
 *         -4  invalid pos
 *         -5 no free flags */
int register_flag(char* name, int pos)
{
	struct flag_entry* e;
	int len;
	unsigned int r;
	static unsigned int crt_flag=0;
	unsigned int last_flag;
	unsigned int h;
	
	len=strlen(name);
	h=get_hash1_raw(name, len) & (FLAGS_NAME_HASH_ENTRIES-1);
	/* check if the name already exists */
	e=flag_search(&name2flags[h], name, len);
	if (e){
		LOG(L_ERR, "ERROR: register_flag: flag %.*s already registered\n",
					len, name);
		return -2;
	}
	/* check if there is already another flag registered at pos */
	if (pos!=-1){
		if ((pos<0) || (pos>MAX_FLAG)){
			LOG(L_ERR, "ERROR: register_flag: invalid flag %.*s "
					"position(%d)\n", len, name, pos);
			return -4;
		}
		if (registered_flags[pos]!=0){
			LOG(L_WARN, "WARNING: register_flag:  %.*s:  flag %d already in "
					"use under another name\n", len, name, pos);
			/* continue */
		}
	}else{
		/* alloc an empty flag */
		last_flag=crt_flag+(MAX_FLAG+1);
		for (; crt_flag!=last_flag; crt_flag++){
			r=crt_flag%(MAX_FLAG+1);
			if (registered_flags[r]==0){
				pos=r;
				break;
			}
		}
		if (pos==-1){
			LOG(L_ERR, "ERROR: register_flag: could not register %.*s"
					" - too many flags\n", len, name);
			return -5;
		}
	}
	registered_flags[pos]++;
	
	e=pkg_malloc(sizeof(struct flag_entry));
	if (e==0){
		LOG(L_ERR, "ERROR: register_flag: memory allocation failure\n");
		return -3;
	}
	e->name.s=name;
	e->name.len=len;
	e->no=pos;
	clist_insert(&name2flags[h], e, next, prev);
	return pos;
}



#ifdef _GET_AWAY

/* wrapping functions for flag processing  */
static int fixup_t_flag(void** param, int param_no)
{
    unsigned int *code;
	char *c;
	int token;

	DBG("DEBUG: fixing flag: %s\n", (char *) (*param));

	if (param_no!=1) {
		LOG(L_ERR, "ERROR: TM module: only parameter #1 for flags can be"
					" fixed\n");
		return E_BUG;
	};

	if ( !(code =pkg_malloc( sizeof( unsigned int) )) ) return E_OUT_OF_MEM;

	*code = 0;
	c = *param;
	while ( *c && (*c==' ' || *c=='\t')) c++; /* initial whitespaces */

	token=1;
	if (strcasecmp(c, "white")==0) *code=FL_WHITE;
	else if (strcasecmp(c, "yellow")==0) *code=FL_YELLOW;
	else if (strcasecmp(c, "green")==0) *code=FL_GREEN;
	else if (strcasecmp(c, "red")==0) *code=FL_RED;
	else if (strcasecmp(c, "blue")==0) *code=FL_BLUE;
	else if (strcasecmp(c, "magenta")==0) *code=FL_MAGENTA;
	else if (strcasecmp(c, "brown")==0) *code=FL_BROWN;
	else if (strcasecmp(c, "black")==0) *code=FL_BLACK;
	else if (strcasecmp(c, "acc")==0) *code=FL_ACC;
	else {
		token=0;
		while ( *c && *c>='0' && *c<='9' ) {
			*code = *code*10+ *c-'0';
			if (*code > (sizeof( flag_t ) * CHAR_BIT - 1 )) {
				LOG(L_ERR, "ERROR: TM module: too big flag number: %s; MAX=%d\n",
					(char *) (*param), sizeof( flag_t ) * CHAR_BIT - 1 );
				goto error;
			}
			c++;
		}
	}
	while ( *c && (*c==' ' || *c=='\t')) c++; /* terminating whitespaces */

	if ( *code == 0 ) {
		LOG(L_ERR, "ERROR: TM module: bad flag number: %s\n", (char *) (*param));
		goto error;
	}

	if (*code < FL_MAX && token==0) {
		LOG(L_ERR, "ERROR: TM module: too high flag number: %s (%d)\n; lower number"
			" bellow %d reserved\n", (char *) (*param), *code, FL_MAX );
		goto error;
	}

	/* free string */
	pkg_free( *param );
	/* fix now */
	*param = code;
	
	return 0;

error:
	pkg_free( code );
	return E_CFG;
}


#endif
