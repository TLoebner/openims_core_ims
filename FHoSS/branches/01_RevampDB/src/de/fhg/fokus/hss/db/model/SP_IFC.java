/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class SP_IFC {
	private int id;
	private SP sp;
	private IFC ifc;
	private int priority;
	
	public SP_IFC(){}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getPriority() {
		return priority;
	}

	public void setPriority(int priority) {
		this.priority = priority;
	}

	public IFC getIfc() {
		return ifc;
	}

	public void setIfc(IFC ifc) {
		this.ifc = ifc;
	}

	public SP getSp() {
		return sp;
	}

	public void setSp(SP sp) {
		this.sp = sp;
	}
	
}

