/**
 * 
 */
package de.fhg.fokus.hss.web.form;

import java.io.Serializable;

import org.apache.struts.action.ActionForm;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class DeleteForm extends ActionForm implements Serializable{
	int id;

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}
	
}
