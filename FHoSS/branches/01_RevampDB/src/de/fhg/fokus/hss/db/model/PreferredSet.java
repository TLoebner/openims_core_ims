/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class PreferredSet {
	private int id;
	private int id_set;
	private String name;
	private String scscf_name;
	private int priority;
	
	public PreferredSet(){}

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

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public int getPriority() {
		return priority;
	}

	public void setPriority(int priority) {
		this.priority = priority;
	}

	public String getScscf_name() {
		return scscf_name;
	}

	public void setScscf_name(String scscf_name) {
		this.scscf_name = scscf_name;
	}

}

