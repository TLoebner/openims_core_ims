/*
 * $Id: dlg_state.c 95 2007-01-19 16:19:58Z vingarzan $
 *
 * Copyright (C) 2004-2007 FhG Fokus
 *
 * This file is part of Open IMS Core - an open source IMS CSCFs & HSS
 * implementation
 *
 * Open IMS Core is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * For a license to use the Open IMS Core software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact Fraunhofer FOKUS by e-mail at the following
 * addresses:
 *     info@open-ims.org
 *
 * Open IMS Core is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * It has to be noted that this Open Source IMS Core System is not
 * intended to become or act as a product in a commercial context! Its
 * sole purpose is to provide an IMS core reference implementation for
 * IMS technology testing and IMS application prototyping for research
 * purposes, typically performed in IMS test-beds.
 *
 * Users of the Open Source IMS Core System have to be aware that IMS
 * technology may be subject of patents and licence terms, as being
 * specified within the various IMS-related IETF, ITU-T, ETSI, and 3GPP
 * standards. Thus all Open IMS Core users have to take notice of this
 * fact and have to agree to check out carefully before installing,
 * using and extending the Open Source IMS Core System, if related
 * patents and licences may become applicable to the intended usage
 * context.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/**
 * \file
 *
 * Binary codec operations
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de

The binary form of the subscription is a vector of bytes and has the same type as
str from SER.

Basic datatype representations:

- Each integer is written as litter endian on a specific width(1->4 bytes)
        int(k,2):
                k & 0x00ff
                k & 0xff00 >>8


- Each str is written as:
        str(s):
                int(s.len,2)
                s.s[0]
                s.s[1]
                .
                .
                .
                s.s[len-1]

- Each regex is written as:
        regex(r):
                int(sizeof(r),2)
                &r[0]
                &r[1]
                .
                .
                .
                &r[len-1]

- Each pointer is a 4 byte offset into the binary form.
        ptr(p)
                int(p,4)

- Array of pointers
        array(p,len)
                int(len,4)
                int(p[1],4)                                     - p[0] can be computed = &array + 4*(len+1)
                int(p[2],4)
                .
                .
                int(p[len],4)                           - the ptr of the next byte after the array








        IMSSubscription:
                str(private_id)
                array(service_profile,service_profile_cnt)


        service_profile:
                array(public_identity,public_identity_cnt)
                array(filter_criteria,filter_criteria_cnt)
                int(cn_service_auth,4)  - optional, if eq 0XFFFFFF means not specified


        public_identity:
                int(barring_indication,1)
                str(identity)

        filter_criteria:
                int(priority,4)
                int(pofile_part_indicator,1)            - ppindicator+1 : if eq 0 then null
                int( condition_type_cnf,1)              - if eq 0xff means no filter_criteria
                array(spt,spt_cnt)


        spt:
                int(condition_negated<<7 | registration_type<<4 | type,1)
                int(group,2)

                str(request_uri)                                - if type==1

                str(method)                                             - if type==2

                int(header->type,1)                             - if type==3
                str(header->header)                             - if type==3
                str(header->content)                    - if type==3
                regex(header->content_comp)             - if type==3

                int(session_case,1)                             - if type==4

                str(session_desc->line)                 - if type==5
                str(session_desc->content)              - if type==5
                regex(session_desc->content_comp)- if type==5
 *
 */



#ifndef _IFC_BIN_H
#define _IFC_BIN_H

#include "ifc_datastruct.h"
#include "../../str.h"


/**
 *			LOADING/SAVING FROM BINARY FORMAT INTO STRUCTURES
 *		- please see BinaryFormat.txt for more info on this
 **/

/**
 *	Array representation
 */
struct _ifc_bin_array
{
	int cnt;				/* Member count	*/
	ifc_ims_bin *get;		/* Member data, concatenated */
};
typedef struct _ifc_bin_array ifc_bin_array;



/**
 *	Encode and Append an entire Filter Criteria
 */
ifc_ims_bin ifc_bin_encode_filter_criteria(ifc_filter_criteria *fc);
/**
 *	Encode the entire user profile and return the pointer to the binary data
 */
ifc_ims_bin *ifc_bin_encode(ifc_ims_subscription *s);

/**
 *	simple print function 
 */
void ifc_bin_print(ifc_ims_bin *imsb);


/**
 *	Decode an entire Filter Criteria
 */
ifc_filter_criteria ifc_bin_decode_filter_criteria(ifc_ims_bin *s);
/**
 *	Decode a binary string into an IMSSubcription data structure 
 */
ifc_ims_subscription *ifc_bin_decode(ifc_ims_bin *imsb);





/*
 *		Binary encoding functions
 */
/* memory allocation and initialization macros */
#define IFC_BIN_ALLOC_METHOD    shm_malloc
#define IFC_BIN_REALLOC_METHOD  shm_realloc
#define IFC_BIN_FREE_METHOD     shm_free

#define IFC_BIN_ALLOC(X) {                                \
	(X) = (ifc_ims_bin*)IFC_BIN_ALLOC_METHOD(sizeof(ifc_ims_bin));     \
    (X)->s=0;(X)->len=0;\
}

//(X)->s = (char*)IFC_BIN_ALLOC_METHOD(1024);(X)->len=0;


#define IFC_BIN_REALLOC(X,DELTA) {        \
	DBG("DEBUG: realloc %p from %d to + %d\n",(X)->s,(X)->len,(DELTA));\
	(X)->s=IFC_BIN_REALLOC_METHOD((X)->s,(X)->len + (DELTA));    \
	DBG("DEBUG: realloc ok %p from %d to + %d\n",(X)->s,(X)->len,(DELTA));\
	if ((X)->s==NULL)                             \
		LOG(L_ERR,"No more memory to expand %d with %d  \n",(X)->len,(DELTA));\
}
#define IFC_BIN_FREE(X) {        \
	IFC_BIN_FREE_METHOD((X)->s);(X)->s=0;(X)->len=0; \
}







#endif
