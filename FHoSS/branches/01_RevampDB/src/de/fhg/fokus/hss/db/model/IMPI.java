/**
 * 
 */
package de.fhg.fokus.hss.db.model;

import java.util.Set;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class IMPI {
	// table fields
	private int id;
	private String identity;
	private String k;
	private int auth_scheme;
	private byte[] amf;
	private byte[] op;
	private String sqn;
	private String ip;

	// associations
	private IMSU imsu;

	// constants
	public static final int AUTH_SCHEME_AKAv1 = 1;
	public static final int AUTH_SCHEME_AKAv2 = 1 << 1;
	public static final int AUTH_SCHEME_MD5 = 1 << 2;
	public static final int AUTH_SCHEME_EARLY = 1 << 3;
	
	public IMPI(){}

	public static int generateAuthScheme(boolean akav1, boolean akav2, boolean md5, boolean early, boolean all){
		if (all){
			return 15;
		}
		else{
			int result = 0;

			if (akav1){
				result |= IMPI.AUTH_SCHEME_AKAv1; 
			}
			if (akav2){
				result |= IMPI.AUTH_SCHEME_AKAv2; 
			}
			if (md5){
				result |= IMPI.AUTH_SCHEME_MD5; 
			}
			if (early){
				result |= IMPI.AUTH_SCHEME_EARLY; 
			}
			return result;
		}
	}
	
	
	// getters and setters
	public int getAuth_scheme() {
		return auth_scheme;
	}


	public void setAuth_scheme(int auth_scheme) {
		this.auth_scheme = auth_scheme;
	}


	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public String getIdentity() {
		return identity;
	}

	public void setIdentity(String identity) {
		this.identity = identity;
	}

	public String getIp() {
		return ip;
	}

	public void setIp(String ip) {
		this.ip = ip;
	}

	public String getK() {
		return k;
	}

	public void setK(String k) {
		this.k = k;
	}

	public byte[] getAmf() {
		return amf;
	}

	public void setAmf(byte[] amf) {
		this.amf = amf;
	}

	public byte[] getOp() {
		return op;
	}

	public void setOp(byte[] op) {
		this.op = op;
	}

	public String getSqn() {
		return sqn;
	}

	public void setSqn(String sqn) {
		this.sqn = sqn;
	}

	public IMSU getImsu() {
		return imsu;
	}

	public void setImsu(IMSU imsu) {
		this.imsu = imsu;
	}
	
}
