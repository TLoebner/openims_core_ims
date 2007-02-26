/*
 * $id$ server.h $date $author$ Dragos Vingarzan dvi vingarzan@gmail.com
 *
 * Copyright (C) 2005 Fhg Fokus
 *
 */

#ifndef __SERVER_H
#define __SERVER_H

#include "cdp/peer.h"
#include "cdp/diameter.h"
#include "cdp/diameter_ims.h"


AAAMessage *send_unknown_request_answer(AAAMessage *req);

int process_incoming(peer *p,AAAMessage *msg,void* ptr);

#endif
