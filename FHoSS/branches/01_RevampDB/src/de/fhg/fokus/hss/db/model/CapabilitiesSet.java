/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class CapabilitiesSet {
	private int id;
	private int id_set;
	private String name;
	private int is_mandatory;
	
	private Capability capability;

	public CapabilitiesSet(){}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getId_set() {
		return id_set;
	}

	public void setId_set(int id_set) {
		this.id_set = id_set;
	}

	public int getIs_mandatory() {
		return is_mandatory;
	}

	public void setIs_mandatory(int is_mandatory) {
		this.is_mandatory = is_mandatory;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public Capability getCapability() {
		return capability;
	}

	public void setCapability(Capability capability) {
		this.capability = capability;
	}
	
	
}
