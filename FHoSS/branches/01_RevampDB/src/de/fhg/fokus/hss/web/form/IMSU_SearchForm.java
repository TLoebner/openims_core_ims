/**
 * 
 */
package de.fhg.fokus.hss.web.form;

import java.io.Serializable;

import javax.servlet.http.HttpServletRequest;

import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionMapping;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class IMSU_SearchForm extends ActionForm implements Serializable{
	private String imsu_id;
	private String name;
	
	private String crtPage;
	private String rowsPerPage;
	
	public void reset(ActionMapping arg0, HttpServletRequest arg1) {
  		this.name = "";
  		this.imsu_id = "";
  		crtPage = "1";
  		rowsPerPage = "20";
  	}
	
	public String getCrtPage() {
		return crtPage;
	}

	public void setCrtPage(String crtPage) {
		this.crtPage = crtPage;
	}

	public String getRowsPerPage() {
		return rowsPerPage;
	}
	public void setRowsPerPage(String rowsPerPage) {
		this.rowsPerPage = rowsPerPage;
	}

	public String getImsu_id() {
		return imsu_id;
	}

	public void setImsu_id(String imsu_id) {
		this.imsu_id = imsu_id;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}
}
