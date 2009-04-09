#ifndef LIB_LOST_CLIENT_H
#define LIB_LOST_CLIENT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define SER	1
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

//#include <libxml/tree.h>
//#include <libxml/parser.h>
//#include <libxml/HTMLparser.h>

//#include <cds/memory.h>
#include <cds/logger.h>
#include <cds/list.h>

#include "../../version.h"
#include "../../str.h"
#include "../../mem/mem.h"
#include "../../mem/shm_mem.h"
#include "../../parser/msg_parser.h"
#include "../../parser/parse_uri.h"
#include "pidf_loc.h"


#define LOST_CONTENT_TYPE		"Content-Type: application/lost+xml"
#define LOST_CACHE_CONTROL		"Cache-Control: no-cache"
//#define LOST_DEBUG			0
#define LOST_ERR_NODE_NAME		"errors"
#define LOST_REDIR_NODE_NAME		"redirect"
#define LOST_WRNG_NODE_NAME		"warnings"
#define LOST_MAPPING_NODE_NAME		"mapping"
#define LOST_URI_NODE_NAME		"uri"
#define LOST_FINDSRESP_NODE_NAME	"findServiceResponse"

#define LOST_MSG_ATTR_NAME		"message"
#define LOST_TGT_ATTR_NAME		"target"
#define LOST_EXPIRES_ATTR_NAME		"expires"

#define LOST_EXP_NO_CACHE		"NO-CACHE"
#define LOST_EXP_NO_EXPIRATION		"NO-EXPIRATION"

#define LOST_NS_HREF			"urn:ietf:params:xml:ns:lost1"
#define LOST_XML_ENC			"UTF-8"
#define LOST_FIND_SERVICE_CMD		"findService"
#define LOST_LOCATION_NODE		"location"
#define LOST_SERVICE_NODE		"service"

#define LOST_ID_PROP			"id"
#define LOST_PROFILE_PROP		"profile"

int init_lost_lib();
void end_lost_lib();

CURL* lost_http_conn(char *url, int port, str* chunk);
int create_lost_req(xmlNode* location, loc_fmt d_loc_fmt, str* lost_req);
int send_POST_data(CURL* connhandle, str data);
void lost_http_disconn(CURL* connhandle);

#endif
