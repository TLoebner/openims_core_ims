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

public class IMPI_SearchForm extends ActionForm implements Serializable{
	private String impi_id;
	private String identity;
	
	private String crtPage;
	private String rowsPerPage;
	
	public void reset(ActionMapping arg0, HttpServletRequest arg1) {
  		
  		this.identity = "";
  		this.impi_id = "";
  		crtPage = "1";
  		rowsPerPage = "20";
  	}
	
	public String getCrtPage() {
		return crtPage;
	}

	public void setCrtPage(String crtPage) {
		this.crtPage = crtPage;
	}

	public String getIdentity() {
		return identity;
	}
	public void setIdentity(String identity) {
		this.identity = identity;
	}

	public String getRowsPerPage() {
		return rowsPerPage;
	}
	public void setRowsPerPage(String rowsPerPage) {
		this.rowsPerPage = rowsPerPage;
	}

	public String getImpi_id() {
		return impi_id;
	}

	public void setImpi_id(String impi_id) {
		this.impi_id = impi_id;
	}
	
}
