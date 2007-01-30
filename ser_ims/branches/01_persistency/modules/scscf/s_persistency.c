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
 * S-CSCF persistency operations
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */

#include <time.h>

#include "bin_scscf.h"

int bin_dump_to_file(bin_data *x,char *prep_fname)
{
	char c[256];
	time_t now;
	FILE *f;
	int k;
	
	now = time(0);
	sprintf(c,"%s_%u.bin",prep_fname,(unsigned int)now);
	f = fopen(c,"w");
	k = fwrite(x->s,1,x->len,f);
	LOG(L_INFO,"INFO:"M_NAME":bin_dump_to_file: Dumped %d bytes into %s.\n",k,c);
	fclose(f);	
	bin_print(x);
	if (k==x->len) return 1;
	else return 0;
}


int snapshot_auth(auth_data *ad)
{
	bin_data x;
	auth_userdata *aud;
	int i;
	
	if (!bin_alloc(&x,256)) goto error;
	
	for(i=0;i<ad->size;i++){
		aud = ad->table[i].head;
		while(aud){
			if (!bin_encode_auth_userdata(&x,aud)) goto error;
			bin_print(&x);
			aud = aud->next;
		}
	}
	i = bin_dump_to_file(&x,"/opt/OpenIMSCore/authdata");
	bin_free(&x);
	return i;
error:
	return 0;
}  


