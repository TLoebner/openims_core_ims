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
 * Diameter Rf interface towards the CDF - Wharf module interface
 * 
 * 
 *  \author Andreea Ancuta Corici andreea dot ancuta dot corici -at- fokus dot fraunhofer dot de
 * 
 */

#include "Rf_data.h"
#include "config.h"
#include "diameter_rf.h"
#include "acr.h"

#ifdef WHARF
#define M_NAME "Client_Rf"
#endif

#include "charging.h"

int Rf_add_chg_info(str sip_uri, str an_charg_id){

	return 1;
}

str get_charg_info(str sip_uri){

	str res = {0,0};

	return res;
}
