package de.fhg.fokus.hss.web.form;

import org.apache.struts.action.ActionErrors;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionMapping;
import org.apache.struts.action.ActionMessage;

import java.io.Serializable;
import java.util.List;

import javax.servlet.http.HttpServletRequest;

public class IMSU_Form extends ActionForm implements Serializable{
	private int id;
	private String name;
	private String scscf_name;
	private String diameter_name;
	private int id_capabilities_set;
	private int id_preferred_scscf;

	private List select_capabilities_set;
	private List select_preferred_scscf;
	
	private String nextAction;
	
	
    public void reset(ActionMapping actionMapping, HttpServletRequest request){
    	this.id = -1;
    	this.name = null;
    	this.scscf_name = null;
    	this.diameter_name = null;
    	this.id_capabilities_set = -1;
    	this.id_preferred_scscf = -1;
    	//this.select_capabilities_set = null;
    	//this.select_preferred_scscf = null;
    }
	
    public ActionErrors validate(ActionMapping actionMapping, HttpServletRequest request){
        ActionErrors actionErrors = new ActionErrors();

        if (name == null || name.equals("")){
        	actionErrors.add("name", new ActionMessage("imsu_form.error.name"));
        }
        
        return actionErrors;
    }

	public String getDiameter_name() {
		return diameter_name;
	}

	public void setDiameter_name(String diameter_name) {
		this.diameter_name = diameter_name;
	}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getId_capabilities_set() {
		return id_capabilities_set;
	}

	public void setId_capabilities_set(int id_capabilities_set) {
		this.id_capabilities_set = id_capabilities_set;
	}

	public int getId_preferred_scscf() {
		return id_preferred_scscf;
	}

	public void setId_preferred_scscf(int id_preferred_scscf) {
		this.id_preferred_scscf = id_preferred_scscf;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getNextAction() {
		return nextAction;
	}

	public void setNextAction(String nextAction) {
		this.nextAction = nextAction;
	}

	public String getScscf_name() {
		return scscf_name;
	}

	public void setScscf_name(String scscf_name) {
		this.scscf_name = scscf_name;
	}

	public List getSelect_capabilities_set() {
		return select_capabilities_set;
	}

	public void setSelect_capabilities_set(List select_capabilities_set) {
		this.select_capabilities_set = select_capabilities_set;
	}

	public List getSelect_preferred_scscf() {
		return select_preferred_scscf;
	}

	public void setSelect_preferred_scscf(List select_preferred_scscf) {
		this.select_preferred_scscf = select_preferred_scscf;
	}
}
