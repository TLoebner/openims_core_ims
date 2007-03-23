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
package de.fhg.fokus.hss.cx;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class CxConstants {

	// IMPU_user_state
	public static final short IMPU_user_state_Not_Registered = 0;
	public static final short IMPU_user_state_Registered = 1;
	public static final short IMPU_user_state_Unregistered = 2;
	public static final short IMPU_user_state_Auth_Pending = 3;
	
	// Public Idenitity Types
	public static final short IMPU_type_Public_User_Identity = 0;
	public static final short IMPU_type_Distinct_PSI = 1;
	public static final short IMPU_type_Wildcarded_PSI = 2;
	 
	// Authentication_Parameters_Size
	public static final int Auth_Parm_Secret_Key_Size = 16;
	public static final int Auth_Parm_Rand_Size = 16;
	public static final int Auth_Parm_Sqn_Size = 6;
	public static final int Auth_Parm_Ak_Size = 6;
	public static final int Auth_Parm_Amf_Size = 2;
	public static final int Auth_Parm_Mac_Size = 8;
	public static final int Auth_Parm_Ck_Size = 16;
	public static final int Auth_Parm_Ik_Size = 16;
	public static final int Auth_Parm_Res_Size = 16; // 4-16 octets
	
	// Profile_Part_Indicator
	public static final int Profile_Part_Indicator_Registered = 0;
	public static final int Profile_Part_Indicator_UnRegistered = 1;
	
	// Server_Assignment_Type
	public static final int Server_Assignment_Type_No_Assignment = 0;
	public static final int Server_Assignment_Type_Registration = 1;
	public static final int Server_Assignment_Type_Re_Registration = 2;
	public static final int Server_Assignment_Type_Unregistered_User = 3;
	public static final int Server_Assignment_Type_Timeout_Deregistration = 4;
	public static final int Server_Assignment_Type_User_Deregistration = 5;
	public static final int Server_Assignment_Type_Timeout_Deregistration_Store_Server_Name = 6;
	public static final int Server_Assignment_Type_User_Deregistration_Store_Server_Name = 7;
	public static final int Server_Assignment_Type_Administrative_Deregistration = 8;
	public static final int Server_Assignment_Type_Authentication_Failure = 9;
	public static final int Server_Assignment_Type_Authentication_Timeout = 10;
	public static final int Server_Assignment_Type_Deregistration_Too_Much_Data = 11;
	
	// User_Data_Already_Available
	public static final int User_Data_Not_Available = 0;
	public static final int User_Data_Already_Available = 1;
	
	// Deregistration-Reason
	public static final int Deregistration_Reason_Permanent_Termination = 0;
	public static final int Deregistration_Reason_New_Server_Assigned = 1;
	public static final int Deregistration_Reason_Server_Change = 2;
	public static final int Deregistration_Reason_Remove_S_CSCF = 3;
	
	// SPT Type: Choice Of...
	public static final int SPT_Type_RequestURI = 0;
	public static final int SPT_Type_Method = 1;
	public static final int SPT_Type_SIPHeader = 2;
	public static final int SPT_Type_SessionCase = 3;
	public static final int SPT_Type_SessionDescription = 4;
	
	// tRegistrationType
	public static final int Registration_Type_Initial_Registration = 0;
	public static final int Registration_Type_Initial_Re_Registration = 1;
	public static final int Registration_Type_Initial_De_Registration = 2;
	
	// tDefaultHandling
	public static final int Default_Handling_Session_Continued = 0;
	public static final int Default_Handling_Session_Terminated = 1;
	
	
	public enum AuthScheme{
		Auth_Scheme_AKAv1("Digest-AKAv1-MD5", 1),
		Auth_Scheme_AKAv2("Digest-AKAv2-MD5", 2),
		Auth_Scheme_MD5("Digest-MD5", 4),
		Auth_Scheme_Early("Early-IMS", 8);
		
		private int code;
		private String name;
		
		private AuthScheme(String name, int code){
			this.name = name;
			this.code = code;
		}

		public int getCode() {
			return code;
		}

		public String getName() {
			return name;
		}
	}
	
	//tIdentityType
	public enum Identity_Type{
		Public_User_Identity("Public_User_Identity", 0),
		Distinct_PSI("Distinct_PSI", 1),
		Wildcarded_PSI("Wildcarded_PSI", 2);

		public int code;
		public String name;
		
		private Identity_Type(String name, int code){
			this.name = name;
			this.code = code;
		}

		public int getCode() {
			return code;
		}

		public void setCode(int code) {
			this.code = code;
		}

		public String getName() {
			return name;
		}

		public void setName(String name) {
			this.name = name;
		}
	}
	
	//Condition Type CNF
	public enum ConditionType{
		CNF("Conjunctive Normal Format", 1),
		DNF("Disjunctive Normal Format", 0);

		public int code;
		public String name;
		
		private ConditionType(String name, int code){
			this.name = name;
			this.code = code;
		}

		public int getCode() {
			return code;
		}

		public String getName() {
			return name;
		}

	}
	
	
}
