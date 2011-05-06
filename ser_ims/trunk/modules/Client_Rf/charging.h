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
 * Wharf CDF_Rf module -  
 * 
 * 
 *  \author Andreea Onofrei Corici andreea dot ancuta dot corici -at- fokus dot fraunhofer dot de
 * 
 */
 
#ifndef _Client_Rf_CHARGING_H
#define _Client_Rf_CHARGING_H

#ifdef WHARF
#include "../../utils/utils.h"
#endif /*WHARF*/

#include "../cdp/diameter.h"
#include "Rf_data.h"

int Rf_add_chg_info(str sip_uri, str an_charg_id);

int get_chg_info(str sip_uri, str * an_charg_id);

#endif
