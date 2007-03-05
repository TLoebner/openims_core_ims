package de.fhg.fokus.hss.web.form;

import org.apache.struts.action.ActionErrors;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionMapping;
import org.apache.struts.action.ActionMessage;

import java.io.Serializable;
import java.util.List;

import javax.servlet.http.HttpServletRequest;

public class IMPU_Form extends ActionForm implements Serializable{

	private int id;
	private String identity;
	private boolean barring;
	private int id_sp;
	private int id_impu_implicitset;
	private int id_charging_info;
	private int user_state;
	private List select_sp;
	private List select_charging_info;
	
	private String nextAction;
	
	public boolean isBarring() {
		return barring;
	}
	public void setBarring(boolean barring) {
		this.barring = barring;
	}

	public int getId_charging_info() {
		return id_charging_info;
	}
	public void setId_charging_info(int id_charging_info) {
		this.id_charging_info = id_charging_info;
	}
	public void setId_sp(int id_sp) {
		this.id_sp = id_sp;
	}
	public int getId_impu_implicitset() {
		return id_impu_implicitset;
	}
	public void setId_impu_implicitset(int id_impu_implicitset) {
		this.id_impu_implicitset = id_impu_implicitset;
	}
	public String getIdentity() {
		return identity;
	}
	public void setIdentity(String identity) {
		this.identity = identity;
	}
	public int getId() {
		return id;
	}
	public void setId(int id) {
		this.id = id;
	}
	public int getId_sp() {
		return id_sp;
	}
	public void set_Id_sp(int id_sp) {
		this.id_sp = id_sp;
	}
	public List getSelect_charging_info() {
		return select_charging_info;
	}
	public void setSelect_charging_info(List select_charging_info) {
		this.select_charging_info = select_charging_info;
	}
	public List getSelect_sp() {
		return select_sp;
	}
	public void setSelect_sp(List select_sp) {
		this.select_sp = select_sp;
	}
		
	public int getUser_state() {
		return user_state;
	}
	public void setUser_state(int user_state) {
		this.user_state = user_state;
	}
	public String getNextAction() {
		return nextAction;
	}
	public void setNextAction(String nextAction) {
		this.nextAction = nextAction;
	}
	
    public void reset(ActionMapping actionMapping, HttpServletRequest request){
    	this.id = -1;
    	this.id_impu_implicitset = -1;
    	this.identity = null;
    	this.id_sp = -1;
    	this.id_charging_info = -1;
    	this.barring = false;
    	this.user_state = 0;
    	
    	this.select_charging_info = null;
    	this.select_sp = null;
    }
	
    public ActionErrors validate(ActionMapping actionMapping, HttpServletRequest request){
        ActionErrors actionErrors = new ActionErrors();

        if (identity == null || ((!identity.startsWith("sip:") && !identity.startsWith("sips:")) && 
        		(!identity.startsWith("tel:") && !identity.startsWith("tels:")))){
        	actionErrors.add("identity", new ActionMessage("impu_form.error.identity"));
        }

        if (this.id_sp == -1){
        	actionErrors.add("id_sp", new ActionMessage("impu_form.error.id_sp"));
        }

        if (this.id_charging_info == -1){
        	actionErrors.add("id_charging_info", new ActionMessage("impu_form.error.id_charging_info"));
        }
        
        return actionErrors;
    }
	
}
