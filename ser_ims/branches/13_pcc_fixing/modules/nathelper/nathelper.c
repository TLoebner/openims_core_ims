/* $Id$
 *
 * Copyright (C) 2003 Porta Software Ltd
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
 * History:
 * ---------
 * 2003-10-09	nat_uac_test introduced (jiri)
 *
 * 2003-11-06   nat_uac_test permitted from onreply_route (jiri)
 *
 * 2003-12-01   unforce_rtp_proxy introduced (sobomax)
 *
 * 2004-01-07	RTP proxy support updated to support new version of the
 *		RTP proxy (20040107).
 *
 *		force_rtp_proxy() now inserts a special flag
 *		into the SDP body to indicate that this session already
 *		proxied and ignores sessions with such flag.
 *
 *		Added run-time check for version of command protocol
 *		supported by the RTP proxy.
 *
 * 2004-01-16   Integrated slightly modified patch from Tristan Colgate,
 *		force_rtp_proxy function with IP as a parameter (janakj)
 *
 * 2004-01-28	nat_uac_test extended to allow testing SDP body (sobomax)
 *
 *		nat_uac_test extended to allow testing top Via (sobomax)
 *
 * 2004-02-21	force_rtp_proxy now accepts option argument, which
 *		consists of string of chars, each of them turns "on"
 *		some feature, currently supported ones are:
 *
 *		 `a' - flags that UA from which message is received
 *		       doesn't support symmetric RTP;
 *		 `l' - force "lookup", that is, only rewrite SDP when
 *		       corresponding session is already exists in the
 *		       RTP proxy. Only makes sense for SIP requests,
 *		       replies are always processed in "lookup" mode;
 *		 `i' - flags that message is received from UA in the
 *		       LAN. Only makes sense when RTP proxy is running
 *		       in the bridge mode.
 *
 *		force_rtp_proxy can now be invoked without any arguments,
 *		as previously, with one argument - in this case argument
 *		is treated as option string and with two arguments, in
 *		which case 1st argument is option string and the 2nd
 *		one is IP address which have to be inserted into
 *		SDP (IP address on which RTP proxy listens).
 *
 * 2004-03-12	Added support for IPv6 addresses in SDPs. Particularly,
 *		force_rtp_proxy now can work with IPv6-aware RTP proxy,
 *		replacing IPv4 address in SDP with IPv6 one and vice versa.
 *		This allows creating full-fledged IPv4<->IPv6 gateway.
 *		See 4to6.cfg file for example.
 *
 *		Two new options added into force_rtp_proxy:
 *
 *		 `f' - instructs nathelper to ignore marks inserted
 *		       by another nathelper in transit to indicate
 *		       that the session is already goes through another
 *		       proxy. Allows creating chain of proxies.
 *		 `r' - flags that IP address in SDP should be trusted.
 *		       Without this flag, nathelper ignores address in the
 *		       SDP and uses source address of the SIP message
 *		       as media address which is passed to the RTP proxy.
 *
 *		Protocol between nathelper and RTP proxy in bridge
 *		mode has been slightly changed. Now RTP proxy expects SER
 *		to provide 2 flags when creating or updating session
 *		to indicate direction of this session. Each of those
 *		flags can be either `e' or `i'. For example `ei' means
 *		that we received INVITE from UA on the "external" network
 *		network and will send it to the UA on "internal" one.
 *		Also possible `ie' (internal->external), `ii'
 *		(internal->internal) and `ee' (external->external). See
 *		example file alg.cfg for details.
 *
 * 2004-03-15	If the rtp proxy test failed (wrong version or not started)
 *		retry test from time to time, when some *rtpproxy* function
 *		is invoked. Minimum interval between retries can be
 *		configured via rtpproxy_disable_tout module parameter (default
 *		is 60 seconds). Setting it to -1 will disable periodic
 *		rechecks completely, setting it to 0 will force checks
 *		for each *rtpproxy* function call. (andrei)
 *
 * 2004-03-22	Fix assignment of rtpproxy_retr and rtpproxy_tout module
 *		parameters.
 *
 * 2004-03-22	Fix get_body position (should be called before get_callid)
 * 				(andrei)
 *
 * 2004-03-24	Fix newport for null ip address case (e.g onhold re-INVITE)
 * 				(andrei)
 *
 * 2004-09-30	added received port != via port test (andrei)
 *
 * 2004-10-10   force_socket option introduced (jiri)
 *
 * 2005-02-24	Added support for using more than one rtp proxy, in which
 *		case traffic will be distributed evenly among them. In addition,
 *		each such proxy can be assigned a weight, which will specify
 *		which share of the traffic should be placed to this particular
 *		proxy.
 *
 *		Introduce failover mechanism, so that if SER detects that one
 *		of many proxies is no longer available it temporarily decreases
 *		its weight to 0, so that no traffic will be assigned to it.
 *		Such "disabled" proxies are periodically checked to see if they
 *		are back to normal in which case respective weight is restored
 *		resulting in traffic being sent to that proxy again.
 *
 *		Those features can be enabled by specifying more than one "URI"
 *		in the rtpproxy_sock parameter, optionally followed by the weight,
 *		which if absent is assumed to be 1, for example:
 *
 *		rtpproxy_sock="unix:/foo/bar=4 udp:1.2.3.4:3456=3 udp:5.6.7.8:5432=1"
 *
 * 2005-02-25	Force for pinging the socket returned by USRLOC (bogdan)
 *
 * 2005-03-22	Support for multiple media streams added (netch)
 *
 * 2005-04-27	Support for doing natpinging using real SIP requests added.
 *		Requires tm module for doing its work. Old method (sending UDP
 *		with 4 zero bytes can be selected by specifying natping_method="null".
 *
 * 2005-12-23	Support for selecting particular RTP proxy node has been added.
 *		In force_rtp_proxy() it can be done via new N modifier, followed
 *		by the index (starting at 0) of the node in the rtpproxy_sock
 *		parameter. For example, in the example above force_rtp_proxy("N1") will
 *		will select node udp:1.2.3.4:3456. In unforce_rtp_proxy(), the same
 *		can be done by specifying index as an argument directly, i.e.
 *		unforce_rtp_proxy(1).
 *
 *		Since nathelper is not transaction or call stateful, care should be
 *		taken to ensure that force_rtp_proxy() in request path matches
 *		force_rtp_proxy() in reply path, that is the same node is selected.
 *
 * 2006-06-10	select nathepler.rewrite_contact
 *		pingcontact function (tma)
 */

#include "nhelpr_funcs.h"
#include "nathelper.h"
#include "sip_body.h"
#include "../../flags.h"
#include "../../sr_module.h"
#include "../../dprint.h"
#include "../../data_lump.h"
#include "../../data_lump_rpl.h"
#include "../../error.h"
#include "../../forward.h"
#include "../../mem/mem.h"
#include "../../parser/parse_from.h"
#include "../../parser/parse_to.h"
#include "../../parser/parse_uri.h"
#include "../../parser/parser_f.h"
#include "../../resolve.h"
#include "../../timer.h"
#include "../../trim.h"
#include "../../ut.h"
#include "../registrar/sip_msg.h"
#include "../../msg_translator.h"
#include "../usrloc/usrloc.h"
#include "../../usr_avp.h"
#include "../../socket_info.h"
#include "../../select.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

MODULE_VERSION

#if !defined(AF_LOCAL)
#define	AF_LOCAL AF_UNIX
#endif
#if !defined(PF_LOCAL)
#define	PF_LOCAL PF_UNIX
#endif

/* NAT UAC test constants */
#define	NAT_UAC_TEST_C_1918	0x01
#define	NAT_UAC_TEST_RCVD	0x02
#define	NAT_UAC_TEST_V_1918	0x04
#define	NAT_UAC_TEST_S_1918	0x08
#define	NAT_UAC_TEST_RPORT	0x10

/* Supported version of the RTP proxy command protocol */
#define	SUP_CPROTOVER	20040107
/* Required additional version of the RTP proxy command protocol */
#define	REQ_CPROTOVER	"20050322"
#define	CPORT		"22222"

struct rtpp_head;
struct rtpp_node;

static int nat_uac_test_f(struct sip_msg* msg, char* str1, char* str2);
static int fix_nated_contact_f(struct sip_msg *, char *, char *);
static int fix_nated_sdp_f(struct sip_msg *, char *, char *);
static int extract_mediaip(str *, str *, int *);
static int extract_mediaport(str *, str *);
static int alter_mediaip(struct sip_msg *, str *, str *, int, str *, int, int);
static int alter_mediaport(struct sip_msg *, str *, str *, str *, int);
static char *gencookie();
static int rtpp_test(struct rtpp_node*, int, int);
static char *send_rtpp_command(struct rtpp_node*, struct iovec *, int);
static int unforce_rtp_proxy0_f(struct sip_msg *, char *, char *);
static int unforce_rtp_proxy1_f(struct sip_msg *, char *, char *);
static int force_rtp_proxy1_f(struct sip_msg *, char *, char *);
static int force_rtp_proxy2_f(struct sip_msg *, char *, char *);
static int fix_nated_register_f(struct sip_msg *, char *, char *);
static char *find_sdp_line(char *, char *, char);
static char *find_next_sdp_line(char *, char *, char, char *);
static int fixup_ping_contact(void **param, int param_no);
static int ping_contact_f(struct sip_msg* msg, char* str1, char* str2);

static int mod_init(void);
static int child_init(int);

struct socket_info* force_socket = 0;


static struct {
	const char *cnetaddr;
	uint32_t netaddr;
	uint32_t mask;
} nets_1918[] = {
	{"10.0.0.0",    0, 0xffffffffu << 24},
	{"172.16.0.0",  0, 0xffffffffu << 20},
	{"192.168.0.0", 0, 0xffffffffu << 16},
	{NULL, 0, 0}
};

static str sup_ptypes[] = {
	{.s = "udp", .len = 3},
	{.s = "udptl", .len = 5},
	{.s = "rtp/avp", .len = 7},
	{.s = "rtp/savpf", .len = 9},
	{.s = NULL, .len = 0}
};

static char *rtpproxy_sock = "unix:/var/run/rtpproxy.sock"; /* list */
static char *force_socket_str = 0;
static int rtpproxy_disable = 0;
static int rtpproxy_disable_tout = 60;
static int rtpproxy_retr = 5;
static int rtpproxy_tout = 1;
static pid_t mypid;
static unsigned int myseqn = 0;


struct rtpp_head {
	struct rtpp_node	*rn_first;
	struct rtpp_node	*rn_last;
};

struct rtpp_node {
	char			*rn_url;	/* unparsed, deletable */
	int			rn_umode;
	char			*rn_address;	/* substring of rn_url */
	int			rn_fd;		/* control fd */
	int			rn_disabled;	/* found unaccessible? */
	unsigned		rn_weight;	/* for load balancing */
	int			rn_recheck_ticks;
	struct rtpp_node	*rn_next;
};

/* RTP proxy balancing list */
static struct rtpp_head rtpp_list;
static int rtpp_node_count = 0;

static cmd_export_t cmds[] = {
	{"fix_nated_contact",  fix_nated_contact_f,    0, 0,             REQUEST_ROUTE | ONREPLY_ROUTE },
	{"fix_nated_sdp",      fix_nated_sdp_f,        1, fixup_int_1, REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE },
	{"unforce_rtp_proxy",  unforce_rtp_proxy0_f,   0, 0,             REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE },
	{"unforce_rtp_proxy",  unforce_rtp_proxy1_f,   1, fixup_var_str_1,   REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE },
	{"force_rtp_proxy",    force_rtp_proxy1_f,     0, 0,             REQUEST_ROUTE | ONREPLY_ROUTE },
	{"force_rtp_proxy",    force_rtp_proxy1_f,     1, fixup_var_str_1,             REQUEST_ROUTE | ONREPLY_ROUTE },
	{"force_rtp_proxy",    force_rtp_proxy2_f,     2, fixup_var_str_12,            REQUEST_ROUTE | ONREPLY_ROUTE },
	{"nat_uac_test",       nat_uac_test_f,         1, fixup_int_1, REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE },
	{"fix_nated_register", fix_nated_register_f,   0, 0,             REQUEST_ROUTE },
	{"ping_contact",       ping_contact_f,         1, fixup_ping_contact,  REQUEST_ROUTE },
	{0, 0, 0, 0, 0}
};

static param_export_t params[] = {
	{"natping_interval",      PARAM_INT,    &natping_interval      },
	{"natping_method",        PARAM_STRING, &natping_method        },
	{"ping_nated_only",       PARAM_INT,    &ping_nated_only       },
	{"rtpproxy_sock",         PARAM_STRING, &rtpproxy_sock         },
	{"rtpproxy_disable",      PARAM_INT,    &rtpproxy_disable      },
	{"rtpproxy_disable_tout", PARAM_INT,    &rtpproxy_disable_tout },
	{"rtpproxy_retr",         PARAM_INT,    &rtpproxy_retr         },
	{"rtpproxy_tout",         PARAM_INT,    &rtpproxy_tout         },
	{"force_socket",          PARAM_STRING, &force_socket_str      },
	{0, 0, 0}
};

struct module_exports exports = {
	"nathelper",
	cmds,
	0,       /* RPC methods */
	params,
	mod_init,
	0, /* reply processing */
	0, /* destroy function */
	0, /* on_break */
	child_init
};

static int sel_nathelper(str* res, select_t* s, struct sip_msg* msg) {  /* dummy */
	return 0;
}

static int sel_rewrite_contact(str* res, select_t* s, struct sip_msg* msg);

SELECT_F(select_any_nameaddr)

select_row_t sel_declaration[] = {
	{ NULL, SEL_PARAM_STR, STR_STATIC_INIT("nathelper"), sel_nathelper, SEL_PARAM_EXPECTED},
	{ sel_nathelper, SEL_PARAM_STR, STR_STATIC_INIT("rewrite_contact"), sel_rewrite_contact, CONSUME_NEXT_INT },

	{ sel_rewrite_contact, SEL_PARAM_STR, STR_STATIC_INIT("nameaddr"), select_any_nameaddr, NESTED | CONSUME_NEXT_STR},

	{ NULL, SEL_PARAM_INT, STR_NULL, NULL, 0}
};

static int
mod_init(void)
{
	int i;
	struct in_addr addr;
	str socket_str;

	if (force_socket_str) {
		socket_str.s=force_socket_str;
		socket_str.len=strlen(socket_str.s);
		force_socket=grep_sock_info(&socket_str,0,0);
	}

	if (natpinger_init() < 0) {
		LOG(L_ERR, "nathelper: natpinger_init() failed\n");
		return -1;
	}

	/* Prepare 1918 networks list */
	for (i = 0; nets_1918[i].cnetaddr != NULL; i++) {
		if (inet_aton(nets_1918[i].cnetaddr, &addr) != 1)
			abort();
		nets_1918[i].netaddr = ntohl(addr.s_addr) & nets_1918[i].mask;
	}

	memset(&rtpp_list, 0, sizeof(rtpp_list));
	rtpp_node_count = 0;
	if (rtpproxy_disable == 0) {
		/* Make rtp proxies list. */
		char *p, *p1, *p2, *plim;

		p = rtpproxy_sock;
		plim = p + strlen(p);
		for(;;) {
			struct rtpp_node *pnode;
			int weight;

			weight = 1;
			while (*p && isspace((unsigned char)*p))
				++p;
			if (p >= plim)
				break;
			p1 = p;
			while (*p && !isspace((unsigned char)*p))
				++p;
			if (p <= p1)
				break; /* may happen??? */
			/* Have weight specified? If yes, scan it */
			p2 = memchr(p1, '=', p - p1);
			if (p2 != NULL) {
				weight = strtoul(p2 + 1, NULL, 10);
			} else {
				p2 = p;
			}
			pnode = pkg_malloc(sizeof(struct rtpp_node));
			if (pnode == NULL) {
				LOG(L_ERR, "nathelper: Can't allocate memory\n");
				return -1;
			}
			memset(pnode, 0, sizeof(*pnode));
			pnode->rn_recheck_ticks = 0;
			pnode->rn_weight = weight;
			pnode->rn_umode = 0;
			pnode->rn_fd = -1;
			pnode->rn_disabled = 0;
			pnode->rn_url = pkg_malloc(p2 - p1 + 1);
			if (pnode->rn_url == NULL) {
				LOG(L_ERR, "nathelper: Can't allocate memory\n");
				return -1;
			}
			memmove(pnode->rn_url, p1, p2 - p1);
			pnode->rn_url[p2 - p1] = 0;
			if (rtpp_list.rn_first == NULL) {
				rtpp_list.rn_first = pnode;
			} else {
				rtpp_list.rn_last->rn_next = pnode;
			}
			rtpp_list.rn_last = pnode;
			++rtpp_node_count;
			/* Leave only address in rn_address */
			pnode->rn_address = pnode->rn_url;
			if (strncmp(pnode->rn_address, "udp:", 4) == 0) {
				pnode->rn_umode = 1;
				pnode->rn_address += 4;
			} else if (strncmp(pnode->rn_address, "udp6:", 5) == 0) {
				pnode->rn_umode = 6;
				pnode->rn_address += 5;
			} else if (strncmp(pnode->rn_address, "unix:", 5) == 0) {
				pnode->rn_umode = 0;
				pnode->rn_address += 5;
			}
			 LOG(L_ERR, "DEB: IP address of RTPPROXY is %s", pnode->rn_address);
		}
	}
	register_select_table(sel_declaration);
	return 0;
}

static int
child_init(int rank)
{
	int n;
	char *cp;
	struct addrinfo hints, *res;
	struct rtpp_node *pnode;
	

	/* Iterate known RTP proxies - create sockets */
	mypid = getpid();
	for (pnode = rtpp_list.rn_first; pnode != NULL; pnode = pnode->rn_next) {
		char *old_colon;

		if (pnode->rn_umode == 0)
			goto rptest;
		/*
		 * This is UDP or UDP6. Detect host and port; lookup host;
		 * do connect() in order to specify peer address
		 */
		old_colon = cp = strrchr(pnode->rn_address, ':');
		if (cp != NULL) {
			old_colon = cp;
			*cp = '\0';
			cp++;
		}
		if (cp == NULL || *cp == '\0')
			cp = CPORT;

		memset(&hints, 0, sizeof(hints));
		hints.ai_flags = 0;
		hints.ai_family = (pnode->rn_umode == 6) ? AF_INET6 : AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		if ((n = getaddrinfo(pnode->rn_address, cp, &hints, &res)) != 0) {
			LOG(L_ERR, "nathelper: getaddrinfo: %s\n", gai_strerror(n));
			return -1;
		}
		if (old_colon)
			*old_colon = ':'; /* restore rn_address */

		pnode->rn_fd = socket((pnode->rn_umode == 6)
		    ? AF_INET6 : AF_INET, SOCK_DGRAM, 0);
		if (pnode->rn_fd == -1) {
			LOG(L_ERR, "nathelper: can't create socket\n");
			freeaddrinfo(res);
			return -1;
		}

		if (connect(pnode->rn_fd, res->ai_addr, res->ai_addrlen) == -1) {
			LOG(L_ERR, "nathelper: can't connect to a RTP proxy\n");
			close(pnode->rn_fd);
			pnode->rn_fd = -1;
			freeaddrinfo(res);
			return -1;
		}
		freeaddrinfo(res);
rptest:
		pnode->rn_disabled = rtpp_test(pnode, 0, 1);
	}

	if (rtpproxy_disable)
		rtpproxy_disable_tout = -1;

	return 0;
}

static int
isnulladdr(str *sx, int pf)
{
	char *cp;

	if (pf == AF_INET6) {
		for(cp = sx->s; cp < sx->s + sx->len; cp++)
			if (*cp != '0' && *cp != ':')
				return 0;
		return 1;
	}
	return (sx->len == 7 && memcmp("0.0.0.0", sx->s, 7) == 0);
}

/*
 * ser_memmem() returns the location of the first occurrence of data
 * pattern b2 of size len2 in memory block b1 of size len1 or
 * NULL if none is found. Obtained from NetBSD.
 */
static void *
ser_memmem(const void *b1, const void *b2, size_t len1, size_t len2)
{
	/* Initialize search pointer */
	char *sp = (char *) b1;

	/* Initialize pattern pointer */
	char *pp = (char *) b2;

	/* Initialize end of search address space pointer */
	char *eos = sp + len1 - len2;

	/* Sanity check */
	if(!(b1 && b2 && len1 && len2))
		return NULL;

	while (sp <= eos) {
		if (*sp == *pp)
			if (memcmp(sp, pp, len2) == 0)
				return sp;

			sp++;
	}

	return NULL;
}

/*
 * Some helper functions taken verbatim from tm module.
 */

/*
 * Extract tag from To header field of a response
 * assumes the to header is already parsed, so
 * make sure it really is before calling this function
 */
static inline int
get_to_tag(struct sip_msg* _m, str* _tag)
{

	if (!_m->to) {
		LOG(L_ERR, "get_to_tag(): To header field missing\n");
		return -1;
	}

	if (get_to(_m)->tag_value.len) {
		_tag->s = get_to(_m)->tag_value.s;
		_tag->len = get_to(_m)->tag_value.len;
	} else {
		_tag->s = 0; /* fixes gcc 4.0 warnings */
		_tag->len = 0;
	}

	return 0;
}


/*
 * Extract tag from From header field of a request
 */
static inline int
get_from_tag(struct sip_msg* _m, str* _tag)
{

	if (parse_from_header(_m) == -1) {
		LOG(L_ERR, "get_from_tag(): Error while parsing From header\n");
		return -1;
	}

	if (get_from(_m)->tag_value.len) {
		_tag->s = get_from(_m)->tag_value.s;
		_tag->len = get_from(_m)->tag_value.len;
	} else {
		_tag->len = 0;
	}

	return 0;
}

/*
 * Extract Call-ID value
 * assumes the callid header is already parsed
 * (so make sure it is, before calling this function or
 *  it might fail even if the message _has_ a callid)
 */
static inline int
get_callid(struct sip_msg* _m, str* _cid)
{

	if ((parse_headers(_m, HDR_CALLID_F, 0) == -1)) {
		LOG(L_ERR, "get_callid(): parse_headers() failed\n");
		return -1;
	}

	if (_m->callid == NULL) {
		LOG(L_ERR, "get_callid(): Call-ID not found\n");
		return -1;
	}

	_cid->s = _m->callid->body.s;
	_cid->len = _m->callid->body.len;
	trim(_cid);
	return 0;
}

/*
 * Extract URI from the Contact header field
 */
static inline int
get_contact_uri(struct sip_msg* _m, struct sip_uri *uri, contact_t** _c)
{

	if ((parse_headers(_m, HDR_CONTACT_F, 0) == -1) || !_m->contact)
		return -1;
	if (!_m->contact->parsed && parse_contact(_m->contact) < 0) {
		LOG(L_ERR, "get_contact_uri: Error while parsing Contact body\n");
		return -1;
	}
	*_c = ((contact_body_t*)_m->contact->parsed)->contacts;
	if (*_c == NULL) {
		LOG(L_ERR, "get_contact_uri: Error while parsing Contact body\n");
		return -1;
	}
	if (parse_uri((*_c)->uri.s, (*_c)->uri.len, uri) < 0 || uri->host.len <= 0) {
		LOG(L_ERR, "get_contact_uri: Error while parsing Contact URI\n");
		return -1;
	}
	return 0;
}

/*
 * Replaces ip:port pair in the Contact: field with the source address
 * of the packet.
 */
static int
fix_nated_contact_f(struct sip_msg* msg, char* str1, char* str2)
{
	int offset, len, len1;
	char *cp, *buf, temp[2];
	contact_t *c;
	struct lump *anchor;
	struct sip_uri uri;
	str hostport;

	if (get_contact_uri(msg, &uri, &c) == -1)
		return -1;
	/* for UAs behind NAT we have to hope that they will reuse the
	 * TCP connection, otherwise they are lost any way. So this check
	 * does not make too much sense.
	if (uri.proto != PROTO_UDP && uri.proto != PROTO_NONE)
		return -1;
	*/
	if ((c->uri.s < msg->buf) || (c->uri.s > (msg->buf + msg->len))) {
		LOG(L_ERR, "ERROR: you can't call fix_nated_contact twice, "
		    "check your config!\n");
		return -1;
	}

	offset = c->uri.s - msg->buf;
	anchor = del_lump(msg, offset, c->uri.len, HDR_CONTACT_T);
	if (anchor == 0)
		return -1;

	hostport = uri.host;
	if (uri.port.len > 0)
		hostport.len = uri.port.s + uri.port.len - uri.host.s;

	cp = ip_addr2a(&msg->rcv.src_ip);
	len = c->uri.len + strlen(cp) + 6 /* :port */ - hostport.len + 1;
	buf = pkg_malloc(len);
	if (buf == NULL) {
		LOG(L_ERR, "ERROR: fix_nated_contact: out of memory\n");
		return -1;
	}
	temp[0] = hostport.s[0];
	temp[1] = c->uri.s[c->uri.len];
	c->uri.s[c->uri.len] = hostport.s[0] = '\0';
	len1 = snprintf(buf, len, "%s%s:%d%s", c->uri.s, cp, msg->rcv.src_port,
	    hostport.s + hostport.len);
	if (len1 < len)
		len = len1;
	hostport.s[0] = temp[0];
	c->uri.s[c->uri.len] = temp[1];
	if (insert_new_lump_after(anchor, buf, len, HDR_CONTACT_T) == 0) {
		pkg_free(buf);
		return -1;
	}
	c->uri.s = buf;
	c->uri.len = len;

	return 1;
}

static int sel_rewrite_contact(str* res, select_t* s, struct sip_msg* msg) {
	static char buf[500];
	contact_t* c;
	int n, def_port_fl, len;
	char *cp;
	str hostport;
	struct sip_uri uri;

	res->len = 0;
	n = s->params[2].v.i;
	if (n <= 0) {
		LOG(L_ERR, "ERROR: rewrite_contact[%d]: zero or negative index not supported\n", n);
		return -1;
	}
	c = 0;
	do {
		if (contact_iterator(&c, msg, c) < 0 || !c)
			return -1;
		n--;
	} while (n > 0);

	if (parse_uri(c->uri.s, c->uri.len, &uri) < 0 || uri.host.len <= 0) {
		LOG(L_ERR, "rewrite_contact[%d]: Error while parsing Contact URI\n", s->params[2].v.i);
		return -1;
	}
	len = c->len - uri.host.len;
	if (uri.port.len > 0)
		len -= uri.port.len;
	def_port_fl = (msg->rcv.proto == PROTO_TLS && msg->rcv.src_port == SIPS_PORT) || (msg->rcv.proto != PROTO_TLS && msg->rcv.src_port == SIP_PORT);
	if (!def_port_fl)
		len += 1/*:*/+5/*port*/;
	if (len > sizeof(buf)) {
		LOG(L_ERR, "ERROR: rewrite_contact[%d]: contact too long\n", s->params[2].v.i);
		return -1;
	}
	hostport = uri.host;
	if (uri.port.len > 0)
		hostport.len = uri.port.s + uri.port.len - uri.host.s;

	res->s = buf;
	res->len = hostport.s - c->name.s;
	memcpy(buf, c->name.s, res->len);
	cp = ip_addr2a(&msg->rcv.src_ip);
	if (def_port_fl) {
		res->len+= snprintf(buf+res->len, sizeof(buf)-res->len, "%s", cp);
	} else {
		res->len+= snprintf(buf+res->len, sizeof(buf)-res->len, "%s:%d", cp, msg->rcv.src_port);
	}
	memcpy(buf+res->len, hostport.s+hostport.len, c->len-(hostport.s+hostport.len-c->name.s));
	res->len+= c->len-(hostport.s+hostport.len-c->name.s);

	return 0;
}

/*
 * Test if IP address pointed to by saddr belongs to RFC1918 networks
 */
static inline int
is1918addr(str *saddr)
{
	struct in_addr addr;
	uint32_t netaddr;
	int i, rval;
	char backup;

	rval = -1;
	backup = saddr->s[saddr->len];
	saddr->s[saddr->len] = '\0';
	if (inet_aton(saddr->s, &addr) != 1)
		goto theend;
	netaddr = ntohl(addr.s_addr);
	for (i = 0; nets_1918[i].cnetaddr != NULL; i++) {
		if ((netaddr & nets_1918[i].mask) == nets_1918[i].netaddr) {
			rval = 1;
			goto theend;
		}
	}
	rval = 0;

theend:
	saddr->s[saddr->len] = backup;
	return rval;
}

/*
 * test for occurrence of RFC1918 IP address in Contact HF
 */
static int
contact_1918(struct sip_msg* msg)
{
	struct sip_uri uri;
	contact_t* c;

	if (get_contact_uri(msg, &uri, &c) == -1)
		return -1;

	return (is1918addr(&(uri.host)) == 1) ? 1 : 0;
}

/*
 * test for occurrence of RFC1918 IP address in SDP
 */
static int
sdp_1918(struct sip_msg* msg)
{
	str body, ip;
	int pf;

	if (extract_sdp_body(msg, &body) == -1) {
		LOG(L_ERR,"ERROR: sdp_1918: cannot extract body from msg!\n");
		return 0;
	}
	if (extract_mediaip(&body, &ip, &pf) == -1) {
		LOG(L_ERR, "ERROR: sdp_1918: can't extract media IP from the SDP\n");
		return 0;
	}
	if (pf != AF_INET || isnulladdr(&ip, pf))
		return 0;

	return (is1918addr(&ip) == 1) ? 1 : 0;
}

/*
 * test for occurrence of RFC1918 IP address in top Via
 */
static int
via_1918(struct sip_msg* msg)
{

	return (is1918addr(&(msg->via1->host)) == 1) ? 1 : 0;
}

static int
nat_uac_test_f(struct sip_msg* msg, char* str1, char* str2)
{
	int tests;

	if (get_int_fparam(&tests, msg, (fparam_t*) str1) < -1) return -1;

	/* return true if any of the NAT-UAC tests holds */

	/* test if the source port is different from the port in Via */
	if ((tests & NAT_UAC_TEST_RPORT) &&
		 (msg->rcv.src_port!=(msg->via1->port?msg->via1->port:SIP_PORT)) ){
		return 1;
	}
	/*
	 * test if source address of signaling is different from
	 * address advertised in Via
	 */
	if ((tests & NAT_UAC_TEST_RCVD) && received_test(msg))
		return 1;
	/*
	 * test for occurrences of RFC1918 addresses in Contact
	 * header field
	 */
	if ((tests & NAT_UAC_TEST_C_1918) && (contact_1918(msg)>0))
		return 1;
	/*
	 * test for occurrences of RFC1918 addresses in SDP body
	 */
	if ((tests & NAT_UAC_TEST_S_1918) && sdp_1918(msg))
		return 1;
	/*
	 * test for occurrences of RFC1918 addresses top Via
	 */
	if ((tests & NAT_UAC_TEST_V_1918) && via_1918(msg))
		return 1;

	/* no test succeeded */
	return -1;

}

#define	ADD_ADIRECTION	0x01
#define	FIX_MEDIP	0x02
#define	ADD_ANORTPPROXY	0x04
#define ADD_ADIRECTIONP 0x08

#define	ADIRECTION	"a=direction:active\r\n"
#define	ADIRECTION_LEN	(sizeof(ADIRECTION) - 1)

#define ADIRECTIONP     "a=direction:passive\r\n"
#define ADIRECTIONP_LEN (sizeof(ADIRECTIONP) - 1)

#define	AOLDMEDIP	"a=oldmediaip:"
#define	AOLDMEDIP_LEN	(sizeof(AOLDMEDIP) - 1)

#define	AOLDMEDIP6	"a=oldmediaip6:"
#define	AOLDMEDIP6_LEN	(sizeof(AOLDMEDIP6) - 1)

#define	AOLDMEDPRT	"a=oldmediaport:"
#define	AOLDMEDPRT_LEN	(sizeof(AOLDMEDPRT) - 1)

#define	ANORTPPROXY	"a=nortpproxy:yes\r\n"
#define	ANORTPPROXY_LEN	(sizeof(ANORTPPROXY) - 1)

static int
fix_nated_sdp_f(struct sip_msg* msg, char* str1, char* str2)
{
	str body, body1, oldip, newip;
	int level, pf;
	char *buf;
	struct lump* anchor;

	if (get_int_fparam(&level, msg, (fparam_t*) str1) < -1) return -1;
	
	if (extract_sdp_body(msg, &body) == -1) {
		LOG(L_ERR,"ERROR: fix_nated_sdp: cannot extract body from msg!\n");
		return -1;
	}

	if (level & (ADD_ADIRECTION | ADD_ANORTPPROXY | ADD_ADIRECTIONP)) {
		msg->msg_flags |= FL_FORCE_ACTIVE;
		anchor = anchor_lump(msg, body.s + body.len - msg->buf, 0, 0);
		if (anchor == NULL) {
			LOG(L_ERR, "ERROR: fix_nated_sdp: anchor_lump failed\n");
			return -1;
		}
		if (level & ADD_ADIRECTION) {
			buf = pkg_malloc(ADIRECTION_LEN * sizeof(char));
			if (buf == NULL) {
				LOG(L_ERR, "ERROR: fix_nated_sdp: out of memory\n");
				return -1;
			}
			memcpy(buf, ADIRECTION, ADIRECTION_LEN);
			if (insert_new_lump_after(anchor, buf, ADIRECTION_LEN, 0) == NULL) {
				LOG(L_ERR, "ERROR: fix_nated_sdp: insert_new_lump_after failed\n");
				pkg_free(buf);
				return -1;
			}
		}
		if (level & ADD_ADIRECTIONP) {
			buf = pkg_malloc(ADIRECTIONP_LEN * sizeof(char));
			if (buf == NULL) {
				LOG(L_ERR, "ERROR: fix_nated_sdp: out of memory\n");
				return -1;
			}
			memcpy(buf, ADIRECTIONP, ADIRECTIONP_LEN);
			if (insert_new_lump_after(anchor, buf, ADIRECTIONP_LEN, 0) == NULL) {
				LOG(L_ERR, "ERROR: fix_nated_sdp: insert_new_lump_after failed\n");
				pkg_free(buf);
				return -1;
			}
		}
		if (level & ADD_ANORTPPROXY) {
			buf = pkg_malloc(ANORTPPROXY_LEN * sizeof(char));
			if (buf == NULL) {
				LOG(L_ERR, "ERROR: fix_nated_sdp: out of memory\n");
				return -1;
			}
			memcpy(buf, ANORTPPROXY, ANORTPPROXY_LEN);
			if (insert_new_lump_after(anchor, buf, ANORTPPROXY_LEN, 0) == NULL) {
				LOG(L_ERR, "ERROR: fix_nated_sdp: insert_new_lump_after failed\n");
				pkg_free(buf);
				return -1;
			}
		}
	}

	if (level & FIX_MEDIP) {
 		/* Iterate all c= and replace ips in them. */
 		unsigned hasreplaced = 0;
 		int pf1 = 0;
 		str body2;
 		char *bodylimit = body.s + body.len;
 		newip.s = ip_addr2a(&msg->rcv.src_ip);
 		newip.len = strlen(newip.s);
 		body1 = body;
 		for(;;) {
 			if (extract_mediaip(&body1, &oldip, &pf) == -1)
 				break;
 			if (pf != AF_INET) {
 				LOG(L_ERR, "ERROR: fix_nated_sdp: "
 				    "not an IPv4 address in SDP\n");
 				goto finalize;
 			}
 			if (!pf1)
 				pf1 = pf;
 			else if (pf != pf1) {
 				LOG(L_ERR, "ERROR: fix_nated_sdp: mismatching "
 				    "address families in SDP\n");
 				return -1;
 			}
 			body2.s = oldip.s + oldip.len;
 			body2.len = bodylimit - body2.s;
 			if (alter_mediaip(msg, &body1, &oldip, pf, &newip, pf,
 			    1) == -1)
 			{
 				LOG(L_ERR, "ERROR: fix_nated_sdp: can't alter media IP\n");
 				return -1;
 			}
 			hasreplaced = 1;
 			body1 = body2;
 		}
 		if (!hasreplaced) {
 			LOG(L_ERR, "ERROR: fix_nated_sdp: can't extract media IP from the SDP\n");
 			goto finalize;
 		}
	}

finalize:
	return 1;
}

static int
extract_mediaip(str *body, str *mediaip, int *pf)
{
	char *cp, *cp1;
	int len, nextisip;

	cp1 = NULL;
	for (cp = body->s; (len = body->s + body->len - cp) > 0;) {
		cp1 = ser_memmem(cp, "c=", len, 2);
		if (cp1 == NULL || cp1[-1] == '\n' || cp1[-1] == '\r')
			break;
		cp = cp1 + 2;
	}
	if (cp1 == NULL) {
		LOG(L_ERR, "ERROR: extract_mediaip: no `c=' in SDP\n");
		return -1;
	}
	mediaip->s = cp1 + 2;
	mediaip->len = eat_line(mediaip->s, body->s + body->len - mediaip->s) - mediaip->s;
	trim_len(mediaip->len, mediaip->s, *mediaip);

	nextisip = 0;
	for (cp = mediaip->s; cp < mediaip->s + mediaip->len;) {
		len = eat_token_end(cp, mediaip->s + mediaip->len) - cp;
		if (nextisip == 1) {
			mediaip->s = cp;
			mediaip->len = len;
			nextisip++;
			break;
		}
		if (len == 3 && memcmp(cp, "IP", 2) == 0) {
			switch (cp[2]) {
			case '4':
				nextisip = 1;
				*pf = AF_INET;
				break;

			case '6':
				nextisip = 1;
				*pf = AF_INET6;
				break;

			default:
				break;
			}
		}
		cp = eat_space_end(cp + len, mediaip->s + mediaip->len);
	}
	if (nextisip != 2 || mediaip->len == 0) {
		LOG(L_ERR, "ERROR: extract_mediaip: "
		    "no `IP[4|6]' in `c=' field\n");
		return -1;
	}
	return 1;
}

static int
extract_mediaport(str *body, str *mediaport)
{
	char *cp, *cp1;
	int len, i;
	str ptype;

	cp1 = NULL;
	for (cp = body->s; (len = body->s + body->len - cp) > 0;) {
		cp1 = ser_memmem(cp, "m=", len, 2);
		if (cp1 == NULL || cp1[-1] == '\n' || cp1[-1] == '\r')
			break;
		cp = cp1 + 2;
	}
	if (cp1 == NULL) {
		LOG(L_ERR, "ERROR: extract_mediaport: no `m=' in SDP\n");
		return -1;
	}
	mediaport->s = cp1 + 2; /* skip `m=' */
	mediaport->len = eat_line(mediaport->s, body->s + body->len -
	  mediaport->s) - mediaport->s;
	trim_len(mediaport->len, mediaport->s, *mediaport);

	/* Skip media supertype and spaces after it */
	cp = eat_token_end(mediaport->s, mediaport->s + mediaport->len);
	mediaport->len -= cp - mediaport->s;
	if (mediaport->len <= 0 || cp == mediaport->s) {
		LOG(L_ERR, "ERROR: extract_mediaport: no port in `m='\n");
		return -1;
	}
	mediaport->s = cp;
	cp = eat_space_end(mediaport->s, mediaport->s + mediaport->len);
	mediaport->len -= cp - mediaport->s;
	if (mediaport->len <= 0 || cp == mediaport->s) {
		LOG(L_ERR, "ERROR: extract_mediaport: no port in `m='\n");
		return -1;
	}
	/* Extract port */
	mediaport->s = cp;
	cp = eat_token_end(mediaport->s, mediaport->s + mediaport->len);
	ptype.len = mediaport->len - (cp - mediaport->s);
	if (ptype.len <= 0 || cp == mediaport->s) {
		LOG(L_ERR, "ERROR: extract_mediaport: no port in `m='\n");
		return -1;
	}
	ptype.s = cp;
	mediaport->len = cp - mediaport->s;
	/* Skip spaces after port */
	cp = eat_space_end(ptype.s, ptype.s + ptype.len);
	ptype.len -= cp - ptype.s;
	if (ptype.len <= 0 || cp == ptype.s) {
		LOG(L_ERR, "ERROR: extract_mediaport: no protocol type in `m='\n");
		return -1;
	}
	/* Extract protocol type */
	ptype.s = cp;
	cp = eat_token_end(ptype.s, ptype.s + ptype.len);
	if (cp == ptype.s) {
		LOG(L_ERR, "ERROR: extract_mediaport: no protocol type in `m='\n");
		return -1;
	}
	ptype.len = cp - ptype.s;

	for (i = 0; sup_ptypes[i].s != NULL; i++)
		if (ptype.len == sup_ptypes[i].len &&
		    strncasecmp(ptype.s, sup_ptypes[i].s, ptype.len) == 0)
			return 0;
	/* Unproxyable protocol type. Generally it isn't error. */
	return -1;
}

static int
alter_mediaip(struct sip_msg *msg, str *body, str *oldip, int oldpf,
  str *newip, int newpf, int preserve)
{
	char *buf;
	int offset;
	struct lump* anchor;
	str omip, nip, oip;

	/* check that updating mediaip is really necessary */
	if (oldpf == newpf && isnulladdr(oldip, oldpf))
		return 0;
	if (newip->len == oldip->len &&
	    memcmp(newip->s, oldip->s, newip->len) == 0)
		return 0;

	/*
	 * Since rewriting the same info twice will mess SDP up,
	 * apply simple anti foot shooting measure - put flag on
	 * messages that have been altered and check it when
	 * another request comes.
	 */
#if 0
	/* disabled:
	 *  - alter_mediaip is called twice if 2 c= lines are present
	 *    in the sdp (and we want to allow it)
	 *  - the message flags are propagated in the on_reply_route
	 *  => if we set the flags for the request they will be seen for the
	 *    reply too, but we don't want that
	 *  --andrei
	 */
	if (msg->msg_flags & FL_SDP_IP_AFS) {
		LOG(L_ERR, "ERROR: alter_mediaip: you can't rewrite the same "
		  "SDP twice, check your config!\n");
		return -1;
	}
#endif

	if (preserve != 0) {
		anchor = anchor_lump(msg, body->s + body->len - msg->buf, 0, 0);
		if (anchor == NULL) {
			LOG(L_ERR, "ERROR: alter_mediaip: anchor_lump failed\n");
			return -1;
		}
		if (oldpf == AF_INET6) {
			omip.s = AOLDMEDIP6;
			omip.len = AOLDMEDIP6_LEN;
		} else {
			omip.s = AOLDMEDIP;
			omip.len = AOLDMEDIP_LEN;
		}
		buf = pkg_malloc(omip.len + oldip->len + CRLF_LEN);
		if (buf == NULL) {
			LOG(L_ERR, "ERROR: alter_mediaip: out of memory\n");
			return -1;
		}
		memcpy(buf, omip.s, omip.len);
		memcpy(buf + omip.len, oldip->s, oldip->len);
		memcpy(buf + omip.len + oldip->len, CRLF, CRLF_LEN);
		if (insert_new_lump_after(anchor, buf,
		    omip.len + oldip->len + CRLF_LEN, 0) == NULL) {
			LOG(L_ERR, "ERROR: alter_mediaip: insert_new_lump_after failed\n");
			pkg_free(buf);
			return -1;
		}
	}

	if (oldpf == newpf) {
		nip.len = newip->len;
		nip.s = pkg_malloc(nip.len);
		if (nip.s == NULL) {
			LOG(L_ERR, "ERROR: alter_mediaip: out of memory\n");
			return -1;
		}
		memcpy(nip.s, newip->s, newip->len);
	} else {
		nip.len = newip->len + 2;
		nip.s = pkg_malloc(nip.len);
		if (nip.s == NULL) {
			LOG(L_ERR, "ERROR: alter_mediaip: out of memory\n");
			return -1;
		}
		memcpy(nip.s + 2, newip->s, newip->len);
		nip.s[0] = (newpf == AF_INET6) ? '6' : '4';
		nip.s[1] = ' ';
	}

	oip = *oldip;
	if (oldpf != newpf) {
		do {
			oip.s--;
			oip.len++;
		} while (*oip.s != '6' && *oip.s != '4');
	}
	offset = oip.s - msg->buf;
	anchor = del_lump(msg, offset, oip.len, 0);
	if (anchor == NULL) {
		LOG(L_ERR, "ERROR: alter_mediaip: del_lump failed\n");
		pkg_free(nip.s);
		return -1;
	}

#if 0
	msg->msg_flags |= FL_SDP_IP_AFS;
#endif

	if (insert_new_lump_after(anchor, nip.s, nip.len, 0) == 0) {
		LOG(L_ERR, "ERROR: alter_mediaip: insert_new_lump_after failed\n");
		pkg_free(nip.s);
		return -1;
	}
	return 0;
}

static int
alter_mediaport(struct sip_msg *msg, str *body, str *oldport, str *newport,
  int preserve)
{
	char *buf;
	int offset;
	struct lump* anchor;

	/* check that updating mediaport is really necessary */
	if (newport->len == oldport->len &&
	    memcmp(newport->s, oldport->s, newport->len) == 0)
		return 0;

	/*
	 * Since rewriting the same info twice will mess SDP up,
	 * apply simple anti foot shooting measure - put flag on
	 * messages that have been altered and check it when
	 * another request comes.
	 */
#if 0
	/* disabled: - it propagates to the reply and we don't want this
	 *  -- andrei */
	if (msg->msg_flags & FL_SDP_PORT_AFS) {
		LOG(L_ERR, "ERROR: alter_mediaip: you can't rewrite the same "
		  "SDP twice, check your config!\n");
		return -1;
	}
#endif

	if (preserve != 0) {
		anchor = anchor_lump(msg, body->s + body->len - msg->buf, 0, 0);
		if (anchor == NULL) {
			LOG(L_ERR, "ERROR: alter_mediaport: anchor_lump failed\n");
			return -1;
		}
		buf = pkg_malloc(AOLDMEDPRT_LEN + oldport->len + CRLF_LEN);
		if (buf == NULL) {
			LOG(L_ERR, "ERROR: alter_mediaport: out of memory\n");
			return -1;
		}
		memcpy(buf, AOLDMEDPRT, AOLDMEDPRT_LEN);
		memcpy(buf + AOLDMEDPRT_LEN, oldport->s, oldport->len);
		memcpy(buf + AOLDMEDPRT_LEN + oldport->len, CRLF, CRLF_LEN);
		if (insert_new_lump_after(anchor, buf,
		    AOLDMEDPRT_LEN + oldport->len + CRLF_LEN, 0) == NULL) {
			LOG(L_ERR, "ERROR: alter_mediaport: insert_new_lump_after failed\n");
			pkg_free(buf);
			return -1;
		}
	}

	buf = pkg_malloc(newport->len);
	if (buf == NULL) {
		LOG(L_ERR, "ERROR: alter_mediaport: out of memory\n");
		return -1;
	}
	offset = oldport->s - msg->buf;
	anchor = del_lump(msg, offset, oldport->len, 0);
	if (anchor == NULL) {
		LOG(L_ERR, "ERROR: alter_mediaport: del_lump failed\n");
		pkg_free(buf);
		return -1;
	}
	memcpy(buf, newport->s, newport->len);
	if (insert_new_lump_after(anchor, buf, newport->len, 0) == 0) {
		LOG(L_ERR, "ERROR: alter_mediaport: insert_new_lump_after failed\n");
		pkg_free(buf);
		return -1;
	}

#if 0
	msg->msg_flags |= FL_SDP_PORT_AFS;
#endif
	return 0;
}

static char *
gencookie()
{
	static char cook[34];

	sprintf(cook, "%d_%u ", (int)mypid, myseqn);
	myseqn++;
	return cook;
}

static int
rtpp_test(struct rtpp_node *node, int isdisabled, int force)
{
	int rtpp_ver;
	char *cp;
	struct iovec v[2] = {{NULL, 0}, {"V", 1}};
	struct iovec vf[4] = {{NULL, 0}, {"VF", 2}, {" ", 1},
	    {REQ_CPROTOVER, 8}};

	if (force == 0) {
		if (isdisabled == 0)
			return 0;
		if (node->rn_recheck_ticks > get_ticks())
			return 1;
	}
	do {
		cp = send_rtpp_command(node, v, 2);
		if (cp == NULL) {
			LOG(L_WARN,"WARNING: rtpp_test: can't get version of "
			    "the RTP proxy\n");
			break;
		}
		rtpp_ver = atoi(cp);
		if (rtpp_ver != SUP_CPROTOVER) {
			LOG(L_WARN, "WARNING: rtpp_test: unsupported "
			    "version of RTP proxy <%s> found: %d supported, "
			    "%d present\n", node->rn_url,
			    SUP_CPROTOVER, rtpp_ver);
			break;
		}
		cp = send_rtpp_command(node, vf, 4);
		if (cp == NULL) {
			LOG(L_WARN,"WARNING: rtpp_test: RTP proxy went down during "
			    "version query\n");
			break;
		}
		if (cp[0] == 'E' || atoi(cp) != 1) {
			LOG(L_WARN, "WARNING: rtpp_test: of RTP proxy <%s>"
			    "doesn't support required protocol version %s\n",
			    node->rn_url, REQ_CPROTOVER);
			break;
		}
		LOG(L_INFO, "rtpp_test: RTP proxy <%s> found, support for "
		    "it %senabled\n",
		    node->rn_url, force == 0 ? "re-" : "");
		return 0;
	} while(0);
	LOG(L_WARN, "WARNING: rtpp_test: support for RTP proxy <%s>"
	    "has been disabled%s\n", node->rn_url,
	    rtpproxy_disable_tout < 0 ? "" : " temporarily");
	if (rtpproxy_disable_tout >= 0)
		node->rn_recheck_ticks = get_ticks() + rtpproxy_disable_tout;

	return 1;
}

static char *
send_rtpp_command(struct rtpp_node *node, struct iovec *v, int vcnt)
{
	struct sockaddr_un addr;
	int fd, len, i;
	char *cp;
	static char buf[256];
	struct pollfd fds[1];

	len = 0;
	cp = buf;
	if (node->rn_umode == 0) {
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_LOCAL;
		strncpy(addr.sun_path, node->rn_address,
		    sizeof(addr.sun_path) - 1);
#ifdef HAVE_SOCKADDR_SA_LEN
		addr.sun_len = strlen(addr.sun_path);
#endif

		fd = socket(AF_LOCAL, SOCK_STREAM, 0);
		if (fd < 0) {
			LOG(L_ERR, "ERROR: send_rtpp_command: can't create socket\n");
			goto badproxy;
		}
		if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			close(fd);
			LOG(L_ERR, "ERROR: send_rtpp_command: can't connect to RTP proxy\n");
			goto badproxy;
		}

		do {
			len = writev(fd, v + 1, vcnt - 1);
		} while (len == -1 && errno == EINTR);
		if (len <= 0) {
			close(fd);
			LOG(L_ERR, "ERROR: send_rtpp_command: can't send command to a RTP proxy\n");
			goto badproxy;
		}
		do {
			len = read(fd, buf, sizeof(buf) - 1);
		} while (len == -1 && errno == EINTR);
		close(fd);
		if (len <= 0) {
			LOG(L_ERR, "ERROR: send_rtpp_command: can't read reply from a RTP proxy\n");
			goto badproxy;
		}
	} else {
		fds[0].fd = node->rn_fd;
		fds[0].events = POLLIN;
		fds[0].revents = 0;
		/* Drain input buffer */
		while ((poll(fds, 1, 0) == 1) &&
		    ((fds[0].revents & POLLIN) != 0)) {
			recv(node->rn_fd, buf, sizeof(buf) - 1, 0);
			fds[0].revents = 0;
		}
		v[0].iov_base = gencookie();
		v[0].iov_len = strlen(v[0].iov_base);
		for (i = 0; i < rtpproxy_retr; i++) {
			do {
				len = writev(node->rn_fd, v, vcnt);
			} while (len == -1 && (errno == EINTR || errno == ENOBUFS));
			if (len <= 0) {
				LOG(L_ERR, "ERROR: send_rtpp_command: "
				    "can't send command to a RTP proxy\n");
				goto badproxy;
			}
			while ((poll(fds, 1, rtpproxy_tout * 1000) == 1) &&
			    (fds[0].revents & POLLIN) != 0) {
				do {
					len = recv(node->rn_fd, buf, sizeof(buf) - 1, 0);
				} while (len == -1 && errno == EINTR);
				if (len <= 0) {
					LOG(L_ERR, "ERROR: send_rtpp_command: "
					    "can't read reply from a RTP proxy\n");
					goto badproxy;
				}
				if (len >= (v[0].iov_len - 1) &&
				    memcmp(buf, v[0].iov_base, (v[0].iov_len - 1)) == 0) {
					len -= (v[0].iov_len - 1);
					cp += (v[0].iov_len - 1);
					if (len != 0) {
						len--;
						cp++;
					}
					goto out;
				}
				fds[0].revents = 0;
			}
		}
		if (i == rtpproxy_retr) {
			LOG(L_ERR, "ERROR: send_rtpp_command: "
			    "timeout waiting reply from a RTP proxy\n");
			goto badproxy;
		}
	}

out:
	cp[len] = '\0';
	return cp;
badproxy:
	LOG(L_ERR, "send_rtpp_command(): proxy <%s> does not responding, disable it\n", node->rn_url);
	node->rn_disabled = 1;
	node->rn_recheck_ticks = get_ticks() + rtpproxy_disable_tout;
	return NULL;
}

/*
 * Main balancing routine. This does not try to keep the same proxy for
 * the call if some proxies were disabled or enabled; proxy death considered
 * too rare. Otherwise we should implement "mature" HA clustering, which is
 * too expensive here.
 */
static struct rtpp_node *
select_rtpp_node(str callid, int do_test, int node_idx)
{
	unsigned sum, sumcut, weight_sum;
	struct rtpp_node* node;
	int was_forced;

	/* Most popular case: 1 proxy, nothing to calculate */
	if (rtpp_node_count == 1) {
		if (node_idx > 0) {
			LOG(L_ERR, "ERROR: select_rtpp_node: node index out or range\n");
			return NULL;
		}
		node = rtpp_list.rn_first;
		if (node->rn_disabled && node->rn_recheck_ticks <= get_ticks()) {
			/* Try to enable if it's time to try. */
			node->rn_disabled = rtpp_test(node, 1, 0);
		}
		return node->rn_disabled ? NULL : node;
	}

	if (node_idx != -1) {
		for (node = rtpp_list.rn_first; node != NULL; node = node->rn_next) {
			if (node_idx > 0) {
				node_idx--;
				continue;
			}
			if (node->rn_disabled && node->rn_recheck_ticks <= get_ticks()) {
				/* Try to enable if it's time to try. */
				node->rn_disabled = rtpp_test(node, 1, 0);
			}
			return node->rn_disabled ? NULL : node;
		}
		LOG(L_ERR, "ERROR: select_rtpp_node: node index out or range\n");
		return NULL;
	}

	/* XXX Use quick-and-dirty hashing algo */
	for(sum = 0; callid.len > 0; callid.len--)
		sum += callid.s[callid.len - 1];
	sum &= 0xff;

	was_forced = 0;
retry:
	weight_sum = 0;
	for (node = rtpp_list.rn_first; node != NULL; node = node->rn_next) {
		if (node->rn_disabled && node->rn_recheck_ticks <= get_ticks()) {
			/* Try to enable if it's time to try. */
			node->rn_disabled = rtpp_test(node, 1, 0);
		}
		if (!node->rn_disabled)
			weight_sum += node->rn_weight;
	}
	if (weight_sum == 0) {
		/* No proxies? Force all to be redetected, if not yet */
		if (was_forced)
			return NULL;
		was_forced = 1;
		for (node = rtpp_list.rn_first; node != NULL; node = node->rn_next) {
			node->rn_disabled = rtpp_test(node, 1, 1);
		}
		goto retry;
	}
	sumcut = sum % weight_sum;
	/*
	 * sumcut here lays from 0 to weight_sum-1.
	 * Scan proxy list and decrease until appropriate proxy is found.
	 */
	for (node = rtpp_list.rn_first; node != NULL; node = node->rn_next) {
		if (node->rn_disabled)
			continue;
		if (sumcut < node->rn_weight)
			goto found;
		sumcut -= node->rn_weight;
	}
	/* No node list */
	return NULL;
found:
	if (do_test) {
		node->rn_disabled = rtpp_test(node, node->rn_disabled, 0);
		if (node->rn_disabled)
			goto retry;
	}
	return node;
}

static int
unforce_rtp_proxy_f(struct sip_msg* msg, int node_idx)
{
	str callid, from_tag, to_tag;
	struct rtpp_node *node;
	struct iovec v[1 + 4 + 3] = {{NULL, 0}, {"D", 1}, {" ", 1}, {NULL, 0}, {" ", 1}, {NULL, 0}, {" ", 1}, {NULL, 0}};
						/* 1 */   /* 2 */   /* 3 */    /* 4 */   /* 5 */    /* 6 */   /* 1 */

	if (get_callid(msg, &callid) == -1 || callid.len == 0) {
		LOG(L_ERR, "ERROR: unforce_rtp_proxy: can't get Call-Id field\n");
		return -1;
	}
	if (get_to_tag(msg, &to_tag) == -1) {
		LOG(L_ERR, "ERROR: unforce_rtp_proxy: can't get To tag\n");
		return -1;
	}
	if (get_from_tag(msg, &from_tag) == -1 || from_tag.len == 0) {
		LOG(L_ERR, "ERROR: unforce_rtp_proxy: can't get From tag\n");
		return -1;
	}
	STR2IOVEC(callid, v[3]);
	STR2IOVEC(from_tag, v[5]);
	STR2IOVEC(to_tag, v[7]);
	node = select_rtpp_node(callid, 1, node_idx);
	if (!node) {
		LOG(L_ERR, "ERROR: unforce_rtp_proxy: no available proxies\n");
		return -1;
	}
	send_rtpp_command(node, v, (to_tag.len > 0) ? 8 : 6);

	return 1;
}

static int
unforce_rtp_proxy0_f(struct sip_msg* msg, char* str1, char* str2)
{

	return unforce_rtp_proxy_f(msg, -1);
}

static int
unforce_rtp_proxy1_f(struct sip_msg* msg, char* str1, char* str2)
{
	int node_idx;
	str rtpnode;

	if (get_str_fparam(&rtpnode, msg, (fparam_t*) str1) < -1) {
		 ERR("force_rtp_proxy(): Error while getting rtp_proxy node (fparam '%s')\n", ((fparam_t*)str1)->orig);
		
		return -1;
	}

	str2int(&rtpnode, (unsigned int*)&node_idx);
	return unforce_rtp_proxy_f(msg, node_idx);
}

/*
 * Auxiliary for some functions.
 * Returns pointer to first character of found line, or NULL if no such line.
 */

static char *
find_sdp_line(char* p, char* plimit, char linechar)
{
	static char linehead[3] = "x=";
	char *cp, *cp1;
	linehead[0] = linechar;
	/* Iterate thru body */
	cp = p;
	for (;;) {
		if (cp >= plimit)
			return NULL;
		cp1 = ser_memmem(cp, linehead, plimit-cp, 2);
		if (cp1 == NULL)
			return NULL;
		/*
		 * As it is body, we assume it has previous line and we can
		 * lookup previous character.
		 */
		if (cp1[-1] == '\n' || cp1[-1] == '\r')
			return cp1;
		/*
		 * Having such data, but not at line beginning.
		 * Skip them and reiterate. ser_memmem() will find next
		 * occurence.
		 */
		if (plimit - cp1 < 2)
			return NULL;
		cp = cp1 + 2;
	}
	/*UNREACHED*/
	return NULL;
}

/* This function assumes p points to a line of requested type. */

static char *
find_next_sdp_line(char* p, char* plimit, char linechar, char* defptr)
{
	char *t;
	if (p >= plimit || plimit - p < 3)
		return defptr;
	t = find_sdp_line(p + 2, plimit, linechar);
	return t ? t : defptr;
}

static int
force_rtp_proxy2_f(struct sip_msg* msg, char* param1, char* param2)
{
	str body, body1, oldport, oldip, newport, newip, str1, str2, s;
	str callid, from_tag, to_tag, tmp;
	int create, port, len, asymmetric, flookup, argc, proxied, real, i;
	int oidx, pf, pf1, force;
	unsigned int node_idx;
	char opts[16];
	char *cp, *cp1;
	char  *cpend, *next;
	char **ap, *argv[10];
	struct lump* anchor;
	struct rtpp_node *node;
	struct iovec v[14] = {
		{NULL, 0},	/* command */
		{NULL, 0},	/* options */
		{" ", 1},	/* separator */
		{NULL, 0},	/* callid */
		{" ", 1},	/* separator */
		{NULL, 7},	/* newip */
		{" ", 1},	/* separator */
		{NULL, 1},	/* oldport */
		{" ", 1},	/* separator */
		{NULL, 0},	/* from_tag */
		{";", 1},	/* separator */
		{NULL, 0},	/* medianum */
		{" ", 1},	/* separator */
		{NULL, 0}	/* to_tag */
	};
	char *v1p, *v2p, *c1p, *c2p, *m1p, *m2p, *bodylimit;
	char medianum_buf[20];
	int medianum, media_multi;
	str medianum_str, tmpstr1;
	int c1p_altered;

	if (param1) {
		if (get_str_fparam(&str1, msg, (fparam_t*)param1) < 0) {
			LOG(L_ERR, "force_rtp_proxy(): Error while parsing 1st param ('%s')\n", 
			    ((fparam_t*)param1)->orig);
			return -1;
		}
	}
	else 
		str1.len = 0;
	
	if (get_str_fparam(&str2, msg, (fparam_t*)param2) < 0) {
		LOG(L_ERR, "force_rtp_proxy(): Error while parsing 2nd param ('%s')\n", 
		    ((fparam_t*)param2)->orig);
		return -1;
	}

	v[1].iov_base=opts;
	asymmetric = flookup = force = real = 0;
	oidx = 1;
	node_idx = -1;
	for (i=0; i<str1.len; i++) {
		switch (str1.s[i]) {
		case ' ':
		case '\t':
			break;

		case 'a':
		case 'A':
			opts[oidx++] = 'A';
			asymmetric = 1;
			real = 1;
			break;

		case 'i':
		case 'I':
			opts[oidx++] = 'I';
			break;

		case 'e':
		case 'E':
			opts[oidx++] = 'E';
			break;

		case 'l':
		case 'L':
			flookup = 1;
			break;

		case 'f':
		case 'F':
			force = 1;
			break;

		case 'r':
		case 'R':
			real = 1;
			break;

		case 'n':
		case 'N':
			i++;
			s.s = str1.s+i; 
			for (s.len = 0; i<str1.len && isdigit((unsigned char)str1.s[i]);
					i++, s.len++)
				continue;
			if (s.len == 0) {
				LOG(L_ERR, "ERROR: force_rtp_proxy2: non-negative integer"
					"should follow N option\n");
				return -1;
			}
			str2int(&s, &node_idx);
			break;
			
		default:
			LOG(L_ERR, "ERROR: force_rtp_proxy2: unknown option `%c'\n", str1.s[i]);
			return -1;
		}
	}

	if (msg->first_line.type == SIP_REQUEST &&
	    msg->first_line.u.request.method_value == METHOD_INVITE) {
		create = 1;
	} else if (msg->first_line.type == SIP_REPLY) {
		create = 0;
	} else {
		return -1;
	}
	/* extract_body will also parse all the headers in the message as
	 * a side effect => don't move get_callid/get_to_tag in front of it
	 * -- andrei */
	if (extract_sdp_body(msg, &body) == -1) {
		LOG(L_ERR, "ERROR: force_rtp_proxy2: can't extract body "
		    "from the message\n");
		return -1;
	}
	if (get_callid(msg, &callid) == -1 || callid.len == 0) {
		LOG(L_ERR, "ERROR: force_rtp_proxy2: can't get Call-Id field\n");
		return -1;
	}
	if (get_to_tag(msg, &to_tag) == -1) {
		LOG(L_ERR, "ERROR: force_rtp_proxy2: can't get To tag\n");
		return -1;
	}
	if (get_from_tag(msg, &from_tag) == -1 || from_tag.len == 0) {
		LOG(L_ERR, "ERROR: force_rtp_proxy2: can't get From tag\n");
		return -1;
	}
	if (flookup != 0) {
		if (create == 0 || to_tag.len == 0)
			return -1;
		create = 0;
		tmp = from_tag;
		from_tag = to_tag;
		to_tag = tmp;
	}
	proxied = 0;
	for (cp = body.s; (len = body.s + body.len - cp) >= ANORTPPROXY_LEN;) {
		cp1 = ser_memmem(cp, ANORTPPROXY, len, ANORTPPROXY_LEN);
		if (cp1 == NULL)
			break;
		if (cp1[-1] == '\n' || cp1[-1] == '\r') {
			proxied = 1;
			break;
		}
		cp = cp1 + ANORTPPROXY_LEN;
	}
	if (proxied != 0 && force == 0)
		return -1;
	/*
	 * Parsing of SDP body.
	 * It can contain a few session descriptions (each starts with
	 * v-line), and each session may contain a few media descriptions
	 * (each starts with m-line).
	 * We have to change ports in m-lines, and also change IP addresses in
	 * c-lines which can be placed either in session header (fallback for
	 * all medias) or media description.
	 * Ports should be allocated for any media. IPs all should be changed
	 * to the same value (RTP proxy IP), so we can change all c-lines
	 * unconditionally.
	 */
	bodylimit = body.s + body.len;
	v1p = find_sdp_line(body.s, bodylimit, 'v');
	if (v1p == NULL) {
		LOG(L_ERR, "ERROR: force_rtp_proxy2: no sessions in SDP\n");
		return -1;
	}
	v2p = find_next_sdp_line(v1p, bodylimit, 'v', bodylimit);
	media_multi = (v2p != bodylimit);
	v2p = v1p;
	medianum = 0;
	for(;;) {
		/* Per-session iteration. */
		v1p = v2p;
		if (v1p == NULL || v1p >= bodylimit)
			break; /* No sessions left */
		v2p = find_next_sdp_line(v1p, bodylimit, 'v', bodylimit);
		/* v2p is text limit for session parsing. */
		m1p = find_sdp_line(v1p, v2p, 'm');
		/* Have this session media description? */
		if (m1p == NULL) {
			LOG(L_ERR, "ERROR: force_rtp_proxy2: no m= in session\n");
			return -1;
		}
		/*
		 * Find c1p only between session begin and first media.
		 * c1p will give common c= for all medias.
		 */
		c1p = find_sdp_line(v1p, m1p, 'c');
		c1p_altered = 0;
		/* Have session. Iterate media descriptions in session */
		m2p = m1p;
		for (;;) {
			m1p = m2p;
			if (m1p == NULL || m1p >= v2p)
				break;
			m2p = find_next_sdp_line(m1p, v2p, 'm', v2p);
			/* c2p will point to per-media "c=" */
			c2p = find_sdp_line(m1p, m2p, 'c');
			/* Extract address and port */
			tmpstr1.s = c2p ? c2p : c1p;
			if (tmpstr1.s == NULL) {
				/* No "c=" */
				LOG(L_ERR, "ERROR: force_rtp_proxy2: can't"
				    " find media IP in the message\n");
				return -1;
			}
			tmpstr1.len = v2p - tmpstr1.s; /* limit is session limit text */
			if (extract_mediaip(&tmpstr1, &oldip, &pf) == -1) {
				LOG(L_ERR, "ERROR: force_rtp_proxy2: can't"
				    " extract media IP from the message\n");
				return -1;
			}
			tmpstr1.s = m1p;
			tmpstr1.len = m2p - m1p;
			if (extract_mediaport(&tmpstr1, &oldport) == -1) {
				LOG(L_ERR, "ERROR: force_rtp_proxy2: can't"
				    " extract media port from the message\n");
				return -1;
			}
			++medianum;
			if (asymmetric != 0 || real != 0) {
				newip = oldip;
			} else {
				newip.s = ip_addr2a(&msg->rcv.src_ip);
				newip.len = strlen(newip.s);
			}
			/* XXX must compare address families in all addresses */
			if (pf == AF_INET6) {
				opts[oidx] = '6';
				oidx++;
			}
			snprintf(medianum_buf, sizeof medianum_buf, "%d", medianum);
			medianum_str.s = medianum_buf;
			medianum_str.len = strlen(medianum_buf);
			opts[0] = (create == 0) ? 'L' : 'U';
			v[1].iov_len = oidx;
			STR2IOVEC(callid, v[3]);
			STR2IOVEC(newip, v[5]);
			STR2IOVEC(oldport, v[7]);
			STR2IOVEC(from_tag, v[9]);
			if (1 || media_multi) /* XXX netch: can't choose now*/
			{
				STR2IOVEC(medianum_str, v[11]);
			} else {
				v[10].iov_len = v[11].iov_len = 0;
			}
			STR2IOVEC(to_tag, v[13]);
			do {
				node = select_rtpp_node(callid, 1, node_idx);
				if (!node) {
					LOG(L_ERR, "ERROR: force_rtp_proxy2: no available proxies\n");
					return -1;
				}
				cp = send_rtpp_command(node, v, (to_tag.len > 0) ? 14 : 12);
			} while (cp == NULL);
			LOG(L_DBG, "force_rtp_proxy2: proxy reply: %s\n", cp);
			/* Parse proxy reply to <argc,argv> */
			argc = 0;
			memset(argv, 0, sizeof(argv));
			cpend=cp+strlen(cp);
			next=eat_token_end(cp, cpend);
			for (ap = argv; cp<cpend; cp=next+1, next=eat_token_end(cp, cpend)){
				*next=0;
				if (*cp != '\0') {
					*ap=cp;
					argc++;
					if ((char*)++ap >= ((char*)argv+sizeof(argv)))
						break;
				}
			}
			if (argc < 1) {
				LOG(L_ERR, "force_rtp_proxy2: no reply from rtp proxy\n");
				return -1;
			}
			port = atoi(argv[0]);
			if (port <= 0 || port > 65535) {
				LOG(L_ERR, "force_rtp_proxy2: incorrect port in reply from rtp proxy\n");
				return -1;
			}

			pf1 = (argc >= 3 && argv[2][0] == '6') ? AF_INET6 : AF_INET;

			if (isnulladdr(&oldip, pf)) {
				if (pf1 == AF_INET6) {
					newip.s = "::";
					newip.len = 2;
				} else {
					newip.s = "0.0.0.0";
					newip.len = 7;
				}
			} else {
				if (argc < 2) {
					newip = str2;
				}
				else {
					newip.s = argv[1];
					newip.len = strlen(newip.s);
				}
			}
			newport.s = int2str(port, &newport.len); /* beware static buffer */
			/* Alter port. */
			body1.s = m1p;
			body1.len = bodylimit - body1.s;
			if (alter_mediaport(msg, &body1, &oldport, &newport, 0) == -1)
				return -1;
			/*
			 * Alter IP. Don't alter IP common for the session
			 * more than once.
			 */
			if (c2p != NULL || !c1p_altered) {
				body1.s = c2p ? c2p : c1p;
				body1.len = bodylimit - body1.s;
				if (alter_mediaip(msg, &body1, &oldip, pf, &newip, pf1, 0) == -1)
					return -1;
				if (!c2p)
					c1p_altered = 1;
			}
		} /* Iterate medias in session */
	} /* Iterate sessions */

	if (proxied == 0) {
		cp = pkg_malloc(ANORTPPROXY_LEN * sizeof(char));
		if (cp == NULL) {
			LOG(L_ERR, "ERROR: force_rtp_proxy2: out of memory\n");
			return -1;
		}
		anchor = anchor_lump(msg, body.s + body.len - msg->buf, 0, 0);
		if (anchor == NULL) {
			LOG(L_ERR, "ERROR: force_rtp_proxy2: anchor_lump failed\n");
			pkg_free(cp);
			return -1;
		}
		memcpy(cp, ANORTPPROXY, ANORTPPROXY_LEN);
		if (insert_new_lump_after(anchor, cp, ANORTPPROXY_LEN, 0) == NULL) {
			LOG(L_ERR, "ERROR: force_rtp_proxy2: insert_new_lump_after failed\n");
			pkg_free(cp);
			return -1;
		}
	}

	return 1;
}

static int
force_rtp_proxy1_f(struct sip_msg* msg, char* str1, char* str2)
{
	char *cp;
	fparam_t param2;
	
	char newip[IP_ADDR_MAX_STR_SIZE];

	cp = ip_addr2a(&msg->rcv.dst_ip);
	strcpy(newip, cp);
	param2.type = FPARAM_STRING;
	param2.orig = param2.v.asciiz = newip;
	return force_rtp_proxy2_f(msg, str1, (char*)&param2);
}



#define DSTIP_PARAM ";dstip="
#define DSTIP_PARAM_LEN (sizeof(DSTIP_PARAM) - 1)

#define DSTPORT_PARAM ";dstport="
#define DSTPORT_PARAM_LEN (sizeof(DSTPORT_PARAM) - 1)


/*
 * Create received SIP uri that will be either
 * passed to registrar in an AVP or apended
 * to Contact header field as a parameter
 */
static int
create_rcv_uri(str* uri, struct sip_msg* m)
{
	static char buf[MAX_URI_SIZE];
	char* p;
	str src_ip, src_port, dst_ip, dst_port;
	int len;
	str proto;

	if (!uri || !m) {
		LOG(L_ERR, "create_rcv_uri: Invalid parameter value\n");
		return -1;
	}

	src_ip.s = ip_addr2a(&m->rcv.src_ip);
	src_ip.len = strlen(src_ip.s);
	src_port.s = int2str(m->rcv.src_port, &src_port.len);

	dst_ip = m->rcv.bind_address->address_str;
	dst_port = m->rcv.bind_address->port_no_str;

	switch(m->rcv.proto) {
	case PROTO_NONE:
	case PROTO_UDP:
		proto.s = 0; /* Do not add transport parameter, UDP is default */
		proto.len = 0;
		break;

	case PROTO_TCP:
		proto.s = "TCP";
		proto.len = 3;
		break;

	case PROTO_TLS:
		proto.s = "TLS";
		proto.len = 3;
		break;

	case PROTO_SCTP:
		proto.s = "SCTP";
		proto.len = 4;
		break;

	default:
		LOG(L_ERR, "BUG: create_rcv_uri: Unknown transport protocol\n");
		return -1;
	}

	len = 4 + src_ip.len + 1 + src_port.len;
	if (proto.s) {
		len += TRANSPORT_PARAM_LEN;
		len += proto.len;
	}

	len += DSTIP_PARAM_LEN + dst_ip.len;
	len += DSTPORT_PARAM_LEN + dst_port.len;

	if (m->rcv.src_ip.af == AF_INET6) {
		len += 2;
	}

	if (len > MAX_URI_SIZE) {
		LOG(L_ERR, "create_rcv_uri: Buffer too small\n");
		return -1;
	}

	p = buf;
	/* as transport=tls is deprecated shouldnt this be sips in case of TLS? */
	memcpy(p, "sip:", 4);
	p += 4;

	if (m->rcv.src_ip.af == AF_INET6)
		*p++ = '[';
	memcpy(p, src_ip.s, src_ip.len);
	p += src_ip.len;
	if (m->rcv.src_ip.af == AF_INET6)
		*p++ = ']';

	*p++ = ':';

	memcpy(p, src_port.s, src_port.len);
	p += src_port.len;

	if (proto.s) {
		memcpy(p, TRANSPORT_PARAM, TRANSPORT_PARAM_LEN);
		p += TRANSPORT_PARAM_LEN;

		memcpy(p, proto.s, proto.len);
		p += proto.len;
	}

	memcpy(p, DSTIP_PARAM, DSTIP_PARAM_LEN);
	p += DSTIP_PARAM_LEN;
	memcpy(p, dst_ip.s, dst_ip.len);
	p += dst_ip.len;

	memcpy(p, DSTPORT_PARAM, DSTPORT_PARAM_LEN);
	p += DSTPORT_PARAM_LEN;
	memcpy(p, dst_port.s, dst_port.len);
	p += dst_port.len;

	uri->s = buf;
	uri->len = len;

	return 0;
}


/*
 * Create an AVP to be used by registrar with the source IP and port
 * of the REGISTER
 */
static int
fix_nated_register_f(struct sip_msg* msg, char* str1, char* str2)
{
	contact_t* c;
	struct lump* anchor;
	char* param;
	str uri;

	if (create_rcv_uri(&uri, msg) < 0) {
		return -1;
	}

	if (contact_iterator(&c, msg, 0) < 0) {
		return -1;
	}

	while(c) {
		param = (char*)pkg_malloc(RECEIVED_LEN + 2 + uri.len);
		if (!param) {
			ERR("No memory left\n");
			return -1;
		}
		memcpy(param, RECEIVED, RECEIVED_LEN);
		param[RECEIVED_LEN] = '\"';
		memcpy(param + RECEIVED_LEN + 1, uri.s, uri.len);
		param[RECEIVED_LEN + 1 + uri.len] = '\"';

		anchor = anchor_lump(msg, c->name.s + c->len - msg->buf, 0, 0);
		if (anchor == NULL) {
			ERR("anchor_lump failed\n");
			return -1;
		}

		if (insert_new_lump_after(anchor, param, RECEIVED_LEN + 1 + uri.len + 1, 0) == 0) {
			ERR("insert_new_lump_after failed\n");
			pkg_free(param);
			return -1;
		}

		if (contact_iterator(&c, msg, c) < 0) {
			return -1;
		}
	}

	return 1;
}

static int
fixup_ping_contact(void **param, int param_no)
{
	int ret;

	if (param_no == 1) {
		ret = fix_param(FPARAM_AVP, param);
		if (ret <= 0) return ret;
		return fix_param(FPARAM_STR, param);
	}
	return 0;
}

static int
ping_contact_f(struct sip_msg *msg, char *str1, char *str2)
{
	struct dest_info dst;
	str s;
	avp_t *avp;
	avp_value_t val;

	switch (((fparam_t *)str1)->type) {
		case FPARAM_AVP:
			if (!(avp = search_first_avp(((fparam_t *)str1)->v.avp.flags,
			    ((fparam_t *)str1)->v.avp.name, &val, 0))) {
				return -1;
			} else {
				if ((avp->flags & AVP_VAL_STR) == 0)
					return -1;
				s = val.s;
			}
			break;
		case FPARAM_STRING:
			s = ((fparam_t *)str1)->v.str;
			break;
	        default:
        	        ERR("BUG: Invalid parameter value in ping_contact\n");
                	return -1;
        }
	init_dest_info(&dst);
	return natping_contact(s, &dst);
}
