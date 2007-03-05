/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class Shared_IFC_Set {
	private int id;
	private int id_set;
	private String name;
	private IFC ifc;
	private int priority;
	
	public Shared_IFC_Set(){}

	public void setId(int id) {
		this.id = id;
	}

	public IFC getIfc() {
		return ifc;
	}

	public void setIfc(IFC ifc) {
		this.ifc = ifc;
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

	public int getId() {
		return id;
	}

}

