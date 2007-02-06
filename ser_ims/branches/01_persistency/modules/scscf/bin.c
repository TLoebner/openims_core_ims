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
 * Binary codec operations
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */


#include <stdio.h>
#include "bin.h"

/** 
 * Whether to print debug message while encoding/decoding 
 */
#define BIN_DEBUG 1

/** 
 * Whether to do sanity checks on the available data when decoding
 * If you are crazy about start-up performance you can disable this.
 * However, this is very useful for detecting broken snapshots
 */
#define BIN_DECODE_CHECKS 1

inline int bin_alloc(bin_data *x, int max_len)
{                                
	x->s = (char*)BIN_ALLOC_METHOD(max_len);     
	if (!x->s){
		LOG(L_ERR,"ERR:"M_NAME":bin_alloc: Error allocating %d bytes.\n",max_len);
		x->len=0;
		x->max=0;
		return 0;
	}
    x->len=0;
    x->max=max_len;
	return 1;
}

inline int bin_realloc(bin_data *x, int delta)
{
#if BIN_DEBUG
	LOG(L_INFO,"INFO:"M_NAME":bin_realloc: realloc %p from %d to + %d\n",x->s,x->max,delta);
#endif	
	x->s=BIN_REALLOC_METHOD(x->s,x->max + delta);    
	if (x->s==NULL){                             
		LOG(L_ERR,"ERR:"M_NAME":bin_realloc: No more memory to expand %d with %d  \n",x->max,delta);
		return 0;
	}
	x->max += delta;
	return 1;
}

inline int bin_expand(bin_data *x, int delta)
{
	if (x->max-x->len>=delta) return 1;
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_realloc: realloc %p from %d to + %d\n",x->s,x->max,delta);
#endif	
	x->s=BIN_REALLOC_METHOD(x->s,x->max + delta);    
	if (x->s==NULL){                             
		LOG(L_ERR,"ERR:"M_NAME":bin_realloc: No more memory to expand %d with %d  \n",x->max,delta);
		return 0;
	}
	x->max += delta;
	return 1;
}

inline void bin_free(bin_data *x)
{
	BIN_FREE_METHOD(x->s);
	x->s=0;x->len=0;x->max=0;
}

/**
 *	simple print function 
 */
inline void bin_print(bin_data *x)
{
	int i,j,w=16;
	char c;
	fprintf(stderr,"----------------------------------\nBinary form %d (max %d) bytes:\n",x->len,x->max);
	for(i=0;i<x->len;i+=w){
		fprintf(stderr,"%04X> ",i);
		for(j=0;j<w;j++){
			if (i+j<x->len) fprintf(stderr,"%02X ",(unsigned char)x->s[i+j]);
			else fprintf(stderr,"   ");
		}
		printf("\t");
		for(j=0;j<w;j++)if (i+j<x->len){
			if (x->s[i+j]>32) c=x->s[i+j];
			else c = '.';
			fprintf(stderr,"%c",c);
		}else fprintf(stderr," ");
		fprintf(stderr,"\n");
	}
	fprintf(stderr,"\n---------------------------------\n");
}


/* basic data type reprezentation functions */

/**
 *	Append an integer of 1 byte 
 */
inline int bin_encode_int1(bin_data *x,int k) 
{ 
	if (!bin_expand(x,1)) return 0;
	if (k>0xff) 
		LOG(L_ERR,"ERROR:"M_NAME":bin_encode_int1: Possible loss in encoding (int > 0xff bytes) %d bytes \n",k);
	x->s[x->len++]=k & 0x000000FF; 
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_encode_int1: [%d]:[%.02x] new len %04x\n",k,x->s[x->len-1],x->len);
#endif
	return 1;   
}

/**
 *	Append an integer of 2 bytes 
 */
inline int bin_encode_int2(bin_data *x,int k) 
{ 
	if (!bin_expand(x,2)) return 0;
	if (k>0xffff) 
		LOG(L_ERR,"ERROR:"M_NAME":bin_encode_int2: Possible loss in encoding (int > 0xffff bytes) %d bytes \n",k);
	x->s[x->len++]=k & 0x000000FF;    
	x->s[x->len++]=(k & 0x0000FF00) >> 8;   
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_encode_int2: [%d]:[%.02x %.02x] new len %04x\n",k,x->s[x->len-2],x->s[x->len-1],x->len);
#endif
	return 1;   
}

/**
 *	Append an integer of 4 bytes
 */
inline int bin_encode_int4(bin_data *x,int k) 
{ 
	if (!bin_expand(x,4)) return 0;
	if (k>0xffffffff) 
		LOG(L_ERR,"ERROR:"M_NAME":bin_encode_int4: Possible loss in encoding (int > 0xffffffff bytes) %d bytes \n",k);
	x->s[x->len++]= k & 0x000000FF;          
	x->s[x->len++]=(k & 0x0000FF00) >> 8;    
	x->s[x->len++]=(k & 0x00FF0000) >>16;    
	x->s[x->len++]=(k & 0xFF000000) >>24;
#if BIN_DEBUG		    
	LOG(L_INFO,"INFO:"M_NAME":bin_encode_int4: [%d]:[%.02x %.02x %.02x %.02x] new len %04x\n",k,
		x->s[x->len-4],x->s[x->len-3],x->s[x->len-2],x->s[x->len-1],x->len);
#endif		
	return 1;   
}

/**
 *	Append a string 
 */
inline int bin_encode_str(bin_data *x,str *s) 
{ 
	if (!bin_expand(x,2+s->len)) return 0;
	if (s->len>65535) 
		LOG(L_ERR,"ERROR:"M_NAME":bin_encode_str: Possible loss of characters in encoding (string > 65535bytes) %d bytes \n",s->len);
	x->s[x->len++]=s->len & 0x000000FF;
	x->s[x->len++]=(s->len & 0x0000FF00)>>8;
	memcpy(x->s+x->len,s->s,s->len);
	x->len+=s->len;
#if BIN_DEBUG		
	LOG(L_INFO,"INFO:"M_NAME":bin_encode_str : [%d]:[%.02x %.02x]:[%.*s] new len %04x\n",s->len,
		x->s[x->len-s->len-2],x->s[x->len-s->len-1],s->len,s->s,x->len);
#endif		
	return 1;   
}


/**
 *	Append a regular expression
 */
#define bin_encode_REGEX(X,R) {\
	int len;    \
	len = sizeof(*(R));\
	BIN_REALLOC(x,2+len);\
	if (len>65535)\
		LOG(L_ERR,"ERROR:"M_NAME":bin_encode_REGEX: Possible loss of characters in encoding (regex > 65535bytes) %d bytes \n",len);\
	x->s[x->len++]=len & 0x000000FF;\
	x->s[x->len++]=(len & 0x0000FF00)>>8;\
	memcpy(x->s+x->len,(R),len);\
	x->len+=len;\
}


/* advanced data type reprezentation functions */



/**
 * Decoding into structures functions, without allocating new space for data
 */

/**
 *	Decode of 1 byte integer
 */
inline int bin_decode_int1(bin_data *x,int *v)
{
#if BIN_DECODE_CHECKS
	if (x->max+1 > x->len) return 0;
#endif	
	*v = (unsigned char)x->s[x->max];
	x->max += 1;
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_decode_int1: [%d] new pos %04x\n",*v,x->max);
#endif
	return 1;
}

/**
 *	Decode of 2 byte integer
 */
inline int bin_decode_int2(bin_data *x,int *v)
{
#if BIN_DECODE_CHECKS
	if (x->max+2 > x->len) return 0;
#endif
	*v =	(unsigned char)x->s[x->max  ]    |
	 		(unsigned char)x->s[x->max+1]<<8;
	x->max += 2;
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_decode_int2: [%d] new pos %04x\n",*v,x->max);
#endif
	return 1;
}

/**
 *	Decode of 4 byte integer
 */
inline int bin_decode_int4(bin_data *x,int *v)
{
#if BIN_DECODE_CHECKS
	if (x->max+4 > x->len) return 0;
#endif
	*v =    (unsigned char)x->s[x->max  ] 		|
			(unsigned char)x->s[x->max+1] <<8 	|
		  	(unsigned char)x->s[x->max+2] <<16 	|
		  	(unsigned char)x->s[x->max+3] <<24 	;
	x->max += 4;
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_decode_int4: [%d] new pos %04x\n",*v,x->max);
#endif
	return 1;
}

/**
 *	Decode of a str string
 */
inline int bin_decode_str(bin_data *x,str *s)
{
#if BIN_DECODE_CHECKS
	if (x->max+2 > x->len) return 0;
#endif
	s->len = (unsigned char)x->s[x->max  ]    |
	 		(unsigned char)x->s[x->max+1]<<8;
	x->max +=2;
	if (x->max+s->len>=x->len) return 0;
	s->s = x->s + x->max;
	x->max += s->len;
#if BIN_DEBUG	
	LOG(L_INFO,"INFO:"M_NAME":bin_decode_str : [%d]:[%.*s] new pos %04x\n",s->len,s->len,s->s,x->max);
#endif
	return 1;
}

/**
 *	Decode of a regular expression
 */
#define BIN_DECODE_REGEX(R,X,PTR) {\
	int len;\
	len = (unsigned char)x->s[(*(PTR))] |\
	      (unsigned char)x->s[(*(PTR))+1]<<8;\
	(R)=*((regex_t*) (x->s+(*PTR)+2));		\
	(*(PTR))+= 2+len;\
}

/* end of bin library functions */
