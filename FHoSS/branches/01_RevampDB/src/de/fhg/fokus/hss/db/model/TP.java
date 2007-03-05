/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class TP {
	private int id;
	private String name;
	private int condition_type_cnf;
	
	public TP(){}

	public int getCondition_type_cnf() {
		return condition_type_cnf;
	}

	public void setCondition_type_cnf(int condition_type_cnf) {
		this.condition_type_cnf = condition_type_cnf;
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