/*
 * $Id: sdp_helpr_funcs.h 4807 2008-09-02 15:00:48Z osas $
 *
 * SDP parser helpers
 *
 * Copyright (C) 2008 SOMA Networks, INC.
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
 *
 *
 * History:
 * --------
 * 2007-09-09 ported helper functions from nathelper module (osas)
 * 2008-04-22 integrated RFC4975 attributes - patch provided by Denis Bilenko (denik)
 *
 */


#ifndef _SDP_HLPR_FUNCS_H
#define  _SDP_HLPR_FUNCS_H

#include "../../str.h"
#include "../../parser/msg_parser.h"

#ifndef SER_MOD_INTERFACE
	/*! str initialization */
	#define str_init(_string)  {_string, sizeof(_string) - 1}
#endif


/**
 * Detect the mixed part delimiter.
 *
 * Example: "boundary1"
 * Content-Type: multipart/mixed; boundary="boundary1"
 */
int get_mixed_part_delimiter(str * body, str * mp_delimiter);

int extract_rtpmap(str *body, str *rtpmap_payload, str *rtpmap_encoding, str *rtpmap_clockrate, str *rtpmap_parmas);
int extract_ptime(str *body, str *ptime);
int extract_sendrecv_mode(str *body, str *sendrecv_mode);
int extract_mediaip(str *body, str *mediaip, int *pf, char *line);
int extract_media_attr(str *body, str *mediamedia, str *mediaport, str *mediatransport, str *mediapayload);
int extract_bwidth(str *body, str *bwtype, str *bwwitdth);

/* RFC4975 attributes */
int extract_accept_types(str *body, str *accept_types);
int extract_accept_wrapped_types(str *body, str *accept_wrapped_types);
int extract_max_size(str *body, str *max_size);
int extract_path(str *body, str *path);

char *find_sdp_line(char *p, char *plimit, char linechar);
char *find_next_sdp_line(char *p, char *plimit, char linechar, char *defptr);

char* get_sdp_hdr_field(char* , char* , struct hdr_field* );

char *find_sdp_line_delimiter(char *p, char *plimit, str delimiter);
char *find_next_sdp_line_delimiter(char *p, char *plimit, str delimiter, char *defptr); 
#endif
