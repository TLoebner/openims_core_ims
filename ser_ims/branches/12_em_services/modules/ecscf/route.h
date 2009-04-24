#ifndef ECSCF_ROUTE_H
#define ECSCF_ROUTE_H

int E_fwd_to_psap(struct sip_msg * msg, str psap_uri);

int E_add_esqk(struct sip_msg * msg, str esqk);

int E_add_record_route(struct sip_msg* msg, char* str1, char* str2);
#endif
