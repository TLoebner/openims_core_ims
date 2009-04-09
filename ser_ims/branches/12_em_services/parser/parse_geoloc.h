/**
 * \file geoloc.h
 * 
 * used by the Emergency-CSCF -Geolocation header parsing
 * (conforming to the draft-ietf-sip-location-conveyance)
 * 
 *  \author Ancuta Onofrei	ancuta_onofrei	at yahoo dot com
 */

#ifndef PARSE_GEOLOC_H
#define PARSE_GEOLOC_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "../mem/mem.h"
#include "../dprint.h"
#include "../ut.h"
#include "../str.h"
#include "hf.h"
#include "msg_parser.h"

struct loc_value{
	struct sip_uri locURI;
	//used to send error responses
	str inserted_by; //mandatory
	//used by the receiving PSAP to know according to which location, the call was routed
	int used_for_routing; //0 for no and 1 for yes, 
	struct loc_value * next;
};

struct geoloc_body{
	//implemented as a stack, the first is the last in the body content
	struct loc_value * loc_list;
	int retrans_par; //0 for no and 1 for yes, default no/0
};

int parse_geoloc(struct sip_msg * msg);
void free_geoloc(struct geoloc_body **);
void print_geoloc(struct geoloc_body *);

#endif
