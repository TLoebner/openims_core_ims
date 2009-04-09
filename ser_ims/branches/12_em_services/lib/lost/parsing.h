#ifndef LIB_LOST_PARSING_H
#define LIB_LOST_PARSING_H

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/HTMLparser.h>

#include "../../str.h"
#include "../../parser/msg_parser.h"

typedef enum {LOST_OK=0, LOST_WRNG, LOST_ERR, LOST_REDIR} lost_resp_type;
typedef enum {EXP_NO_CACHE, EXP_NO_EXP, EXP_TIME} expire_type;

#define name_compar(str1, str2, len)\
	((str1[len] == '\0') && (strncmp(str1, str2, len)==0))

//parse a string using the libxml2 library
xmlNode * xml_parse_string(str response);
//parse a LoST response and set the type (OK, ERR, WARN) and the reason, if the case
xmlNode* get_LoST_resp_type(str response, lost_resp_type* resp_type, str* reason);
//get the URI from a LoST response, the expiration information and the parsed URI
str get_mapped_psap(xmlNode* root, expire_type *, time_t*, struct sip_uri *);

//get the first node child node of a node
xmlNode* child_node( xmlNode *);
//get the child node with a name
xmlNode * child_named_node(xmlNode * a_node, char* name);
//get the child node with a name and name lenght
xmlNode * child_node_name_len(xmlNode * a_node, char* name, int len);

//get the sibling node with a name
xmlNode * sibling_named_node(xmlNode * a_node, char* name);
//get the sibling node with a name and a name length
xmlNode * sibling_node_name_len(xmlNode * a_node, char* name, int len);

//get a attribute with a name [of a node]
xmlAttr * get_attr(xmlNode * node, char* attr_name);
//get a attribute with a name and name length[of a node]
xmlAttr * get_attr_name_len(xmlNode * node, char* attr_name, int attr_name_len);

//get a namespace with a certain prefix [of a node]
xmlNs* get_ns_prfx_len(xmlNode * node, char* prfx, int prfx_len);
//get a namespace with a certain href [of a node]
xmlNs* get_ns_href_len(xmlNode* node, char* href, int href_len);

//printing functions
void print_attr(xmlNode * node, char * attr_name);
void print_element_names(xmlNode * a_node);

#endif
