/*
 * $Id$
 *
 * Copyright (C) 2009 Fhg Fokus
 *
 */
/**
 * \file
 * 
 * CDF module for Wharf - Module configuration
 * 
 * 
 *  \author Andreea Ancuta Onofrei Corici andreea dot ancuta dot corici -at- fokus dot fraunhofer dot de
 * 
 */  

#ifdef WHARF

#include "../../utils/utils.h"
 
#include "config.h"

extern client_rf_cfg cfg;

/**
 * Parse a MobileIP module configuration from a configuration XML
 * @param config
 * @return
 */
int client_rf_parse_config(str config)
{
	xmlDocPtr doc=0;
	xmlDtdPtr dtd=0;
	xmlNodePtr root=0,i=0;
	xmlChar *xc=0;
	long int l;
	str config_dtd;
	config_dtd.s=CLIENT_RF_CONFIG_DTD;
	config_dtd.len = strlen(config_dtd.s);
	
	doc = xml_get(config);
	if (!doc){
		LOG(L_ERR,"  The configuration does not contain a valid XML\n");
		goto error;
	}
	
	dtd = xml_get_dtd(config_dtd);
	if (!dtd){
		LOG(L_ERR,"  The DTD is not valid\n");
		goto error;
	}
	
	if (xml_validate_dtd(doc,dtd)!=1){
		LOG(L_ERR,"  Verification of configuration XML against DTD failed!\n");
		goto error;
	}
	
	root = xmlDocGetRootElement(doc);
	if (!root){
		LOG(L_ERR,"  Empty XML\n");
		goto error;
	}
	
	while(root && (root->type!=XML_ELEMENT_NODE || strcasecmp((char*)root->name,"Client_Rf")!=0)){
		root = root->next;
	}
	
	if (!root) {
		LOG(L_ERR," No <Client_Rf> found in configuration\n");
		goto error;
	}

	for(i=root->children;i;i=i->next)
		if (i->type==XML_ELEMENT_NODE) {

			/* Rf */
			if (strcasecmp((char*)i->name,"Rf")==0){
				
				if (!xml_get_prop_as_int(i,"node_role",&l)||l<0 || l>10){
					LOG(L_ERR,"node_role value between 0 and 10\n");
					goto error;
				}else
					cfg.node_func = (int)l;				
			}
		}
	
	if (doc) xml_free(doc);	
	if (dtd) xml_free_dtd(dtd);
	return 1;
error:
	if (xc) xmlFree(xc);
	if (doc) xml_free(doc);
	if (dtd) xml_free_dtd(dtd);
	return 0;
	
}

#endif /* WHARF */

