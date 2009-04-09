#ifndef ECSCF_LRF_H
#define ECSCF_LRF_H

#include "../../str.h"
#include "../tm/h_table.h"

struct initial_tr {
	/* tells in which hash table entry the initial transaction lives */
	unsigned int  hash_index;
	/* sequence number within hash collision slot */
	unsigned int  label;
	/* callid of the initial trans*/
	str callid;
};

int E_query_LRF(struct sip_msg* msg, char* str1, char* str2);

int E_get_location(struct sip_msg* msg, char* str1, char* str2);

int E_del_ESQK_info(struct sip_msg * inv_repl, char* str1, char* str2);

#endif
