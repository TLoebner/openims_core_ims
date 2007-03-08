/**
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
}
