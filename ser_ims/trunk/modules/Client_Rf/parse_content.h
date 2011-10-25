/*
 * $Id: parse_content.h 4007 2011-10-04 09:10:23Z aru $
 *
 * Copyright (C) 2001-2003 FhG Fokus
 *
 * This file is part of Kamailio, a free SIP server.
 *
 * Kamailio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * Kamailio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*!
 * \file
 * \brief Content header parser
 * \ingroup parser
 */

#ifndef _PARSE_CONTENT_H
#define _PARSE_CONTENT_H

#include "../../parser/msg_parser.h"


struct mime_type {
	unsigned short type;
	unsigned short subtype;
};



/*
 * Mimes types/subtypes that are recognized
 */
#define TYPE_TEXT            1
#define TYPE_MESSAGE         2
#define TYPE_APPLICATION     3
#define TYPE_MULTIPART       4
#define TYPE_ALL             0xfe
#define TYPE_UNKNOWN         0xff

#define SUBTYPE_PLAIN        1
#define SUBTYPE_CPIM         2
#define SUBTYPE_SDP          3
#define SUBTYPE_CPLXML       4
#define SUBTYPE_PIDFXML      5
#define SUBTYPE_RLMIXML      6
#define SUBTYPE_RELATED      7
#define SUBTYPE_LPIDFXML     8
#define SUBTYPE_XPIDFXML     9
#define SUBTYPE_WATCHERINFOXML     10
#define SUBTYPE_EXTERNAL_BODY      11
#define SUBTYPE_XML_MSRTC_PIDF     12
#define SUBTYPE_SMS          13       /* simple-message-summary */
#define SUBTYPE_MIXED        14
#define SUBTYPE_ALL          0xfe
#define SUBTYPE_UNKNOWN      0xff


/*!
 * Maximum number of mimes allowed in Accept header 
 */
#define MAX_MIMES_NR         128

/*!
 * returns the content-length value of a sip_msg as an integer
 */
#define get_content_length(_msg_)   ((long)((_msg_)->content_length->parsed))


/*!
 * returns the content-type value of a sip_msg as an integer
 */
#define get_content_type(_msg_)   ((int)(long)((_msg_)->content_type->parsed))


/*!
 * returns the accept values of a sip_msg as an null-terminated array
 * of integers
 */
#define get_accept(_msg_) ((int*)((_msg_)->accept->parsed))

/*!
 * parse the body of the Content-Type header. It's value is also converted
 * as int.
 * Returns:   n (n>0)  : the found type
 *            0        : hdr not found
 *           -1        : error (parse error )
 */
int ecscf_parse_content_type_hdr( struct sip_msg *msg);

/*!
 * parse the body of the Accept header. It's values are also converted
 * as an null-terminated array of ints.
 * \return 1 for OK, 0 for hdr not found, -1 on errors (parse errors)
 */
int parse_accept_hdr( struct sip_msg *msg );


/*!
 *  parse the body of a Content_-Length header. Also tries to recognize the
 *  type specified by this header (see th above defines).
 *  \return first chr after the end of the header.
 */
char* ecscf_parse_content_length( char* buffer, char* end, int* len);


/*!
 * parse a string containing a mime description
 */
char* ecscf_decode_mime_type(char *start, char *end, unsigned int *mime_type);

#endif
