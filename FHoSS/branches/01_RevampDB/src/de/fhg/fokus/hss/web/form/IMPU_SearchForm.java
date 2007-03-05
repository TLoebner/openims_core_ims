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

public class IMPU_SearchForm extends ActionForm implements Serializable{
	private String impu_id;
	private String identity;
	private String id_impu_implicitset;
	private int user_state;
	
	private String crtPage;
	private String rowsPerPage;
	
	public String getIdentity() {
		return identity;
	}
	public void setIdentity(String identity) {
		this.identity = identity;
	}
	public int getUser_state() {
		return user_state;
	}
	public void setUser_state(int user_state) {
		this.user_state = user_state;
	}
	public String getRowsPerPage() {
		return rowsPerPage;
	}
	public void setRowsPerPage(String rowsPerPage) {
		this.rowsPerPage = rowsPerPage;
	}
	
  	public String getCrtPage() {
		return crtPage;
	}
	public void setCrtPage(String crtPage) {
		this.crtPage = crtPage;
	}
	public String getId_impu_implicitset() {
		return id_impu_implicitset;
	}
	public void setId_impu_implicitset(String id_impu_implicitset) {
		this.id_impu_implicitset = id_impu_implicitset;
	}
	public String getImpu_id() {
		return impu_id;
	}
	public void setImpu_id(String impu_id) {
		this.impu_id = impu_id;
	}
	public void reset(ActionMapping arg0, HttpServletRequest arg1) {
  		
  		this.id_impu_implicitset = "";
  		this.identity = "";
  		this.impu_id = "";
  		crtPage = "1";
  		rowsPerPage = "20";
  	}
	
}
