/*
 * $Id$
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
 * Binary codec operations for the S-CSCF
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de

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



#ifndef _BIN_SCSCF_H
#define _BIN_SCSCF_H

#include "bin.h"

#define BIN_INITIAL_ALLOC_SIZE 256

#endif
