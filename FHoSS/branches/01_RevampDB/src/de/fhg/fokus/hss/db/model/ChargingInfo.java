/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class ChargingInfo {
	private int id;
	private String name;
	private String pri_ecf;
	private String sec_ecf;
	private String pri_ccf;
	private String sec_ccf;

	public ChargingInfo(){}

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

	public String getPri_ccf() {
		return pri_ccf;
	}

	public void setPri_ccf(String pri_ccf) {
		this.pri_ccf = pri_ccf;
	}

	public String getPri_ecf() {
		return pri_ecf;
	}

	public void setPri_ecf(String pri_ecf) {
		this.pri_ecf = pri_ecf;
	}

	public String getSec_ccf() {
		return sec_ccf;
	}

	public void setSec_ccf(String sec_ccf) {
		this.sec_ccf = sec_ccf;
	}

	public String getSec_ecf() {
		return sec_ecf;
	}

	public void setSec_ecf(String sec_ecf) {
		this.sec_ecf = sec_ecf;
	}

}
