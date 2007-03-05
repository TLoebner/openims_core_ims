/**
 * 
 */
package de.fhg.fokus.hss.db.model;

import java.util.Set;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class IMSU {
	private int id;
	private String name;
	private String scscf_name;
	private String diameter_name;
	
	private int id_capabilities_set;
	private int id_preferred_scscf;
	
	public IMSU(){}
	
	public String getDiameter_name() {
		return diameter_name;
	}
	public void setDiameter_name(String diameter_name) {
		this.diameter_name = diameter_name;
	}
	public int getId() {
		return id;
	}
	public void setId(int id) {
		this.id = id;
	}

	public int getId_capabilities_set() {
		return id_capabilities_set;
	}

	public void setId_capabilities_set(int id_capabilities_set) {
		this.id_capabilities_set = id_capabilities_set;
	}

	public int getId_preferred_scscf() {
		return id_preferred_scscf;
	}
	public void setId_preferred_scscf(int id_preferred_scscf) {
		this.id_preferred_scscf = id_preferred_scscf;
	}
	public String getName() {
		return name;
	}
	public void setName(String name) {
		this.name = name;
	}
	public String getScscf_name() {
		return scscf_name;
	}
	public void setScscf_name(String scscf_name) {
		this.scscf_name = scscf_name;
	}
}

