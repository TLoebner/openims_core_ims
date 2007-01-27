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
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../str.h"
#include "../../mem/mem.h"
#include "../../mem/shm_mem.h"
#include "../../dprint.h"

#include "ifc_bin.h"
#include "ifc_datastruct.h"


/* basic data type reprezentation functions */

/**
 *	Append an integer of 1 byte 
 */
#define IFC_BIN_APPEND_INT1(X,K) { \
	IFC_BIN_REALLOC((X),1);\
	if ((K)>0xff) \
		LOG(L_ERR,"Possible loss in encoding (int > 0xff bytes) %d bytes \n",(K));\
	(X)->s[(X)->len++]=(K) & 0x000000FF;    \
}

/**
 *	Append an integer of 2 bytes 
 */
#define IFC_BIN_APPEND_INT2(X,K) { \
	IFC_BIN_REALLOC((X),2);\
	if ((K)>0xffff) \
		LOG(L_ERR,"Possible loss in encoding (int > 0xffff bytes) %d bytes \n",(K));\
	(X)->s[(X)->len++]=(K) & 0x000000FF;    \
	(X)->s[(X)->len++]=(K) & 0x0000FF00 >> 8;    \
}

/**
 *	Append an integer of 4 byte, without expansion of memory
 *	Obs: must have enough space in X->s to do this
 */
#define IFC_BIN_APPEND_INT4_NOALLOC( X , K ) { \
	if ((K)>0xffffffff) \
		LOG(L_ERR,"Possible loss in encoding (int > 0xffffffff bytes) %d bytes \n",(K));\
	(X)->s[(X)->len++]= (K) & 0x000000FF;          \
	(X)->s[(X)->len++]=((K) & 0x0000FF00) >> 8;    \
	(X)->s[(X)->len++]=((K) & 0x00FF0000) >>16;    \
	(X)->s[(X)->len++]=((K) & 0xFF000000) >>24;    \
}
/**
 *	Append an integer of 4 bytes with memory expansion 
 */
#define IFC_BIN_APPEND_INT4( X , K ) { \
    IFC_BIN_REALLOC((X),4);\
    IFC_BIN_APPEND_INT4_NOALLOC( X , K ) \
}

/**
 *	Append a string 
 */
#define IFC_BIN_APPEND_STR( X , S ) { \
	IFC_BIN_REALLOC((X),2+(S).len); \
	if ((S).len>65535)          \
	    LOG(L_ERR,"Possible loss of characters in encoding (string > 65535bytes) %d bytes \n",(S).len);\
	(X)->s[(X)->len++]=(S).len & 0x000000FF;\
	(X)->s[(X)->len++]=((S).len & 0x0000FF00)>>8;\
	memcpy((X)->s+(X)->len,(S).s,(S).len);\
	(X)->len+=(S).len;\
}

/**
 *	Append a regular expression
 */
#define IFC_BIN_APPEND_REGEX(X,R) {\
	int len;    \
	len = sizeof(*(R));\
	IFC_BIN_REALLOC((X),2+len);\
	if (len>65535)\
		LOG(L_ERR,"Possible loss of characters in encoding (regex > 65535bytes) %d bytes \n",len);\
	(X)->s[(X)->len++]=len & 0x000000FF;\
	(X)->s[(X)->len++]=(len & 0x0000FF00)>>8;\
	memcpy((X)->s+(X)->len,(R),len);\
	(X)->len+=len;\
}


/* advanced data type reprezentation functions */
/**
 *	Append Y to the end of X
 */
#define IFC_BIN_APPEND_BIN(X,Y) {\
	IFC_BIN_REALLOC( (X) , (X)->len + (Y)->len);\
	memcpy( (X)->s + (X)->len , (Y)->s , (Y)->len);\
	(X)->len += (Y)->len;\
}
/**
 *	Encode and append an array
 */
static void ifc_bin_append_array(ifc_ims_bin *x,ifc_ims_bin *a,int cnt)
{
	int offset,i;
	offset=x->len+4*(cnt+1);
    IFC_BIN_REALLOC( x , 4*(cnt+1));
	IFC_BIN_APPEND_INT4_NOALLOC( x , cnt);
	DBG("DEBUG:ifc:ifc_bin_encode_array: array size %d appended\n",cnt);

	for(i=0;i<cnt;i++){
		offset+=a[i].len;
		DBG("DEBUG:ifc:ifc_bin_encode_array: array index %d = %d appended\n",i,offset);
		IFC_BIN_APPEND_INT4_NOALLOC(x,offset);
		DBG("DEBUG:ifc:ifc_bin_encode_array: array index %d = %d appended\n",i,offset);
	}
	DBG("DEBUG:ifc:ifc_bin_encode_array: array indexes appended\n");

	for(i=0;i<cnt;i++){
		IFC_BIN_APPEND_BIN(x,&(a[i]));
		IFC_BIN_FREE(&(a[i]));
	}
	DBG("DEBUG:ifc:ifc_bin_encode_array: array elements appended\n");

}

/* ifc_structures reprezentation functions */

/**
 *	Encode and append a SPT
 */
static ifc_ims_bin ifc_bin_encode_spt(ifc_spt *spt)
{
	ifc_ims_bin x;
	x.s=0;x.len=0;
	IFC_BIN_APPEND_INT1(&x,spt->condition_negated<<7 | spt->registration_type<<4 | spt->type);
	DBG("DEBUG:ifc:ifc_bin_encode_spt: condition_negated %d & type %d appended\n",
	    spt->condition_negated,spt->type);

	IFC_BIN_APPEND_INT2(&x,spt->group);
	DBG("DEBUG:ifc:ifc_bin_encode_spt: group %d appended\n",spt->group);

	switch(spt->type){
		case 1:
			IFC_BIN_APPEND_STR(&x,spt->request_uri);
			DBG("DEBUG:ifc:ifc_bin_encode_spt: RequestUri appended\n");
			break;
		case 2:
			IFC_BIN_APPEND_STR(&x,spt->method);
			DBG("DEBUG:ifc:ifc_bin_encode_spt: Method appended\n");
			break;
		case 3:
			IFC_BIN_APPEND_INT1(&x,spt->sip_header.type);
			IFC_BIN_APPEND_STR(&x,spt->sip_header.header);
			IFC_BIN_APPEND_STR(&x,spt->sip_header.content);
			IFC_BIN_APPEND_REGEX(&x,&spt->sip_header.content_comp);
			DBG("DEBUG:ifc:ifc_bin_encode_spt: SipHeader appended\n");
			break;
		case 4:
			IFC_BIN_APPEND_INT1(&x,spt->session_case);
			DBG("DEBUG:ifc:ifc_bin_encode_spt: SessionCase appended\n");
			break;
		case 5:
			IFC_BIN_APPEND_STR(&x,spt->session_desc.line);
			IFC_BIN_APPEND_STR(&x,spt->session_desc.content);
			IFC_BIN_APPEND_REGEX(&x,&spt->session_desc.content_comp);
			DBG("DEBUG:ifc:ifc_bin_encode_spt: SessionDesc appended\n");
			break;
	}
	return x;
}

/**
 *	Encode and append a Public Indentity
 */
static ifc_ims_bin ifc_bin_encode_public_identity(ifc_public_identity *pi)
{
	ifc_ims_bin x;
	x.s=0;x.len=0;

	IFC_BIN_APPEND_INT1(&x,pi->barring_indication);
	DBG("DEBUG:ifc:ifc_bin_encode_public_identity: barring indication appended \n");

	IFC_BIN_APPEND_STR(&x,pi->identity);
	DBG("DEBUG:ifcfc_bin:ifc_bin_encode_public_identity: identity appended\n");

	return x;
}


/**
 *	Encode and Append an entire Filter Criteria
 */
ifc_ims_bin ifc_bin_encode_filter_criteria(ifc_filter_criteria *fc)
{
	ifc_ims_bin x,*spt;
	int i,ppindicator;
	x.s=0;x.len=0;

	IFC_BIN_APPEND_INT4(&x,fc->priority);
	DBG("DEBUG:ifc:ifc_bin_encode_filter_criteria: priority appended\n");
	
	if (fc->profile_part_indicator) ppindicator = (*fc->profile_part_indicator)+1;
	else ppindicator = 0;
	IFC_BIN_APPEND_INT1(&x,ppindicator);		
	DBG("DEBUG:ifc:ifc_bin_encode_filter_criteria: profile part indicator appended\n");
	
	if (fc->trigger_point) {
		
		IFC_BIN_APPEND_INT1(&x,fc->trigger_point->condition_type_cnf);
		DBG("DEBUG:ifc:ifc_bin_encode_filter_criteria: cond_type_cnf appended\n");

		spt = pkg_malloc(sizeof(ifc_ims_bin)*fc->trigger_point->spt_cnt);
		for(i=0;i<fc->trigger_point->spt_cnt;i++){
			spt[i]=ifc_bin_encode_spt(&(fc->trigger_point->spt[i]));
            if (fc->trigger_point->spt[i].type==3||
                fc->trigger_point->spt[i].type==5)
                    ifc_bin_print(spt+i);
		}
		DBG("DEBUG:ifc:ifc_bin_encode_filter_criteria: spt array generated\n");


		ifc_bin_append_array(&x,spt,fc->trigger_point->spt_cnt);
		pkg_free(spt);
		DBG("DEBUG:ifc:ifc_bin_encode_filter_criteria: spt array appended\n");

	}
	else
		IFC_BIN_APPEND_INT4(&x,0xff)
	DBG("DEBUG:ifc:ifc_bin_encode_filter_criteria: trigger_point done\n");

	IFC_BIN_APPEND_STR(&x,fc->application_server.server_name);
	IFC_BIN_APPEND_INT1(&x,fc->application_server.default_handling);
	IFC_BIN_APPEND_STR(&x,fc->application_server.service_info);
	DBG("DEBUG:ifc:ifc_bin_encode_filter_criteria: server details appended\n");
	return x;
}

/**
 *	Encode and append a Service Profile
 */
static ifc_ims_bin ifc_bin_encode_service_profile(ifc_service_profile *sp)
{
	ifc_ims_bin x,pi[sp->public_identity_cnt],fc[sp->filter_criteria_cnt];
	int i;
	x.s=0;x.len=0;

	//public identity
	for(i=0;i<sp->public_identity_cnt;i++)
		pi[i]=ifc_bin_encode_public_identity(&(sp->public_identity[i]));
	DBG("DEBUG:ifc:ifc_bin_encode_service_profile: public_identity array generated\n");

	ifc_bin_append_array(&x,pi,sp->public_identity_cnt);
	DBG("DEBUG:ifc:ifc_bin_encode_service_profile: public_identity array appended\n");

	//filter criteria
	for(i=0;i<sp->filter_criteria_cnt;i++)
		fc[i]=ifc_bin_encode_filter_criteria(&(sp->filter_criteria[i]));
		DBG("DEBUG:ifc:ifc_bin_encode_service_profile: filter_criteria array generated\n");

	ifc_bin_append_array(&x,fc,sp->filter_criteria_cnt);
	DBG("DEBUG:ifc:ifc_bin_encode_service_profile: filter_criteria array appended\n");

	//cn service_auth
	if (sp->cn_service_auth)
		IFC_BIN_APPEND_INT4(&x,sp->cn_service_auth->subscribed_media_profile_id)
	else
		IFC_BIN_APPEND_INT4(&x,0xFFFFFFFF);
	DBG("DEBUG:ifc:ifc_bin_encode_service_profile: cn_service_auth appended\n");

	return x;
}


/**
 *	Encode the entire user profile and return the pointer to the binary data
 */
ifc_ims_bin *ifc_bin_encode(ifc_ims_subscription *s)
{
	ifc_ims_bin *x,sp[s->service_profile_cnt];
	int i;

	IFC_BIN_ALLOC(x);
	IFC_BIN_APPEND_STR(x,s->private_identity);
	DBG("DEBUG:ifc:ifc_bin_encode: private_identity appended\n");

	for(i=0;i<s->service_profile_cnt;i++)
		sp[i]=ifc_bin_encode_service_profile(&(s->service_profile[i]));
	DBG("DEBUG:ifc:ifc_bin_encode: service_profile array generated\n");

	ifc_bin_append_array(x,sp,s->service_profile_cnt);
	DBG("DEBUG:ifc:ifc_bin_encode: service_profile array appended\n");

	return x;
}




/**
 *	simple print function 
 */
void ifc_bin_print(ifc_ims_bin *imsb)
{
	int i,j,w=16;
	char x;
	printf("----------------------------------\nBinary form %d bytes:\n",imsb->len);
	for(i=0;i<imsb->len;i+=w){
		printf("%04X> ",i);
		for(j=0;j<w;j++){
			if (i+j<imsb->len) printf("%02X ",(unsigned char)imsb->s[i+j]);
			else printf("   ");
		}
		printf("\t");
		for(j=0;j<w;j++)if (i+j<imsb->len){
			if (imsb->s[i+j]>32) x=imsb->s[i+j];
			else x = '.';
			printf("%c",x);
		}else printf(" ");
		printf("\n");
	}
	printf("\n---------------------------------\n");
}






/**
 * Decoding into structures functions, without allocating new space for data
 */

/**
 *	Decode of 1 byte integer
 */
#define IFC_BIN_DECODE_INT1(X,PTR) ((unsigned char)(X)->s[(*(PTR))++])

/**
 *	Decode of 2 byte integer
 */
#define IFC_BIN_DECODE_INT2(X,PTR) \
    ((unsigned char)(X)->s[(*(PTR))] | \
	 (unsigned char)(X)->s[(*(PTR))+1] <<8),*(PTR)+=2

/**
 *	Decode of 4 byte integer
 */
#define IFC_BIN_DECODE_INT4(X,PTR) \
		 ((unsigned char)(X)->s[(*(PTR))] | \
		  (unsigned char)(X)->s[(*(PTR))+1] <<8 |\
		  (unsigned char)(X)->s[(*(PTR))+2] <<16 |\
		  (unsigned char)(X)->s[(*(PTR))+3] <<24 ),*(PTR)+=4

/**
 *	Decode of a str string
 */
#define IFC_BIN_DECODE_STR(S,X,PTR) {\
	(S).len = (unsigned char)(X)->s[(*(PTR))] |\
			(unsigned char)(X)->s[(*(PTR))+1]<<8;\
	(S).s = (X)->s + (*PTR) +2;\
	(*(PTR)) += 2+(S).len;\
}

/**
 *	Decode of a regular expression
 */
#define IFC_BIN_DECODE_REGEX(R,X,PTR) {\
	int len;\
	len = (unsigned char)(X)->s[(*(PTR))] |\
	      (unsigned char)(X)->s[(*(PTR))+1]<<8;\
	(R)=*((regex_t*) ((X)->s+(*PTR)+2));		\
	(*(PTR))+= 2+len;\
}

/* decoding advanced data structures with memory allocation */
/**
 *	Decode an array
 */
static ifc_bin_array ifc_bin_decode_array(ifc_ims_bin *imsb,int *ptr)
{
	ifc_bin_array x;
	int i;
	int last;
	x.cnt=0;x.get=NULL;

	x.cnt=(unsigned char)imsb->s[*ptr] |
		  (unsigned char)imsb->s[*ptr+1]<<8 |
		  (unsigned char)imsb->s[*ptr+2]<<16 |
		  (unsigned char)imsb->s[*ptr+3]<<24;
    DBG("DEBUG:ifc:ifc_bin_decode_array: Found array of size %d\n",x.cnt);

	x.get = (ifc_ims_bin*) pkg_malloc(sizeof(ifc_ims_bin)*x.cnt);
	last=(*ptr) + 4*(x.cnt+1);
	*ptr+=4;

	for(i=0;i<x.cnt;i++){
		x.get[i].s = imsb->s + last;
		last =(unsigned char)imsb->s[*ptr] |
		  (unsigned char)imsb->s[*ptr+1]<<8 |
		  (unsigned char)imsb->s[*ptr+2]<<16 |
		  (unsigned char)imsb->s[*ptr+3]<<24;
		x.get[i].len = last - (x.get[i].s - imsb->s);
		*ptr += 4;
		DBG("DEBUG:ifc:ifc_bin_decode_array: Found element[%2d] of size %3d\n",i,x.get[i].len);
	}
	*ptr = last;
	//*ptr = x.get[x.cnt-1].s - imsb->s+x.get[x.cnt-1].len;
	return x;
}

/**
 *	Free up memory taken by an array
 */
static inline void ifc_bin_free_array(ifc_bin_array x)
{
	pkg_free(x.get);
}


/* decoding ifc_imss data structures*/
/**
 *	Decode an entire Filter Criteria
 */
ifc_filter_criteria ifc_bin_decode_filter_criteria(ifc_ims_bin *s)
{
	ifc_filter_criteria fc;
	ifc_bin_array bin_array;
	int ptr=0,ptr2,cnf,i,ppindicator;
	
	fc.imsb = s;
	
	fc.priority = IFC_BIN_DECODE_INT4(s,&ptr);
	DBG("DEBUG:ifc:ifc_bin_decode_filter_criteria: Priority decoded %d\n",fc.priority);

	ppindicator = IFC_BIN_DECODE_INT1(s,&ptr);
	DBG("DEBUG:ifc:ifc_bin_decode_filter_criteria: Profile Part Indicator decoded %d\n",ppindicator);
	if (!ppindicator){
		fc.profile_part_indicator = 0;
	}
	else {
		fc.profile_part_indicator = (int*)shm_malloc(sizeof(int));
		*(fc.profile_part_indicator) = ppindicator-1;
	}
	
	cnf= IFC_BIN_DECODE_INT1(s,&ptr);	
	DBG("DEBUG:ifc:ifc_bin_decode_filter_criteria: Condition type CNF decoded %d\n",cnf);

	if (cnf==0xFF)
		fc.trigger_point=NULL;
	else {
		fc.trigger_point = (ifc_trigger_point*)shm_malloc(sizeof(ifc_trigger_point));
		fc.trigger_point->condition_type_cnf=cnf;

		bin_array = ifc_bin_decode_array(s,&ptr);
		fc.trigger_point->spt_cnt = bin_array.cnt;
		fc.trigger_point->spt = (ifc_spt*)shm_malloc(
				sizeof(ifc_spt)*bin_array.cnt);
		for(i=0;i<bin_array.cnt;i++){
			ptr2=0;
			fc.trigger_point->spt[i].type= IFC_BIN_DECODE_INT1(bin_array.get+i,&ptr2);
			fc.trigger_point->spt[i].condition_negated=((fc.trigger_point->spt[i].type & 0x80)!=0);
			fc.trigger_point->spt[i].registration_type=((fc.trigger_point->spt[i].type & 0x70)>>4);
			fc.trigger_point->spt[i].type=fc.trigger_point->spt[i].type & 0x0F;
		    DBG("DEBUG:ifc:ifc_bin_decode_filter_criteria: condition_negated %d & type %d decoded\n",
			    fc.trigger_point->spt[i].condition_negated,fc.trigger_point->spt[i].type);

			fc.trigger_point->spt[i].group= IFC_BIN_DECODE_INT2(bin_array.get+i,&ptr2);
			DBG("DEBUG:ifc:ifc_bin_decode_filter_criteria: group %d decoded\n",
				fc.trigger_point->spt[i].group);

			switch(fc.trigger_point->spt[i].type){
				case 1:
					IFC_BIN_DECODE_STR(fc.trigger_point->spt[i].request_uri,bin_array.get+i,&ptr2);
					DBG("DEBUG:ifc:ifc_bin_decode_filter_criteria: RequestUri decoded %.*s\n",
						fc.trigger_point->spt[i].request_uri.len,fc.trigger_point->spt[i].request_uri.s);
					break;
				case 2:
					IFC_BIN_DECODE_STR(fc.trigger_point->spt[i].method,bin_array.get+i,&ptr2);
					DBG("DEBUG:ifc:ifc_bin_decode_filter_criteria: Method decoded %.*s\n",
						fc.trigger_point->spt[i].method.len,fc.trigger_point->spt[i].method.s);
					break;
				case 3:
					fc.trigger_point->spt[i].sip_header.type = IFC_BIN_DECODE_INT1(bin_array.get+i,&ptr2);
					IFC_BIN_DECODE_STR(fc.trigger_point->spt[i].sip_header.header,bin_array.get+i,&ptr2);
					IFC_BIN_DECODE_STR(fc.trigger_point->spt[i].sip_header.content,bin_array.get+i,&ptr2);
					IFC_BIN_DECODE_REGEX(fc.trigger_point->spt[i].sip_header.content_comp,bin_array.get+i,&ptr2);
					DBG("DEBUG:ifc:ifc_bin_decode_filter_criteria: SipHeader decoded %.*s\n",
						fc.trigger_point->spt[i].sip_header.header.len,fc.trigger_point->spt[i].sip_header.header.s);
					break;
				case 4:
					fc.trigger_point->spt[i].session_case = IFC_BIN_DECODE_INT1(bin_array.get+i,&ptr2);
					DBG("DEBUG:ifc:ifc_bin_decode_filter_criteria: SessionCase decoded %d\n",
						fc.trigger_point->spt[i].session_case);
					break;
				case 5:
					IFC_BIN_DECODE_STR(fc.trigger_point->spt[i].session_desc.line,bin_array.get+i,&ptr2);
					IFC_BIN_DECODE_STR(fc.trigger_point->spt[i].session_desc.content,bin_array.get+i,&ptr2);
					IFC_BIN_DECODE_REGEX(fc.trigger_point->spt[i].session_desc.content_comp,bin_array.get+i,&ptr2);
					DBG("DEBUG:ifc:ifc_bin_decode_filter_criteria: SessionDesc decoded %.*s\n",
						fc.trigger_point->spt[i].session_desc.line.len,fc.trigger_point->spt[i].session_desc.line.s);
					break;

			}
		}
		DBG("DEBUG:ifc:ifc_bin_decode_filter_criteria: spt[%d] array decoded \n",bin_array.cnt);
		ifc_bin_free_array(bin_array);

	}
	IFC_BIN_DECODE_STR(fc.application_server.server_name,s,&ptr);
	fc.application_server.default_handling=IFC_BIN_DECODE_INT1(s,&ptr);
	IFC_BIN_DECODE_STR(fc.application_server.service_info,s,&ptr);
	DBG("DEBUG:ifc:ifc_bin_decode_filter_criteria: Server details decoded \n");

	return fc;
}

/**
 *	Decode an entire service profile
 */
static ifc_service_profile ifc_bin_decode_service_profile(ifc_ims_bin *s)
{
	ifc_service_profile sp;
	ifc_bin_array bin_array;
	int i,ptr,ptr2;

	sp.public_identity_cnt=0;
	sp.filter_criteria_cnt=0;
	sp.cn_service_auth=NULL;
	ptr = 0;

	bin_array = ifc_bin_decode_array(s,&ptr);
	sp.public_identity_cnt = bin_array.cnt;
	sp.public_identity = (ifc_public_identity*)shm_malloc(
				sizeof(ifc_public_identity)*bin_array.cnt);
	for(i=0;i<bin_array.cnt;i++){
		ptr2=0;
		sp.public_identity[i].barring_indication= IFC_BIN_DECODE_INT1(bin_array.get+i,&ptr2);
		IFC_BIN_DECODE_STR(sp.public_identity[i].identity,bin_array.get+i,&ptr2);
	}
	DBG("DEBUG:ifc:ifc_bin_decode_service_profile: public_identity[%d] array decoded\n",bin_array.cnt);
	ifc_bin_free_array(bin_array);


	bin_array = ifc_bin_decode_array(s,&ptr);

	sp.filter_criteria_cnt = bin_array.cnt;
	sp.filter_criteria = (ifc_filter_criteria*)shm_malloc(
				sizeof(ifc_filter_criteria)*bin_array.cnt);
	for(i=0;i<bin_array.cnt;i++)
		sp.filter_criteria[i]= ifc_bin_decode_filter_criteria(bin_array.get+i);
	DBG("DEBUG:ifc:ifc_bin_decode_service_profile: filter_criteria[%d] array decoded\n",bin_array.cnt);
	ifc_bin_free_array(bin_array);

	i = IFC_BIN_DECODE_INT4(s,&ptr);
	if (i==0xFFFFFFFF)
		sp.cn_service_auth=NULL;
	else {
		sp.cn_service_auth= (ifc_cn_service_auth*)shm_malloc(sizeof(ifc_cn_service_auth));
		sp.cn_service_auth->subscribed_media_profile_id=i;
	}
	DBG("DEBUG:ifc:ifc_bin_decode_service_profile: CN services auth decoded\n");

	return sp;
}


/**
 *	Decode a binary string into an IMSSubcription data structure 
 */
ifc_ims_subscription *ifc_bin_decode(ifc_ims_bin *imsb)
{
	ifc_ims_subscription *imss;
	ifc_bin_array bin_array;
	int i,ptr;
	imss = (ifc_ims_subscription*) shm_malloc(sizeof(ifc_ims_subscription));

	imss->imsb = (str*)shm_malloc(sizeof(str));
	*imss->imsb = *imsb;

	ptr=0;
	IFC_BIN_DECODE_STR(imss->private_identity,imsb,&ptr);
	DBG("DEBUG:ifc:ifc_bin_decode: private_identity %.*s\n",imss->private_identity.len,imss->private_identity.s);

	bin_array = ifc_bin_decode_array(imsb,&ptr);
	imss->service_profile_cnt = bin_array.cnt;
	imss->service_profile = (ifc_service_profile*)shm_malloc(
				sizeof(ifc_service_profile)*bin_array.cnt);
	for(i=0;i<bin_array.cnt;i++)
		imss->service_profile[i] = ifc_bin_decode_service_profile(bin_array.get+i);
	DBG("DEBUG:ifc:ifc_bin_decode: service_profile[%d] array decoded\n",bin_array.cnt);
	ifc_bin_free_array(bin_array);


	return imss;
}


/* end of ifc_bin library functions */
