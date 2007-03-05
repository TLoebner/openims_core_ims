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
	

	//Authentication_Parameters_Size
	public static final int Auth_Parm_Secret_Key_Size = 16;
	public static final int Auth_Parm_Rand_Size = 16;
	public static final int Auth_Parm_Sqn_Size = 6;
	public static final int Auth_Parm_Ak_Size = 6;
	public static final int Auth_Parm_Amf_Size = 2;
	public static final int Auth_Parm_Mac_Size = 8;
	public static final int Auth_Parm_Ck_Size = 16;
	public static final int Auth_Parm_Ik_Size = 16;
	public static final int Auth_Parm_Res_Size = 16; // 4-16 octets
	
	public enum AuthScheme{
		Auth_Scheme_AKAv1("Digest-AKAv1-MD5", 1),
		Auth_Scheme_AKAv2("Digest-AKAv2-MD5", 2),
		Auth_Scheme_MD5("Digest-MD5", 3),
		Auth_Scheme_Early("Early-IMS", 4);
		
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
		
		// Authentication_Scheme
		//public static final int Auth_Scheme_AKAv1 = 1;
		//public static final int Auth_Scheme_AKAv2 = 2;
		//public static final int Auth_Scheme_MD5 = 4;
		//public static final int Auth_Scheme_Early = 8;

	}
}
