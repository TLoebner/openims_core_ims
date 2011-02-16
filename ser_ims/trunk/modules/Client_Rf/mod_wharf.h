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

#ifndef _Client_Rf__H
#define _Client_Rf__H

#include "../../base/mod.h"


int mod_init(str config);
int mod_child_init(int rank);
void mod_destroy();
void* mod_get_bind();



#endif

#endif /* WHARF */
