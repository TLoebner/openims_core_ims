/**
 * $Id: pcc.c,v 1.10 2007/03/14 16:18:28 Alberto Exp $
 *   
 * Copyright (C) 2004-2007 FhG Fokus
 *
 * This file is part of Open IMS Core - an open source IMS CSCFs & HSS
 * implementation
 *
 * Open IMS Core is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * For a license to use the Open IMS Core software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact Fraunhofer FOKUS by e-mail at the following
 * addresses:
 *     info@open-ims.org
 *
 * Open IMS Core is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * It has to be noted that this Open Source IMS Core System is not 
 * intended to become or act as a product in a commercial context! Its 
 * sole purpose is to provide an IMS core reference implementation for 
 * IMS technology testing and IMS application prototyping for research 
 * purposes, typically performed in IMS test-beds.
 * 
 * Users of the Open Source IMS Core System have to be aware that IMS
 * technology may be subject of patents and licence terms, as being 
 * specified within the various IMS-related IETF, ITU-T, ETSI, and 3GPP
 * standards. Thus all Open IMS Core users have to take notice of this 
 * fact and have to agree to check out carefully before installing, 
 * using and extending the Open Source IMS Core System, if related 
 * patents and licences may become applicable to the intended usage 
 * context.  
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */
 
/**
 * \file
 *
 * P-CSCF Policy and Charging Control interface - Gq'
 *  
 * \author Alberto Diez Albaladejo -at- fokus dot fraunhofer dot de
 */

#include "pcc_gqprima.h"
#include "sip.h"
#include "pcc_avp.h"
#include "mod.h"

extern struct cdp_binds cdpb;

t_binding_list* get_binding_list(str sdp);
int free_binding_list(t_binding_list *list);
int add_binding_information(AAAMessage *m,t_binding_list *inblist,t_binding_list *outblist);




/**
 * Creates and adds a AVP_Framed_IP_Address and a AVP_ETSI_Address_Realm to a  AVP_ETSI_Globally_Unique_Address group.
 * @param msg - the Diameter message to add to.
 * @param ip - ue ip address
 * @param realm - realm
 * @returns 1 on success or 0 on error
 */
inline int gqprima_add_g_unique_address(AAAMessage *msg, str ip,enum ip_type version,str realm)
{
	AAA_AVP_LIST list;
	str group;

	list.head=0;list.tail=0;

	if (version==ip_type_v4)
	{
		cdpb.AAAAddAVPToList(&list,pcc_create_framed_ip_address(ip));
	} else
	{
		//TODO AVP_Framed_IPv6_Prefix
	}
	if (realm.len)
	{
		cdpb.AAAAddAVPToList(&list,cdpb.AAACreateAVP(AVP_ETSI_Address_Realm,AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,realm.s,realm.len,AVP_DUPLICATE_DATA));
	}

	group = cdpb.AAAGroupAVPS(list);

	cdpb.AAAFreeAVPList(&list);

	return cdpb.AAAAddAVPToMessage(msg,cdpb.AAACreateAVP(AVP_ETSI_Globally_Unique_Address,AAA_AVP_FLAG_MANDATORY|AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,group.s,group.len,AVP_FREE_DATA),msg->avpList.tail);

}

/**
 * Subsequent request can not modify
 * user-name, specific-action, charging-identifier, service-class
 *
 */

int gqprima_AAR(AAAMessage *aar,struct sip_msg *req, struct sip_msg *res, char *str1,int relatch)
{
	AAA_AVP *avp=0;
	//AAASession* auth=0;
	str id={0,0};
	str ip={0,0};
	enum ip_type version;
	str realm={0,0};
	str sdp={0,0};
	str service_class={"IMS Services",12};
	t_binding_list *blist=0;
	char x[4];
	enum p_dialog_direction tag=get_dialog_direction(str1);

	if (extract_id(req,tag,&id)!=-1 &&id.len)
	{
		avp=cdpb.AAACreateAVP(AVP_User_Name,AAA_AVP_FLAG_MANDATORY,0,id.s,id.len,AVP_FREE_DATA);
		cdpb.AAAAddAVPToMessage(aar,avp,aar->avpList.tail);
	}
	if (tag==DLG_MOBILE_ORIGINATING)
	{
		extract_body(req,&sdp);
		blist=get_binding_list(sdp);
		version=pcc_get_ip_address(req,&ip);
	} else {
		extract_body(res,&sdp);
		blist=get_binding_list(sdp);
		version=pcc_get_ip_address(res,&ip);
	}

	add_binding_information(aar,blist,NULL);
	if (relatch!=-1)
	{
		if (relatch) {
			set_4bytes(x,AVP_ETSI_Latching_Indication_Latch);
		} else {
			set_4bytes(x,AVP_ETSI_Latching_Indication_Relatch);
		}

		avp = cdpb.AAACreateAVP(AVP_ETSI_Latching_Indication,AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,x,4,AVP_DUPLICATE_DATA);
		cdpb.AAAAddAVPToMessage(aar,avp,aar->avpList.tail);
	}
	//add reservation priority
	set_4bytes(x,0);
	avp = cdpb.AAACreateAVP(AVP_ETSI_Reservation_Priority,AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,x,4,AVP_DUPLICATE_DATA);
	cdpb.AAAAddAVPToMessage(aar,avp,aar->avpList.tail);



	gqprima_add_g_unique_address(aar,ip,version,realm);
	//add service-class
	avp = cdpb.AAACreateAVP(AVP_ETSI_Service_Class,AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,service_class.s,service_class.len,AVP_DUPLICATE_DATA);
	cdpb.AAAAddAVPToMessage(aar,avp,aar->avpList.tail);

	//add overbooking-indicator
	//if overbooking means that the resources authorized can be re-used for other services later without
	//notifying the RACS
	set_4bytes(x,0);
	avp = cdpb.AAACreateAVP(AVP_ETSI_Overbooking_Indication,AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,x,4,AVP_DUPLICATE_DATA);
	cdpb.AAAAddAVPToMessage(aar,avp,aar->avpList.tail);
	/*
	 * TODO: care about multicast
	 *
	 * Authorization-Package-Id AVP [461] (string)
	 * Media-Authorization-Context-Id [462] (string)
	 *
	package_id=get_multicast_package(req,res,PER_SESSION);
	if (package_id.len)
	{
		//add authorization-package-id AVP
	}
	*/
	if (blist) free_binding_list(blist);
	return 1;
}

/*
 * In the AAA
 * store the Output-List received in the Binding-Information AVP
 * Reservation-Priority AVP [458] (enumerated 0-7)
 * error codes could be
 *
 * INSUFFICENT_RESOURCES 4041
 * COMMIT_FAILURE		 4043
 * REFRESH_FAILURE		 4044
 * QOS_PROFILE_FAILURE		4045
 * ACCES_PROFILE_FAILURE	4046
 * PRIORITY_NOT_GRANTED	4047
 * BINDING_FAILURE		5021
 * MODIFICATION_FAILURE	5041
 *
 */

/**
 * Gqprima processing of the AAA Answer
 * @param dia_msg - the AAA message
 * @return the result-code to be processed by someone else
 */
int gqprima_AAA(AAAMessage *dia_msg)
{
	AAA_AVP *avp=0;
	int rc=0;
	//get the result code
	avp=cdpb.AAAFindMatchingAVP(dia_msg,dia_msg->avpList.head,AVP_Result_Code,IMS_vendor_id_3GPP,AAA_FORWARD_SEARCH);
	if (!avp || avp->data.len==0)
	{
		rc=-1;
		//look Experimental-Result-Code AVP
	} else {
		rc=get_4bytes(avp->data.s);
	}
	//read binding information (output-list)
	//TODO
	return rc;
}



/**
 * Looks for the contact in the sip message and gets the ip address
 * @param r - sip message to look for contact
 * @param ip - the ip address to return
 * @returns 0 on error 1 on success
 */
int get_ip_address(struct sip_msg *r, str *ip)
{
	enum ip_type version=ip_type_v4;
	char *p=0;
	if (r)
	{
		if (r->contact)
		{
			p=strstr(r->contact->body.s,"sip:");
			p+=4; //sip:
			if (*p=='[')
			{
				version=ip_type_v6;
				ip->s=p++;
				p=index(ip->s,']');

			} else {
				version=ip_type_v4;
				ip->s=p;
				p=index(ip->s,':');

			}
			if (!p) return 0;
			ip->len=p-ip->s;
		}
	}
	return version;
}


/**
 * Adds the Binding-Information AVP
 * @param m - the Diameter Message to add to
 * @param inblist - the input binding list
 * @param outblist - the output binding list
 * @returns 0 on error 1 on success
 */
int add_binding_information(AAAMessage *m,t_binding_list *inblist,t_binding_list *outblist)
{
	AAA_AVP_LIST list={0,0},sublist={0,0},subsublist={0,0};
	t_binding_unit *bunit=0;
	char x[4];
	str data={0,0};
	if (inblist)
	{
		for(bunit=inblist->head;bunit;bunit=bunit->next)
		{
			if (bunit->v==ip_type_v4)
			{
				cdpb.AAAAddAVPToList(&subsublist,pcc_create_framed_ip_address(bunit->addr));

			} else {
				//TODO create_framed_ipv6_prefix
			}
			set_4bytes(x,bunit->port_start);
			cdpb.AAAAddAVPToList(&subsublist,cdpb.AAACreateAVP(AVP_ETSI_Port_Number,AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,x,4,AVP_DUPLICATE_DATA));

			data=cdpb.AAAGroupAVPS(subsublist);
			cdpb.AAAFreeAVPList(&subsublist);
			if (bunit->v==ip_type_v4)
			{
				cdpb.AAAAddAVPToList(&sublist,cdpb.AAACreateAVP(AVP_ETSI_V4_transport_address,AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,data.s,data.len,AVP_FREE_DATA));
			} else {
				cdpb.AAAAddAVPToList(&sublist,cdpb.AAACreateAVP(AVP_ETSI_V6_transport_address,AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,data.s,data.len,AVP_FREE_DATA));
			}

		}
		data=cdpb.AAAGroupAVPS(sublist);
		cdpb.AAAFreeAVPList(&sublist);
		cdpb.AAAAddAVPToList(&list,cdpb.AAACreateAVP(AVP_ETSI_Binding_Input_List,AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,data.s,data.len,AVP_FREE_DATA));
	}
	if (outblist)
	{
		for(bunit=outblist->head;bunit;bunit=bunit->next)
		{
			if (bunit->v==ip_type_v4)
			{
				if (bunit->addr.len)
				{
					cdpb.AAAAddAVPToList(&subsublist,pcc_create_framed_ip_address(bunit->addr));
				} else {
					data.s="0.0.0.0";
					data.len=7;
					cdpb.AAAAddAVPToList(&subsublist,pcc_create_framed_ip_address(data));
					//wildcard
				}
			} else {
				//TODO create_framed_ipv6_prefix
			}
			if (bunit->port_start)
			{
				set_4bytes(x,bunit->port_start);
			}
			cdpb.AAAAddAVPToList(&subsublist,cdpb.AAACreateAVP(AVP_ETSI_Port_Number,AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,x,4,AVP_DUPLICATE_DATA));

			data=cdpb.AAAGroupAVPS(subsublist);
			cdpb.AAAFreeAVPList(&subsublist);
			if (bunit->v==ip_type_v4)
			{
				cdpb.AAAAddAVPToList(&sublist,cdpb.AAACreateAVP(AVP_ETSI_V4_transport_address,AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,data.s,data.len,AVP_FREE_DATA));
			} else {
				cdpb.AAAAddAVPToList(&sublist,cdpb.AAACreateAVP(AVP_ETSI_V6_transport_address,AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,data.s,data.len,AVP_FREE_DATA));
			}

		}
		data=cdpb.AAAGroupAVPS(sublist);
		cdpb.AAAFreeAVPList(&sublist);
		cdpb.AAAAddAVPToList(&list,cdpb.AAACreateAVP(AVP_ETSI_Binding_Input_List,AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,data.s,data.len,AVP_FREE_DATA));
	}
	data=cdpb.AAAGroupAVPS(list);
	cdpb.AAAFreeAVPList(&list);
	cdpb.AAAAddAVPToMessage(m,cdpb.AAACreateAVP(AVP_ETSI_Binding_Information,AAA_AVP_FLAG_VENDOR_SPECIFIC,IMS_vendor_id_ETSI,data.s,data.len,AVP_FREE_DATA),m->avpList.tail);
	return 1;
}




/**
 * Free binding list (including the list itself)
 */
int free_binding_list(t_binding_list *list)
{
	t_binding_unit *bunit=0,*nbunit=0;
	if (list)
	{
		bunit=list->head;
		while (bunit)
		{
			nbunit=bunit->next;
			if (bunit->addr.s) shm_free(bunit->addr.s);
			shm_free(bunit);
			bunit=nbunit;
		}
		shm_free(list);
	}
	return 1;
}


/*
 * Extract the token
 * @param line - pointer to line in SDP body
 * @param token - string where the shm will be allocated and saved
 * @number - which token .. separated by ' ' or ' ' and then '\n'
 * returns 1 on success, 0 on error
 */


int extract_token_shm(char *line,str token,int number)
{
	char *p=0,*q=0,*r=0;
	int i;

	p=line;
	for(i=1; i<number;i++)
	{
		p=index(p,' ');
		if (p==NULL)
			return 0;
		while (*p==' ')
		{
			p++;
		}

	}
	q=index(p,' ');
	r=index(p,'\n');
	if (!r) r=index(p,'\0');
	q=q < r? q : r;
	while isspace(*(q-1)) q--;
	if (q-p>0)
	{
		i=q-p;
		token.s = shm_malloc(i);
		if(!token.s)
		{
			LOG(L_ERR,"ERR:extract_token_shm: no memory left for token.s\n");
			return 0;
		}
		memcpy(token.s,p,i);
		token.len=i;
		return 1;
	} else {
		return 0;
	}
}


/**
 * Analyzes the SDP content to extract the corresponding binding list
 * to be passed to the RACS functionality
 * @param m -  the SDP content to analyze
 * @returns a binding list allocated in share memory with the ips and ports
 */
/*
 * TODO: in order to generate the actual binding list we need the addresses in the access and in the core side
 * a binding list always has an even number of binding units
 * Each pair corresponds to the address on the access side and the address on the core side
 * it should be ordered as given in the SDP
 */
t_binding_list* get_binding_list(str sdp)
{
	t_binding_list* blist=0;
	t_binding_unit* bunit=0;
	char *cline=0,*mline=0,*nmline=0;
	enum ip_type version=ip_type_v4;
	char port[64];
	char *aux=0;
	str ip={0,0};
	cline=find_sdp_line(sdp.s,sdp.s+sdp.len,'c');
	if (!cline)
	{
		LOG(L_DBG,"DBG:get_binding_list: no c line in SDP\n");
		return 0;
	}
	mline=find_sdp_line(sdp.s,sdp.s+sdp.len,'m');
	if (!mline)
	{
		LOG(L_DBG,"DBG:get_binding_list: no m line in SDP\n");
		return 0;
	}
	blist = shm_malloc(sizeof(t_binding_list));
	if(!blist)
	{
		LOG(L_ERR,"ERR:get_binding_list: no memory left for blist\n");
		return 0;
	}
	memset(blist,0,sizeof(t_binding_list));
	if (mline>cline)
	{
		//its a general c-line so this IP address has to be remembered
		if (extract_token_shm(cline,ip,3)!=1)
		{
			LOG(L_DBG,"DBG:get_binding_list: error while extracting ip address from SDP\n");
			goto error;
		}
		if (index(ip.s,':'))
		{
			version=ip_type_v6;
		} else {
			version=ip_type_v4;
		}
		cline=find_next_sdp_line(cline,sdp.s+sdp.len,'c',NULL);
	}
	nmline=find_next_sdp_line(mline,sdp.s+sdp.len,'m',NULL);

	//loop
	// i have to process mline and i have nmline and cline
	while (mline)
	{
		bunit = shm_malloc(sizeof(t_binding_unit));
		if(!bunit)
		{
			LOG(L_ERR,"ERR:get_binding_list: no memory left for bunit\n");
			goto error;
		}
		memset(bunit,0,sizeof(t_binding_unit));
		if (!cline || (nmline && cline>nmline) || extract_token_shm(cline,bunit->addr,3)==0 )
		{
			STR_SHM_DUP(bunit->addr,ip,"get_binding_list")
			bunit->v=version;
		} else {
			//if i have arrived here then i have extracted the token in shared memory in the previous if
			if (index(bunit->addr.s,':')) bunit->v=ip_type_v6;
			else bunit->v=ip_type_v4;
		}
		if (extract_token(mline,port,64,2))
		{
			switch(is_a_port(port))
			{
				case 1:
					bunit->port_start=atoi(port);
					bunit->port_end=0;
					break;
				case 2:
					aux=index(port,'/');
					*aux='\0';
					bunit->port_start=atoi(port);
					bunit->port_end=atoi(aux+1);
					break;
				default:
					LOG(L_ERR,"ERR:get_binding_list: problem with port parsing from SDP\n");
				break;
			}
		} else {
			LOG(L_DBG,"DBG:get_binding_list: no port in media line in SDP\n");
		}
		LOG(L_DBG,"DBG:get_binding_list: binding unit with ip %.*s port_start %d port_end %d\n",bunit->addr.len,bunit->addr.s,bunit->port_start,bunit->port_end);
		FL_APPEND(blist,bunit)
		bunit = shm_malloc(sizeof(t_binding_unit));
		if(!bunit)
		{
			LOG(L_ERR,"ERR:get_binding_list: no memory left for bunit\n");
			goto error;
		}
		memset(bunit,0,sizeof(t_binding_unit));
		/*
		 * TODO: here generate the appropriate pair for the core side
		 * 0 will be wildcarded
		 * we need the r_contact struct that you get with hot,port,transport
		 * once you have that then look for the if (contact->pinhole) and there
		 * you would have the appropriate address
		 */
		FL_APPEND(blist,bunit)



		if (cline<nmline) {
			cline=find_next_sdp_line(cline,sdp.s+sdp.len,'c',NULL);
		}
		nmline=find_next_sdp_line(nmline,sdp.s+sdp.len,'m',NULL);
		mline=nmline;
	}
	goto end;
out_of_memory:
error:
	if (blist)
	{
		free_binding_list(blist);
	}
	if (ip.s) shm_free(ip.s);
	return 0;
end:
	if (ip.s) shm_free(ip.s);
	return blist;
}
