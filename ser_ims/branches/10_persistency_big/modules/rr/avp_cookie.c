/*
 * Route & Record-Route module, avp cookie support
 *
 * $Id$
 *
 * Copyright (C) 2001-2003 FhG Fokus
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

#include <stdio.h>
#include "avp_cookie.h"
#include "../../crc.h"
#include "../../usr_avp.h"
#include "../../mem/mem.h"

#define AVP_COOKIE_NAME "avp="
#define AVP_COOKIE_BUFFER 1024
#define CRC_LEN 4

unsigned short crc_secret = 0;
regex_t* cookie_filter_re = 0;
avp_flags_t avp_flag_dialog = 0;

void base64decode(char* src_buf, int src_len, char* tgt_buf, int* tgt_len) {
	int pos, i, n;
	unsigned char c[4];
	for (pos=0, i=0, *tgt_len=0; pos < src_len; pos++) {
		if (src_buf[pos] >= 'A' && src_buf[pos] <= 'Z')
			c[i] = src_buf[pos] - 65;   /* <65..90>  --> <0..25> */
		else if (src_buf[pos] >= 'a' && src_buf[pos] <= 'z')
			c[i] = src_buf[pos] - 71;   /* <97..122>  --> <26..51> */
		else if (src_buf[pos] >= '0' && src_buf[pos] <= '9')
			c[i] = src_buf[pos] + 4;    /* <48..56>  --> <52..61> */
		else if (src_buf[pos] == '+')
			c[i] = 62;
		else if (src_buf[pos] == '/')
			c[i] = 63;
		else  /* '=' */
			c[i] = 64;
		i++;
		if (pos == src_len-1) {
			while (i < 4) {
				c[i] = 64;
				i++;
			}
		}
		if (i==4) {
			if (c[0] == 64)
				n = 0;
			else if (c[2] == 64)
				n = 1;
			else if (c[3] == 64)
				n = 2;
			else
				n = 3;
			switch (n) {
				case 3:
					tgt_buf[*tgt_len+2] = (char) (((c[2] & 0x03) << 6) | c[3]);
					/* no break */
				case 2:
					tgt_buf[*tgt_len+1] = (char) (((c[1] & 0x0F) << 4) | (c[2] >> 2));
					/* no break */
				case 1:
					tgt_buf[*tgt_len+0] = (char) ((c[0] << 2) | (c[1] >> 4));
					break;
			}
			i=0;
			*tgt_len+= n;
		}
	}
}

void base64encode(char* src_buf, int src_len, char* tgt_buf, int* tgt_len) {
	static char code64[64+1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int pos;
	for (pos=0, *tgt_len=0; pos < src_len; pos+=3,*tgt_len+=4) {
		tgt_buf[*tgt_len+0] = code64[(unsigned char)src_buf[pos+0] >> 2];
		tgt_buf[*tgt_len+1] = code64[(((unsigned char)src_buf[pos+0] & 0x03) << 4) | ((pos+1 < src_len)?((unsigned char)src_buf[pos+1] >> 4):0)];
		if (pos+1 < src_len)
			tgt_buf[*tgt_len+2] = code64[(((unsigned char)src_buf[pos+1] & 0x0F) << 2) | ((pos+2 < src_len)?((unsigned char)src_buf[pos+2] >> 6):0)];
		else
			(*tgt_len)--;
			/* tgt_buf[*tgt_len+2] = '='; do not add trailing equals - not header compliant without quotes */
		if (pos+2 < src_len)
			tgt_buf[*tgt_len+3] = code64[(unsigned char)src_buf[pos+2] & 0x3F];
		else
			(*tgt_len)--;
			/* tgt_buf[*tgt_len+3] = '='; */
	}
}

#define MAX_AVP_DIALOG_LISTS 4
static unsigned short avp_dialog_lists[MAX_AVP_DIALOG_LISTS] = {AVP_CLASS_URI|AVP_TRACK_FROM, AVP_CLASS_URI|AVP_TRACK_TO,
                                                                AVP_CLASS_USER|AVP_TRACK_FROM, AVP_CLASS_USER|AVP_TRACK_TO};
typedef char rr_avp_flags_t;

str *rr_get_avp_cookies(void) {
	unsigned short crc, ll;
	static char buf[AVP_COOKIE_BUFFER];
	int len, l, avp_list_no;
	struct usr_avp *avp;
	int_str avp_val;
	str *avp_name;
	str *result = 0;
	rr_avp_flags_t avp_flags;
	void** p_data;

	len = sizeof(crc);
	for (avp_list_no=0; avp_list_no<MAX_AVP_DIALOG_LISTS; avp_list_no++) {
		for ( avp=get_avp_list(avp_dialog_lists[avp_list_no]); avp; avp = avp->next ) {

			if ( (avp->flags & avp_flag_dialog) == 0)
				continue;

			if ((avp->flags&(AVP_NAME_STR|AVP_VAL_STR)) == AVP_NAME_STR) {
				/* avp type str, int value */
				p_data=&avp->data;
				avp_name = & ((struct str_int_data*)p_data)->name;
			}
			else if ((avp->flags&(AVP_NAME_STR|AVP_VAL_STR)) == (AVP_NAME_STR|AVP_VAL_STR)) {
				/* avp type str, str value */
				p_data=&avp->data;
				avp_name = & ((struct str_str_data*)p_data)->name;
			}
			else
				avp_name = 0;  /* dummy */

			get_avp_val(avp, &avp_val);

			l = sizeof(rr_avp_flags_t);
			if (avp->flags & AVP_NAME_STR )
				l += avp_name->len+sizeof(unsigned short);
			else
				l += sizeof(avp->id);
			if (avp->flags & AVP_VAL_STR )
				l += avp_val.s.len+sizeof(unsigned short);
			else
				l += sizeof(avp_val.n);
			if (len+l > AVP_COOKIE_BUFFER) {
				LOG(L_ERR, "rr:get_avp_cookies: not enough memory to prepare all cookies\n");
				goto brk;
			}
			avp_flags = (avp->flags & 0x0F)|(avp_list_no << 4);
			memcpy(buf+len, &avp_flags, sizeof(rr_avp_flags_t));
			len += sizeof(rr_avp_flags_t);
			if (avp->flags & AVP_NAME_STR) {
				if (avp_name->len > 0xFFFF)
					ll = 0xFFFF;
				else
					ll = avp_name->len;
				memcpy(buf+len, &ll, sizeof(ll));
				len+= sizeof(ll);
				memcpy(buf+len, avp_name->s, ll);
				len+= ll;
			}
			else {
				memcpy(buf+len, &avp->id, sizeof(avp->id));
				len+= sizeof(avp->id);
			}
			if (avp->flags & AVP_VAL_STR) {
				if (avp_val.s.len > 0xFFFF)
					ll = 0xFFFF;
				else
					ll = avp_val.s.len;
				memcpy(buf+len, &ll, sizeof(ll));
				len+= sizeof(ll);
				memcpy(buf+len, avp_val.s.s, ll);
				len+= ll;
			}
			else {
				memcpy(buf+len, &avp_val.n, sizeof(avp_val.n));
				len+= sizeof(avp_val.n);
			}
		}
	}
brk:
	if (len > sizeof(crc)) {
		result = (str*) pkg_malloc(sizeof(*result) + sizeof(crc) + (len*4)/3 + 3);
		if (!result) {
			LOG(L_ERR, "rr:get_avp_cookies: not enough memory\n");
			return 0;
		}
		result->s = (char*)result + sizeof(*result);
		crc = crcitt_string_ex(buf+sizeof(crc), len-sizeof(crc), crc_secret);
		memcpy(&buf, &crc, sizeof(crc));

		base64encode(buf, len, result->s, &result->len);
		DBG("avp_cookies: len=%d, crc=0x%x, base64(%u)='%.*s'\n", len, crc, result->len, result->len, result->s);
	}
	return result;
}

void rr_set_avp_cookies(str *enc_cookies, int reverse_direction) {
	char *buf;
	int len, pos;
	unsigned short crc;
	struct usr_avp avp;
	int_str avp_name, avp_val;
	regmatch_t pmatch;
	rr_avp_flags_t avp_flags;

	DBG("rr_set_avp_cookies: enc_cookie(%d)='%.*s'\n", enc_cookies->len, enc_cookies->len, enc_cookies->s);
	buf = (char*) pkg_malloc((enc_cookies->len*3)/4 + 3);
	if (!buf) {
		LOG(L_ERR, "rr:set_avp_cookies: not enough memory\n");
		return;
	}
	base64decode(enc_cookies->s, enc_cookies->len, buf, &len);

	if (len <= sizeof(crc))
		return;
	crc = crcitt_string_ex(buf+sizeof(crc), len-sizeof(crc), crc_secret);
	if (crc != *(unsigned short*) buf) {
		LOG(L_ERR, "rr:set_avp_cookies: bad CRC when decoding AVP cookie\n");
		return;
	}
	pos = sizeof(crc);
	while (pos < len) {
		memcpy(&avp_flags, buf+pos, sizeof(avp_flags));
		if ((avp_flags >> 4) >= MAX_AVP_DIALOG_LISTS) {
			LOG(L_ERR, "rr:set_avp_cookies: AVP cookies corrupted\n");
			break;
		}

		avp.flags = (avp_flags & 0x0F) | avp_dialog_lists[avp_flags >> 4];
		if (reverse_direction && (avp.flags & (AVP_CLASS_DOMAIN|AVP_CLASS_USER|AVP_CLASS_URI)) ) {
			avp.flags ^= AVP_TRACK_ALL;  /* flip from/to flags */
		}
		pos+= sizeof(rr_avp_flags_t);
		if (avp.flags & AVP_NAME_STR) {
			avp_name.s.len = 0;
			memcpy(&avp_name.s.len, buf+pos, sizeof(unsigned short));
			avp_name.s.s = buf+pos+sizeof(unsigned short);
			pos+= sizeof(unsigned short)+avp_name.s.len;
			DBG("rr:set_avp_cookies: found cookie '%.*s'\n", avp_name.s.len, avp_name.s.s);
		}
		else {
			memcpy(&avp.id, buf+pos, sizeof(avp.id));
			pos+= sizeof(avp.id);
			avp_name.n = avp.id;
			DBG("rr:set_avp_cookies: found cookie #%d\n", avp_name.n);
		}
		if (pos >= len) {
			LOG(L_ERR, "rr:set_avp_cookies: AVP cookies corrupted\n");
			break;
		}
		if (avp.flags & AVP_VAL_STR) {
			avp_val.s.len = 0;
			memcpy(&avp_val.s.len, buf+pos, sizeof(unsigned short));
			avp_val.s.s = buf+pos+sizeof(unsigned short);
			pos+= sizeof(unsigned short)+avp_val.s.len;
		}
		else {
			memcpy(&avp_val.n, buf+pos, sizeof(avp_val.n));
			pos+= sizeof(avp_val.n);
		}
		if (pos > len) {
			LOG(L_ERR, "rr:set_avp_cookies: AVP cookies corrupted\n");
			break;
		}
		/* filter cookie */
		if (cookie_filter_re) {
			if (avp.flags & AVP_NAME_STR) {
				char savec;
				savec = avp_name.s.s[avp_name.s.len];
				avp_name.s.s[avp_name.s.len] = 0;
				if (regexec(cookie_filter_re, avp_name.s.s, 1, &pmatch, 0) != 0) {
					DBG("rr:set_avp_cookies: regex doesn't match (str)\n");
					avp_name.s.s[avp_name.s.len] = savec;
					continue;
				}
				avp_name.s.s[avp_name.s.len] = savec;
			}
			else {
				char buf[25];
				snprintf(buf, sizeof(buf)-1, "i:%d", avp_name.n);
				buf[sizeof(buf)-1]=0;
				if (regexec(cookie_filter_re, buf, 1, &pmatch, 0) != 0) {
					DBG("rr:set_avp_cookies: regex doesn't match (int)\n");
					continue;
				}
			}
		}
		/* set avp from cookie */
		DBG("rr:set_avp_cookies: adding AVP\n");

		if ( add_avp(avp.flags|avp_flag_dialog, avp_name, avp_val)!=0 ) {
			LOG(L_ERR, "ERROR: rr:set_avp_cookies: add_avp failed\n");
		}
	}
	pkg_free(buf);
}
