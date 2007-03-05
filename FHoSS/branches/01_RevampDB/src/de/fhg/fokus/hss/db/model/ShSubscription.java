/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class ShSubscription {
	private int id;
	private int id_application_server;
	private int id_impi;
	private int id_impu;
	private int data_ref;
	private int expires;

	public ShSubscription(){}

	public int getData_ref() {
		return data_ref;
	}

	public void setData_ref(int data_ref) {
		this.data_ref = data_ref;
	}

	public int getExpires() {
		return expires;
	}

	public void setExpires(int expires) {
		this.expires = expires;
	}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getId_application_server() {
		return id_application_server;
	}

	public void setId_application_server(int id_application_server) {
		this.id_application_server = id_application_server;
	}

	public int getId_impi() {
		return id_impi;
	}

	public void setId_impi(int id_impi) {
		this.id_impi = id_impi;
	}

	public int getId_impu() {
		return id_impu;
	}

	public void setId_impu(int id_impu) {
		this.id_impu = id_impu;
	}
	
}
