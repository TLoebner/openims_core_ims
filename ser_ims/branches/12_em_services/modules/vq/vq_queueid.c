/*
 * $Id$
 *
 * Virtual Queue Call Identifier generation function
 *
 * Copyright (C) 2009-2010 FhG FOKUS
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
 * author Jordi Pallares
 */

#include <stdio.h>
#include <stdlib.h>
#include "vq_queueid.h"
#include "../../parser/parse_fline.h"
#include "../../mem/shm_mem.h"
#include "../../dprint.h"
#include "../../sr_module.h"
#include "sip.h"


/*! Print a binary hexadecimal buffer in readable format.
 * 
 * Function that prints 'len' characters from the hexadecimal binary string 'src' 
 * to stdout. If specified, print also the readable hexadecimal string into the 'dst' buffer.
 */
static void 
print_hex (unsigned const char *src, unsigned char *dst, int len)
{
  int i, lo;
	
  for (i=0, lo=0 ;i < len;i++) {
    printf("%02x", src[i]);
    if (dst != NULL) lo += sprintf((void *)&dst[lo], "%02x", src[i]);
  }
	
  if (dst == NULL) printf("\n");

  return;

}

/**
 * Procedure to calculate the virtual queue ID from an incoming call.
 * This procedure also sets the 'arrival time' of the call.
 * The formula is id = md5sum { "REQ/RES:Method:from:to" }
 *
 * @param struct sip_msg *msg incoming SIP message
 * @return pointer to the new allocated call information
 */
queueID_t *
vq_get_call_id (struct sip_msg *msg)
{
  queueID_t *newcall_ID;
  char type[5];
  char string[256];
  char method[12];
  str from = {NULL, 0};
  str to = {NULL, 0};
  struct msg_start *fl;
  //struct hdr_field *hf;
  int isReq;
      
  MD5_CTX Md5Ctx;
  char HA1[HASHLEN];
  
  DBG ("Generating a queue ID...\n");
  
  memset (type, 0, sizeof(type));
  memset (string, 0, sizeof(string));
  memset (method, 0, sizeof(method));
  
  fl = &msg->first_line;
  
  isReq = fl->type;
  if (isReq != SIP_REQUEST) {
    DBG ("Not a SIP request");
    return NULL;
  }
  
  newcall_ID = shm_malloc (sizeof(queueID_t));
  if (!newcall_ID) {
    ERR ("Could not allocate memory !\n");
    return NULL;
  }

  // init time in the call_ID
  gettimeofday (&newcall_ID->time, NULL);
  memset (HA1, 0, HASHLEN);
  
  //DBG ("set call time\n");
  
  // calculate the hash
  // Use md5sum { "REQ/RES:Method:from:to" }
  MD5Init(&Md5Ctx);
  
  // add type REQ/RES
  snprintf (type, sizeof(type), "%d", fl->type);

  // add method
  memcpy (method, msg->first_line.u.request.method.s, msg->first_line.u.request.method.len);

  // add from
  cscf_get_from_uri (msg, &from);
  //DBG ("From 'uri': %.*s\n", from.len, from.s);
   
  // add to
  cscf_get_to_uri (msg, &to);
  //DBG ("To 'uri': %.*s\n", to.len, to.s);
  
  // put it together
  snprintf (string, sizeof(string), "%s:%s:%.*s:%.*s", type, method, from.len, from.s, to.len, to.s);
  LOG (L_INFO, "Calculated ID for \"%s\"\n", string);
    
  MD5Update(&Md5Ctx, string, strlen(string));
  
  MD5Final(HA1, &Md5Ctx);

  memcpy (newcall_ID->id, HA1, HASHLEN);
  print_hex ((void *)newcall_ID->id, (void *)newcall_ID->strid, HASHLEN);
  
  LOG (L_INFO, " is %s\n", newcall_ID->strid);

  return newcall_ID;

}

void 
vq_free_call_id (queueID_t *call)
{
  shm_free (call);
}


