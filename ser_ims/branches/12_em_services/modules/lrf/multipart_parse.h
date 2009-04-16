#ifndef MOD_MULTIPART_BODY_PARSE
#define MOD_MULTIPART_BODY_PARSE

int get_pidf_lo_body(struct sip_msg* _m, str * pidf_body);
int get_mixed_body_content(str* mixed_body, str delimiter, unsigned int type, unsigned int subtype, 
	str * body_content);
int get_body_content(struct sip_msg * msg, str * body_content, unsigned int type, unsigned int subtype);

#endif
