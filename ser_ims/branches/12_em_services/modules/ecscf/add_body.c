
#include "mod.h"

#include "../../data_lump.h"
#include "../../ut.h"
#include "../../parser/parser_f.h"
#include "../mem/mem.h"
#include "sdp_helpr_funcs.h"
#include "parse_content.h"
#include "multipart_parse.h"
#include "sip.h"

typedef enum {NO_CONT=0, SINGLE_CONT, MULTI_CONT} cont_type_e;

static int add_body_multi_content(struct sip_msg* msg, str body, str content, str boundary);
static int add_body_single_content(struct sip_msg* msg, str body, str content);
static int add_body_no_content(struct sip_msg* msg, str body, str content);

int add_body_part(struct sip_msg * msg, str body, str content);
int add_loc_info_body_part(struct sip_msg * msg, str loc_info);
struct lump * add_before_body(struct sip_msg * msg, str string);
str get_headers_single_body_part(struct sip_msg * msg);
struct lump * add_after_body(struct sip_msg * msg, str string);
str create_string4_added_body(str body, str content);

int parse_multipart_body(struct sip_msg * _m, cont_type_e * cnt_type, str * boundary){
	
	str body;
	int mime;
	
	body.s = get_body(_m);
	if (body.s==0) {
		LOG(L_ERR, "ERR:"M_NAME":parse_multipart_body:failed to get the message body\n");
		return -1;
	}

	body.len = _m->len -(int)(body.s - _m->buf);
	if (body.len==0) {
		LOG(L_DBG, "DBG:"M_NAME":parse_multipart_body:message body has length zero\n");
		*cnt_type = NO_CONT;
		return 0;
	}

	mime = ecscf_parse_content_type_hdr(_m);
	if (mime <= 0) {
		LOG(L_ERR, "DBG:"M_NAME":parse_multipart_body:could not parse the type of body\n");
		return -1;
	}
	if( (((unsigned int)mime)>>16) != TYPE_MULTIPART){
		*cnt_type = SINGLE_CONT;
		return 0;
	}

	if(get_mixed_part_delimiter(&(_m->content_type->body), boundary) > 0) {
		LOG(L_DBG, "DBG:"M_NAME":parse_multipart_body: boundary is %.*s\n", 
					boundary->len, boundary->s);
		*cnt_type = MULTI_CONT;
	} else {
		LOG(L_ERR, "ERR:"M_NAME":parse_multipart_body:could not get the delimiter of the multipart content\n");
		return -1;
	}
	
	return 0;
}



int add_loc_info_body_part(struct sip_msg * msg, str loc_info){
	
	str pidf_lo_cnt = {"application/pidf+xml", 20};
	return add_body_part(msg, loc_info, pidf_lo_cnt);
}

int add_body_part(struct sip_msg * msg, str body, str content){

	cont_type_e cnt_type;
	str boundary = {NULL, 0};

	if(parse_multipart_body(msg, &cnt_type, &boundary) < 0)
		goto error;

	switch(cnt_type){
		case MULTI_CONT: add_body_multi_content(msg, body, content, boundary);
				 break;
		case NO_CONT:   add_body_no_content(msg, body, content);
				break;
		case SINGLE_CONT: add_body_single_content(msg, body, content);
				  break;
	}
	
	LOG(L_DBG,"DBG:"M_NAME":add_body_part: successfully added the body %.*s\n",
			body.len, body.s);
	return 0;
error:
	LOG(L_ERR,"ERR:"M_NAME":add_body_part: could not add the body %.*s\n",
			body.len, body.s);

	return -1;
}

static int add_body_multi_content(struct sip_msg* msg, str body, str content, str boundary){
	return 0;
}
static str middle_default_boundary = {"\r\n\n--abc\r\n",10};
static str s_end = {"\r\n",2};
static str end_default_boundary={"\r\n--abc--\r\n", 13};
static str final_def_cnt_type = {"multipart/mixed; boundary=\"abc\"", 31};

static int add_body_single_content(struct sip_msg* msg, str body, str content){

	struct lump * crt;
	str init_headers = {NULL, 0};
	str first_midd_def_boundary = {NULL,0};
	str final_cnt_type = {NULL,0};

	LOG(L_DBG, "DBG:"M_NAME":add_body_single_content: adding body %.*s with content %.*s to the message\n",
			body.len, body.s, content.len, content.s);

	init_headers = get_headers_single_body_part(msg);
	if(!init_headers.s || !init_headers.len){
		LOG(L_ERR, "ERR:"M_NAME":add_body_single_content: could not create the headers for the initial body part\n");
		return -1;
	}
	
	int init_cnt_length = cscf_get_content_len(msg);
	if(init_cnt_length == 0){
		LOG(L_ERR, "ERR:"M_NAME":add_body_single_content: could not retrieve the content length\n");
		return -1;
	}

	str init_cnt_type = msg->content_type->body;
	STR_PKG_DUP(final_cnt_type, final_def_cnt_type, "add_body_single_content");
	if(cscf_replace_string(msg, init_cnt_type, final_cnt_type) == 0){
		LOG(L_ERR, "ERR:"M_NAME":add_body_single_content: could not replace the content type\n");
		return -1;
	}
	
	STR_PKG_DUP(first_midd_def_boundary, middle_default_boundary, "add_body_single_content");
	//add boundary
	if(!(crt = add_before_body(msg, first_midd_def_boundary)) ){
		LOG(L_ERR, "ERR:"M_NAME":add_body_single_content: could not add the first middle boundary\n");
		return -1;
	}	
	//add headers of the initial body part
	if(!(crt =insert_new_lump_after(crt, init_headers.s, init_headers.len, HDR_EOH_T)) ){
		LOG(L_ERR, "ERR:"M_NAME":add_body_single_content: could not add the headers of the initial body\n");
		return -1;
	}
	
	str rest = create_string4_added_body(body, content);
	if(!rest.len || !rest.s){
		LOG(L_ERR, "ERR:"M_NAME":add_body_single_content: error creating the rest of the multipart message");
		return -1; 
	}

	if(!(crt = add_after_body(msg, rest)) ){
		LOG(L_ERR, "ERR:"M_NAME":add_body_single_content: could not add the second middle boundary\n");
		return -1;
	}	
	return 0;
out_of_memory:
	return -1;
}
static int add_body_no_content(struct sip_msg* msg, str body, str content){
	return 0;
}

struct lump * add_before_body(struct sip_msg * msg, str string){

	int offset, len;
	struct lump * res = NULL;

	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":add_before_body: error parsing headers\n");
		return res;
	}

	len = 0;
	char * c = get_body(msg);
	if(!c){
		LOG(L_ERR, "ERR:"M_NAME":add_before_body: error getting body\n");
		return res;
	}
	offset = (int)(c-msg->buf);
	struct lump* anchor = anchor_lump(msg, offset, len, HDR_EOH_T);
	if(!anchor){
		LOG(L_ERR, "ERR:"M_NAME":add_before_body: error creating anchor\n" );
		return res;
	}

	if(!(res =insert_new_lump_after(anchor, string.s, string.len, HDR_EOH_T)) ){
		LOG(L_ERR, "ERR:"M_NAME":add_before_body: error creating lump\n" );
	}	
	return res;
}

struct lump * add_after_body(struct sip_msg * msg, str string){

	int offset, len;
	struct lump * res = NULL;

	if (parse_headers(msg, HDR_EOH_F, 0)<0){
		LOG(L_ERR,"ERR:"M_NAME":add_after_body: error parsing headers\n");
		return res;
	}

	len = 0;
	offset = msg->len;
	struct lump* anchor = anchor_lump(msg, offset, len, HDR_EOH_T);
	if(!anchor){
		LOG(L_ERR, "ERR:"M_NAME":add_after_body: error creating anchor\n" );
		return res;
	}

	if(!(res =insert_new_lump_after(anchor, string.s, string.len, HDR_EOH_T)) ){
		LOG(L_ERR, "ERR:"M_NAME":add_after_body: error creating lump\n" );
	}	
	return res;
}



static str content_type_s = {"Content-Type: ", 14};
static str content_length_s = {"Content-Length: ", 16};
str get_headers_single_body_part(struct sip_msg * msg){

	str headers= {NULL, 0};
	str content_type = {NULL, 0}, content_len={NULL, 0};

	content_type = cscf_get_content_type(msg);
	if(!content_type.s || !content_type.len){
		LOG(L_ERR, "ERR:"M_NAME":get_headers_single_body_part: invalid content-type header\n");
		return headers;
	}

	int length = cscf_get_content_len(msg);
	if(length == 0){
		LOG(L_ERR, "ERR:"M_NAME":get_headers_single_body_part: invalid content-len header\n");
		return headers;
	}
	content_len = msg->content_length->body;
	
	int len = content_type_s.len + content_type.len + 
		content_length_s.len + content_len.len + s_end.len*2;
	headers.s = (char*)pkg_malloc(len*sizeof(char));
	if(!headers.s)
		return headers;
	headers.len = 0;
	STR_APPEND(headers, content_type_s);
	STR_APPEND(headers, content_type);
	STR_APPEND(headers, s_end);
	STR_APPEND(headers, content_length_s);
	STR_APPEND(headers, content_len);
	STR_APPEND(headers, s_end);
	
	LOG(L_DBG, "DBG:"M_NAME":get_headers_single_body_part: the headers are: %.*s\n", headers.len, headers.s);

	return headers;
}

//add midd boundary + headers of the second body part + second body part + end boundary
str create_string4_added_body(str body, str content){

	str res = {NULL, 0};
	str cont_len = {NULL, 0};
	/* returns a pointer to a static buffer containing l in asciiz & sets len */
	cont_len.s = int2str((unsigned int)body.len, &cont_len.len);
	if(!cont_len.s || !cont_len.len){
		LOG(L_ERR, "ERR:"M_NAME":create_string4_added_body: error while "
				"converting the body len into string");
		return res;
	}
	int len = middle_default_boundary.len+ end_default_boundary.len +
		content_type_s.len + content.len + 
		content_length_s.len + cont_len.len + s_end.len*4 + 
		body.len;
	res.s = (char*)pkg_malloc(len*sizeof(char));
	if(!res.s)
		return res;
	res.len = 0;
	
	STR_APPEND(res, middle_default_boundary);
	STR_APPEND(res, content_type_s);
	STR_APPEND(res, content);
	STR_APPEND(res, s_end);
	STR_APPEND(res, content_length_s);
	STR_APPEND(res, cont_len);
	STR_APPEND(res, s_end);
	STR_APPEND(res, s_end);
	STR_APPEND(res, body);
	STR_APPEND(res, s_end);
	STR_APPEND(res, end_default_boundary);

	LOG(L_DBG, "DBG:"M_NAME":create_string4_added_body: res is %.*s\n",
			res.len, res.s);
	return res;
}
