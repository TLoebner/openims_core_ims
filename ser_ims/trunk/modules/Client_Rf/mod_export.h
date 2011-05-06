/*
 * $Id$
 * 
 * Copyright (C) 2009 FhG Fokus
 * 
 * This file is part of the Wharf project.
 * 
 */

/**
 * \file
 * 
 * Client_Rf Wharf module interface
 * 
 * 
 *  \author Andreea Ancuta Corici andreea dot ancuta dot corici -at- fokus dot fraunhofer dot de
 * 
 */

#ifdef WHARF

#ifndef _Client_Rf_EXPORT__H
#define _Client_Rf_EXPORT__H

#include "../cdp/session.h"
#include "Rf_data.h"

typedef int (*AAASendAccRequest_f)(AAASession *session, Rf_ACR_t * rf_data);
typedef int (*Rf_add_chg_info_f)(str sip_uri, str an_charg_id);


struct client_rf_binds{
	AAASendAccRequest_f	AAASendACR;
	Rf_add_chg_info_f	Rf_add_chg_info;
};

#endif

#endif /* WHARF */
