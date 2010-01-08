/**
 * $Id: pcc.c,v 1.10 2007/03/14 16:18:28 Alberto Exp $
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
 * P-CSCF Policy and Charging Control interface - Gq'
 *  
 * \author Alberto Diez Albaladejo -at- fokus dot fraunhofer dot de
 */

#ifndef PCC_GQPRIMA_H_
#define PCC_GQPRIMA_H_
#include "dlg_state.h"
#include "../cdp/cdp_load.h"
#include "pcc_avp.h"

#define FL_APPEND(list,add)                                                      \
do {                                                                             \
  (add)->next = NULL;															 \
  (add)->prev = (list)->tail;													 \
  if ((list)->tail) {                                                            \
	  ((list)->tail)->next=(add);												 \
      (add)->next = NULL;                                                        \
  } else {                                                                       \
      (list)->head = (add);                                                      \
  }                                                                              \
  (list)->tail=(add);                                                        	 \
} while (0);

int gqprima_AAR(AAAMessage *aar,struct sip_msg *req, struct sip_msg *res, char *str1,int relatch);
int gqprima_AAA(AAAMessage *dia_msg);


typedef struct _t_binding_unit
{
	enum ip_type v;
	str addr;
	int port_start;
	int port_end; //only for multi-ports

	struct _t_binding_unit *prev,*next;
}t_binding_unit;

typedef struct _t_binding_list
{
	t_binding_unit *head,*tail;
}t_binding_list;



#endif /* PCC_GQPRIMA_H_ */
