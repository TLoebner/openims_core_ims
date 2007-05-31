/*
 * $Id$
 *
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
 *
 * History:
 * -------
 * 2003-03-20  regex support in modparam (janakj)
 * 2004-03-12  extra flag USE_FUNC_PARAM added to modparam type -
 *             instead of copying the param value, a func is called (bogdan)
 * 2005-07-01  PARAM_STRING & PARAM_STR support
 */


#include "modparam.h"
#include "dprint.h"
#include "mem/mem.h"
#include <sys/types.h>
#include <regex.h>
#include <string.h>

int set_mod_param(char* _mod, char* _name, modparam_t _type, void* _val)
{
	return set_mod_param_regex(_mod, _name, _type, _val);
}

int set_mod_param_regex(char* regex, char* name, modparam_t type, void* val)
{
	struct sr_module* t;
	regex_t preg;
	int mod_found, len;
	char* reg;
	void *ptr, *val2;
	modparam_t param_type;
	str s;

	if (!regex) {
		LOG(L_ERR, "set_mod_param_regex(): Invalid mod parameter value\n");
		return -5;
	}
	if (!name) {
		LOG(L_ERR, "set_mod_param_regex(): Invalid name parameter value\n");
		return -6;
	}

	len = strlen(regex);
	reg = pkg_malloc(len + 2 + 1);
	if (reg == 0) {
		LOG(L_ERR, "set_mod_param_regex(): No memory left\n");
		return -1;
	}
	reg[0] = '^';
	memcpy(reg + 1, regex, len);
	reg[len + 1] = '$';
	reg[len + 2] = '\0';

	if (regcomp(&preg, reg, REG_EXTENDED | REG_NOSUB | REG_ICASE)) {
		LOG(L_ERR, "set_mod_param_regex(): Error while compiling regular expression\n");
		pkg_free(reg);
		return -2;
	}

	mod_found = 0;
	for(t = modules; t; t = t->next) {
		if (regexec(&preg, t->exports->name, 0, 0, 0) == 0) {
			DBG("set_mod_param_regex: '%s' matches module '%s'\n", regex, t->exports->name);
			mod_found = 1;
			/* PARAM_STR (PARAM_STRING) may be assigned also to PARAM_STRING(PARAM_STR) so let get both module param */
			ptr = find_param_export(t, name, type | ((type & (PARAM_STR|PARAM_STRING))?PARAM_STR|PARAM_STRING:0), &param_type);
			if (ptr) {
				     /* type casting */
				if (type == PARAM_STRING && PARAM_TYPE_MASK(param_type) == PARAM_STR) {
					s.s = (char*)val;
					s.len = s.s ? strlen(s.s) : 0;
					val2 = &s;
				} else if (type == PARAM_STR && PARAM_TYPE_MASK(param_type) == PARAM_STRING) {
					val2 = s.s;	/* zero terminator expected */
				} else {
					val2 = val;
				}
				DBG("set_mod_param_regex: found <%s> in module %s [%s]\n", name, t->exports->name, t->path);
				if (param_type & PARAM_USE_FUNC) {
					if ( ((param_func_t)(ptr))(param_type, val2) < 0) {
						regfree(&preg);
						pkg_free(reg);
						return -4;
					}
				}
				else {
					switch(PARAM_TYPE_MASK(param_type)) {
						case PARAM_STRING:
							*((char**)ptr) = pkg_malloc(strlen((char*)val2)+1);
							if (!*((char**)ptr)) {
								LOG(L_ERR, "set_mod_param_regex(): No memory left\n");
								return -1;
							}
							strcpy(*((char**)ptr), (char*)val2);
							break;

						case PARAM_STR:
							((str*)ptr)->s = pkg_malloc(((str*)val2)->len+1);
							if (!((str*)ptr)->s) {
								LOG(L_ERR, "set_mod_param_regex(): No memory left\n");
								return -1;
							}
							memcpy(((str*)ptr)->s, ((str*)val2)->s, ((str*)val2)->len);
							((str*)ptr)->len = ((str*)val2)->len;
							((str*)ptr)->s[((str*)ptr)->len] = 0;
							break;

						case PARAM_INT:
							*((int*)ptr) = (int)(long)val2;
							break;
					}
				}
			}
			else {
				LOG(L_ERR, "set_mod_param_regex: parameter <%s> not found in module <%s>\n",
				    name, t->exports->name);
				regfree(&preg);
				pkg_free(reg);
				return -3;
			}
		}
	}

	regfree(&preg);
	pkg_free(reg);
	if (!mod_found) {
		LOG(L_ERR, "set_mod_param_regex: No module matching <%s> found\n", regex);
		return -4;
	}
	return 0;
}
