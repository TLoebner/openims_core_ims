/* 
 * $Id$ 
 *
 * Identity, Identity-info Header Field Name Parsing Macros
 *
 * Copyright (c) 2007 iptelorg GmbH
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
 * --------
 */


#ifndef CASE_IDEN_H
#define CASE_IDEN_H

#include "../comp_defs.h"
	      

#define INFO_CASE                                     \
        switch(LOWER_DWORD(val)) {                    \
        case _info_:                                  \
                hdr->type = HDR_IDENTITY_INFO_T;       \
                p += 4;                               \
	        goto dc_end;                          \
        }


#define TITY_CASE                                     \
        switch(LOWER_DWORD(val)) {                    \
        case _tity_:                                  \
                p += 4;                               \
                switch(LOWER_BYTE(*p)) {              \
                case ':':                             \
                case ' ':                             \
                        hdr->type = HDR_IDENTITY_T;   \
                        goto dc_end;                  \
		case '-':                             \
	                p++;                          \
    			val = READ(p);                \
		        INFO_CASE;                    \
		}                                     \
                goto other;                           \
        }


#define iden_CASE         \
        p += 4;           \
        val = READ(p);    \
        TITY_CASE;        \
        goto other;


#endif /* CASE_PROX_H */
