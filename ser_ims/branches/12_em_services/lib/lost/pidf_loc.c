
#include "client.h"
#include "pidf_loc.h"
#include "parsing.h"

//the root of the PIDF-LO is a node with the name "presence", see RFC 4119
static xmlNode* get_loc_info_kids(xmlNode* presence){

	xmlNode * res, *child;
	
	res = NULL;

	if(!presence){
	
		ERROR_LOG("null argument\n");
		return res;
	}
	//search for the first device element
	child = child_node_name_len(presence, PIDF_DEV, PIDF_DEV_LEN);
	//search for the first tuple element
	if(!child){
		child = child_node_name_len(presence, PIDF_TUPL, PIDF_TUPL_LEN);
		//search for the status element from a tuple
		if(child){
			child = child_node_name_len(child, PIDF_STATUS, PIDF_STATUS_LEN);
			if(!child){
				ERROR_LOG("malformed request: tuple node has no status child\n");
				return res;	
			}
		}
	}
	//search for the first person element
	if(!child){
		child = child_node_name_len(presence, PIDF_PERS, PIDF_PERS_LEN);
		if(!child){
			ERROR_LOG("malformed request: no device, tuple, or person element\n");
			return res;
		}

	}

	DEBUG("found a location node: type %s\n", (char*)child->name);

	child = child_node_name_len(child, PIDF_GEOPRIV, PIDF_GEOPRIV_LEN);
	if(!child){
		ERROR_LOG("malformed request: no geopriv child element\n");
		return res;
	}

	res = child_node_name_len(child, PIDF_INFO, PIDF_INFO_LEN);
	if(!res){
		ERROR_LOG("malformed request: geopiv element has no location-info child element\n");
		return res;
	}
	if(!res->children){
	
		ERROR_LOG("location-info node has no children\n");
	}

	return res;

}
//like in RFC 4119
xmlNode * is_geo_coord_fmt(xmlNode * locNode, loc_fmt * crt_loc_fmt){

	xmlNode * node;
	
	DEBUG("checking if it has geo_coord format, like in RFC 4119\n");
	node = child_node_name_len(locNode, PIDF_GML_LOC, PIDF_GML_LOC_LEN);
	if(!node)
		goto end;

	DEBUG("found a %.*s child, might have geo coord format, RFC 4119\n",
			PIDF_GML_LOC_LEN, PIDF_GML_LOC);
	if(!(node = child_node_name_len(node,  PIDF_POINT_SHAPE, PIDF_POINT_SHAPE_LEN))){
		ERROR_LOG("the location node has no child");
	        *crt_loc_fmt = ERR_LOC;
		goto end;
	}
	DEBUG("found child node Point, geo coord format present\n");
	*crt_loc_fmt = GEO_COORD_LOC;
end:
	return node;
}


xmlNode * is_geo_shape_fmt(xmlNode* locNode, loc_fmt * crt_loc_fmt){

	xmlNode * node;
	char* name;

	DEBUG("checking if it has geo_shape format\n");
	node = child_node(locNode);
	if(!node){
		ERROR_LOG("no shape node child found\n");
		*crt_loc_fmt = ERR_LOC;
		goto end;
	}
	name = (char*)node->name;
	if(name_compar(name, PIDF_POINT_SHAPE, PIDF_POINT_SHAPE_LEN) || 
			name_compar(name, PIDF_POLYGON_SHAPE, PIDF_POINT_SHAPE_LEN) ||
			name_compar(name, PIDF_CIRCLE_SHAPE, PIDF_CIRCLE_SHAPE_LEN) ||
			name_compar(name, PIDF_ELLIPSE_SHAPE, PIDF_ELLIPSE_SHAPE_LEN) ||
			name_compar(name, PIDF_ARC_BAND_SHAPE, PIDF_ARC_BAND_SHAPE_LEN)){
		
		*crt_loc_fmt = GEO_SHAPE_LOC;
	}else if(name_compar(name, PIDF_SPHERE_SHAPE, PIDF_SPHERE_SHAPE_LEN) ||
			name_compar(name, PIDF_ELLIPSOID_SHAPE, PIDF_ELLIPSOID_SHAPE_LEN) ||
			name_compar(name, PIDF_PRISM_SHAPE, PIDF_PRISM_SHAPE_LEN)){
		
		*crt_loc_fmt = GEO_NEW_SHAPE_LOC;
	}else{
		ERROR_LOG("no valid geo shape found\n");
		*crt_loc_fmt = ERR_LOC;
		node = NULL;
	}	
	
end:
	return node;
}

xmlNode * is_RFC4119_civic_fmt(xmlNode * locNode, loc_fmt * crt_loc_fmt){

	xmlNode * node;

	DEBUG("checking if it has RFC 4119 civic format\n");

	node = child_node_name_len(locNode, PIDF_CIV_LOC, PIDF_CIV_LOC_LEN);
	if(node){
		DEBUG("found a %.*s child, has RFC 4119 civic format\n",
				PIDF_CIV_LOC_LEN, PIDF_CIV_LOC);
		*crt_loc_fmt = OLD_CIV_LOC;
	}else{
		ERROR_LOG("did not find a %.*s child, no civic fromat\n",
				PIDF_CIV_LOC_LEN, PIDF_CIV_LOC);
	       	*crt_loc_fmt = ERR_LOC;
	       	node = NULL;
	}

	return node;
}

//if firt child is location, then geodetic-2D type
//if the node is civicAddress, then type civic type 
//if there is a namespace with href PIDF_OLD_CIV_NS_HREF=>old type of civic address
//if there is a namespace with href PIDF_NEW_CIV_NS_HREF=>new type of civic address
xmlNode* is_new_civic_fmt(xmlNode * locNode, loc_fmt * crt_loc_fmt){

	xmlNode * node;
	
	DEBUG("checking if it has new civic format\n");

	node = child_node_name_len(locNode, PIDF_CIV_LOC, PIDF_CIV_LOC_LEN);
	if(node){
		DEBUG("found a %.*s child, has new civic format\n",
				PIDF_CIV_LOC_LEN, PIDF_CIV_LOC);
		*crt_loc_fmt = NEW_CIV_LOC;
	}else{
		ERROR_LOG("did not find a %.*s child, no civic fromat\n",
				PIDF_CIV_LOC_LEN, PIDF_CIV_LOC);
	       	*crt_loc_fmt = ERR_LOC;
	       	node = NULL;
	}

	return node;
}

/* Checks if the xml tree corresponds to a PIDF-LO
 * @param presence is the root node, for the presence element node
 * @param loc - the address of the xmlNode pointing to location information if ok, otherwise NULL
 * @param crt_loc_fmt - the type of location format: geodetic-2D, civic new or old, or other
 * @returns 0 if ok, 1 if malformed and 2 if not a location object is present, 3 if BUG
 */
xmlNode* has_loc_info(int * code, xmlNode* presence, loc_fmt * crt_loc_fmt){

	xmlNs * ns;
	char * content;
	xmlNode * locNode,* res;

	res = NULL;

	if(!presence){
		ERROR_LOG("NULL presence object!!!\n");
		*code = 3;
		goto err;
	}
	
	ns = get_ns_prfx_len(presence, PIDF_GEO_NS_PRFX, PIDF_GEO_NS_PRFX_LEN);
	if(!ns){
	
		DEBUG("the application/pidf+xml presence node has no xmlns:%.*s namespace, ignoring it\n",
				PIDF_GEO_NS_PRFX_LEN, PIDF_GEO_NS_PRFX);
		*code = 2;
		goto err;
	}
	
	if(ns->href == NULL){

		ERROR_LOG("Malformed PIDF_LO : presence node with no value for the namespace xmlns:%.*s\n",
				PIDF_GEO_NS_PRFX_LEN, PIDF_GEO_NS_PRFX);
		*code = 1;
		goto err;
	}
	
	content = (char*)ns->href;
	if(!(name_compar(content, PIDF_GEO_NS_HREF, PIDF_GEO_NS_HREF_LEN))){
		ERROR_LOG("namespace xmlns:%.*s of the presence node has not the expected value %.*s\n",
				PIDF_GEO_NS_PRFX_LEN, PIDF_GEO_NS_PRFX,
				PIDF_GEO_NS_PRFX_LEN, PIDF_GEO_NS_HREF);
		*code = 2;
		goto err;
	}
	
	locNode = get_loc_info_kids(presence);
	if(!locNode){
		*code = 1;
		goto err;
	}

	//check if the prefix ca is present in a xmlnamespace
	ns = get_ns_href_len(locNode, PIDF_NEW_CIV_NS_HREF, PIDF_NEW_CIV_NS_HREF_LEN);
	if(ns){
		res = is_new_civic_fmt(locNode, crt_loc_fmt);
		if(res)
			return res;
	}

	//check if the prefix gs is present in a xmlnamespace
	ns = get_ns_href_len(locNode, PIDF_GEO_SHAPE_NS_HREF, PIDF_GEO_SHAPE_NS_HREF_LEN);
	if(ns){
		res = is_geo_shape_fmt(locNode, crt_loc_fmt);
		if(res)
			return res;
	}

	//check if the prefix gml is present in a xmlnamespace
	ns = get_ns_href_len(locNode, PIDF_GEO_COORD_NS_HREF, PIDF_GEO_COORD_NS_HREF_LEN);
	if(ns){
		res = is_geo_coord_fmt(locNode, crt_loc_fmt);
		if(res)
			return res;
	}

	
	//check if the prefix cl is present in a xmlnamespace
	ns = get_ns_href_len(locNode, PIDF_OLD_CIV_NS_HREF, PIDF_OLD_CIV_NS_HREF_LEN);
	if(ns){
		res = is_RFC4119_civic_fmt(locNode, crt_loc_fmt);
		if(res)
			return res;
	}

	ERROR_LOG("no valid xml namespace with the valid href values  %.*s, %.*s, %.*s or %.*s, or the right format\n",
			PIDF_NEW_CIV_NS_HREF_LEN, PIDF_NEW_CIV_NS_HREF,
			PIDF_GEO_SHAPE_NS_HREF_LEN, PIDF_GEO_SHAPE_NS_HREF,
			PIDF_GEO_COORD_NS_HREF_LEN, PIDF_GEO_COORD_NS_HREF,
			PIDF_OLD_CIV_NS_HREF_LEN, PIDF_OLD_CIV_NS_HREF);
err:
	return res;
}
