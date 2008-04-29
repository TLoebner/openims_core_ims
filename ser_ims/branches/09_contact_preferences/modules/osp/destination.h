/*
 * ser osp module. 
 *
 * This module enables ser to communicate with an Open Settlement 
 * Protocol (OSP) server.  The Open Settlement Protocol is an ETSI 
 * defined standard for Inter-Domain VoIP pricing, authorization
 * and usage exchange.  The technical specifications for OSP 
 * (ETSI TS 101 321 V4.1.1) are available at www.etsi.org.
 *
 * Uli Abend was the original contributor to this module.
 * 
 * Copyright (C) 2001-2005 Fhg Fokus
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


#ifndef OSP_MOD_DESTINATION_H
#define OSP_MOD_DESTINATION_H

#include "../../str.h"

struct _osp_dest {
	char validafter[100];
	char validuntil[100];
	char callid[100];
	char callednumber[100];
	char callingnumber[100];
	char source[100];
	char sourcedevice[100];
	char destination[100];
	char destinationdevice[100];
	char network_id[100];
	char osptoken[2000];
	unsigned int sizeofcallid;
	unsigned int timelimit;
	unsigned int sizeoftoken;
	int  used;
	int  last_code;
	time_t time_auth;
	time_t time_100;
	time_t time_180;
	time_t time_200;
	unsigned long long tid;
	int type;
	int reported;
};

typedef struct _osp_dest osp_dest;

osp_dest* initDestination(osp_dest* dest);

osp_dest* getNextOrigDestination();
osp_dest* getTermDestination();


int       saveOrigDestination(osp_dest* dest);
int       saveTermDestination(osp_dest* dest);

void	  recordEvent(int client_code, int server_code);

void      dumpDebugInfo();
void      dumbDestDebugInfo(osp_dest *dest);
#endif
