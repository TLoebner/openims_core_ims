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

#ifdef WHARF

#include "stdlib.h"

#include "../../base/mod.h"

#include "../../base/mod_ops.h"

#include "../../base/fork.h"

#include "../../utils/utils.h"

#include "mod_wharf.h"
#include "mod_export.h"
#include "diameter_rf.h"

/** This variable has to be called exactly mod_exports, as it is searched by the wharf base */
wharf_mod_export_t mod_exports={
		
		"Client_Rf",					/**< Module's unique name */
		
		WHARF_VERSION"-"WHARF_REVISION,		
		
		client_rf_init,				/**< Module init function */
		client_rf_child_init,			/**< Module child init function */
		client_rf_destroy,			/**< Module destroy function */
		
		client_rf_get_bind			/**< Module get binding function */
		
};	


/** 
 * Module init function.
 * 
 * - Initializes the Rf module using the provided configuration file.
 * - Registers with pt the required number of processes.
 */
int client_rf_init(str config_data)
{
	LOG(L_INFO," Client_Rf initializing\n");
	return 1;
}

/**
 * Module initialization function - called once for every process.
 * \note All modules have by now executed the mod_init.
 * If this returns failure, wharf will exit
 * 
 * @param rank - rank of the process calling this
 * @return 1 on success or 0 on failure
 */

int client_rf_child_init(int rank)
{
	if (rank == WHARF_PROCESS_ATTENDANT) { 
		LOG(L_INFO,"Client_Rf starting ...\n");		
		LOG(L_INFO," ... Client_Rf started\n");		
	}
	return 1;
}

/**
 * Module destroy function. 
 * Should clean-up and do nice shut-down.
 * \note Will be called multiple times, once from each process, although crashed processes might not.
 */
void client_rf_destroy(int rank)
{
}



struct client_rf_binds client_rf_binding={
	AAASendACR
};


/**
 * Returns the module's binding. This will give the structure containing the 
 * functions and data to be used from other processes.
 * @return the pointer to the binding.
 */
void* client_rf_get_bind()
{
	return &client_rf_binding;
}



#endif /* WHARF */
