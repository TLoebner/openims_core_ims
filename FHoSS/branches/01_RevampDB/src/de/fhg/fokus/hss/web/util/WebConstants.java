/*
  *  Copyright (C) 2004-2007 FhG Fokus
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
  * patents and licenses may become applicable to the intended usage
  * context. 
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  
  * 
  */

package de.fhg.fokus.hss.web.util;

import java.util.ArrayList;
import java.util.List;

import de.fhg.fokus.hss.cx.CxConstants;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class WebConstants {
	public static final String FORWARD_SUCCESS = "success";
	public static final String FORWARD_FAILURE = "failure";
	public static final String FORWARD_DELETE = "delete";

	public static final List select_identity_type;
	static{
		select_identity_type = new ArrayList();
		select_identity_type.add(CxConstants.Identity_Type.Public_User_Identity);
		select_identity_type.add(CxConstants.Identity_Type.Wildcarded_PSI);
		select_identity_type.add(CxConstants.Identity_Type.Distinct_PSI);
	}

	public static final List select_auth_scheme;
	static{
		select_auth_scheme = new ArrayList();
		select_auth_scheme.add(CxConstants.AuthScheme.Auth_Scheme_AKAv1);
		select_auth_scheme.add(CxConstants.AuthScheme.Auth_Scheme_AKAv2);
		select_auth_scheme.add(CxConstants.AuthScheme.Auth_Scheme_MD5);
		select_auth_scheme.add(CxConstants.AuthScheme.Auth_Scheme_Digest);
		select_auth_scheme.add(CxConstants.AuthScheme.Auth_Scheme_HTTP_Digest_MD5);
		select_auth_scheme.add(CxConstants.AuthScheme.Auth_Scheme_Early);
		select_auth_scheme.add(CxConstants.AuthScheme.Auth_Scheme_NASS_Bundle);
	}
	
	public static final List select_user_state;
	static{
		select_user_state = new ArrayList();
		select_user_state.add(CxConstants.IMPU_user_state_Not_Registered);
		select_user_state.add(CxConstants.IMPU_user_state_Registered);
		select_user_state.add(CxConstants.IMPU_user_state_Unregistered);
		select_user_state.add(CxConstants.IMPU_user_state_Auth_Pending);
	}

	public static final List<Tuple> select_default_handling;
	static{
		select_default_handling = new ArrayList<Tuple>();
		select_default_handling.add(new Tuple("Session - Continued", CxConstants.Default_Handling_Session_Continued));
		select_default_handling.add(new Tuple("Session - Terminated", CxConstants.Default_Handling_Session_Terminated));
	}
	
	public static final List<Tuple> select_condition_type_cnf;
	static{
		select_condition_type_cnf = new ArrayList<Tuple>();
		select_condition_type_cnf.add(new Tuple("Disjunctive Normal Format", CxConstants.ConditionType.DNF.code));
		select_condition_type_cnf.add(new Tuple("Conjunctive Normal Format", CxConstants.ConditionType.CNF.code));
	}	

	public static final List<Tuple> select_spt_type;
	static{
		select_spt_type = new ArrayList<Tuple>();
		select_spt_type.add(new Tuple("Request-URI", CxConstants.SPT_Type_RequestURI));
		select_spt_type.add(new Tuple("Method", CxConstants.SPT_Type_Method));
		select_spt_type.add(new Tuple("SIP-Header", CxConstants.SPT_Type_SIPHeader));
		select_spt_type.add(new Tuple("Session-Case", CxConstants.SPT_Type_SessionCase));
		select_spt_type.add(new Tuple("SDP-Line", CxConstants.SPT_Type_SessionDescription));
	}

	public static final ArrayList<Tuple> select_spt_method_type;
	static{
		select_spt_method_type = new ArrayList<Tuple>();
		select_spt_method_type.add(new Tuple("INVITE", "INVITE"));
		select_spt_method_type.add(new Tuple("REGISTER", "REGISTER"));
		select_spt_method_type.add(new Tuple("CANCEL", "CANCEL"));
		select_spt_method_type.add(new Tuple("OPTION", "OPTION"));
		select_spt_method_type.add(new Tuple("PUBLISH", "PUBLISH"));
		select_spt_method_type.add(new Tuple("SUBSCRIBE", "SUBSCRIBE"));
		select_spt_method_type.add(new Tuple("MESSAGE", "MESSAGE"));
		
		//etc
	}
	public static final ArrayList<Tuple> select_profile_part_indicator;
	static{
		select_profile_part_indicator = new ArrayList<Tuple>();
		select_profile_part_indicator.add(new Tuple("Any", -1));
		select_profile_part_indicator.add(new Tuple(CxConstants.Profile_Part_Indicator_Registered_Name,
				CxConstants.Profile_Part_Indicator_Registered));
		select_profile_part_indicator.add(new Tuple(CxConstants.Profile_Part_Indicator_UnRegistered_Name, 
				CxConstants.Profile_Part_Indicator_UnRegistered));
	}

	public static final ArrayList<Tuple> select_cap_type;
	static{
		select_cap_type = new ArrayList<Tuple>();
		select_cap_type.add(new Tuple("Optional", 0));
		select_cap_type.add(new Tuple("Mandatory", 1));		
	}
	public static final ArrayList<Tuple> select_direction_of_request;
	static{
		select_direction_of_request = new ArrayList<Tuple>();
		select_direction_of_request.add(new Tuple(CxConstants.Direction_of_Request_Originating_Session_Name, 
				CxConstants.Direction_of_Request_Originating_Session));
		select_direction_of_request.add(new Tuple(CxConstants.Direction_of_Request_Terminating_Registered_Name, 
				CxConstants.Direction_of_Request_Terminating_Registered));
		select_direction_of_request.add(new Tuple(CxConstants.Direction_of_Request_Terminating_Unregistered_Name, 
				CxConstants.Direction_of_Request_Terminating_Unregistered));
		select_direction_of_request.add(new Tuple(CxConstants.Direction_of_Request_Originating_Unregistered_Name, 
				CxConstants.Direction_of_Request_Originating_Unregistered));
	}
	
	
	
	
}
