/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class SP {
	private int id;
	private String name;
	private int cn_service_auth;

	public SP(){}

	public int getCn_service_auth() {
		return cn_service_auth;
	}

	public void setCn_service_auth(int cn_service_auth) {
		this.cn_service_auth = cn_service_auth;
	}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}
	
}
