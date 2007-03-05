/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class SP_Shared_IFC_Set {
	private int id;
	private SP sp;
	private Shared_IFC_Set shared_ifc_set;
	
	public SP_Shared_IFC_Set(){}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public Shared_IFC_Set getShared_ifc_set() {
		return shared_ifc_set;
	}

	public void setShared_ifc_set(Shared_IFC_Set shared_ifc_set) {
		this.shared_ifc_set = shared_ifc_set;
	}

	public SP getSp() {
		return sp;
	}

	public void setSp(SP sp) {
		this.sp = sp;
	}

}

