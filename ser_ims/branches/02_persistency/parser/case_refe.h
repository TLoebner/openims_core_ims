/*
 * Refer-To Header Field Name Parsing Macros
 *
 * Refer-To, Referred-By Header Field Name Parsing Macros
 *
 * Copyright (C) 2005 Juha Heinanen
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

#ifndef CASE_REFE_H
#define CASE_REFE_H

#define REFERRED_BY_CASE                 \
        switch(LOWER_DWORD(val)) {       \
        case __by1_:                     \
	        hdr->type = HDR_REFERREDBY_T; \
	        hdr->name.len = 11;      \
	        return (p + 4);          \
        case __by2_:                      \
	        hdr->type = HDR_REFERREDBY_T; \
	        p += 4;                  \
	        goto dc_end;             \
	}

#define r_to_CASE                        \
        switch(LOWER_DWORD(val)) {       \
        case _r_to_:  \
                hdr->type = HDR_REFER_TO_T;  \
                p += 4;                  \
                goto dc_end;             \
        \
        case _rred_:                     \
                p += 4;                  \
                val = READ(p);           \
                REFERRED_BY_CASE;        \
                goto other;              \
        }

#define refe_CASE      \
     p += 4;           \
     val = READ(p);    \
     r_to_CASE;        \
     goto other;


#endif /* CASE_REFE_H */
