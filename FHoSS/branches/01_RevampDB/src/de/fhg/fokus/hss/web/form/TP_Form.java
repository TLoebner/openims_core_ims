/*
  *  Copyright (C) 2004-2007 FhG Fokus
  *
  * This file is part of Open IMS Core - an open source IMS CSCFs & HSS
  * implementation
  *
  * Open IMS Core is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License, or
  * (at your option) any later version.
  *
  * For a license to use the Open IMS Core software under conditions
  * other than those described here, or to purchase support for this
  * software, please contact Fraunhofer FOKUS by e-mail at the following
  * addresses:
  *     info@open-ims.org
  *
  * Open IMS Core is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * It has to be noted that this Open Source IMS Core System is not
  * intended to become or act as a product in a commercial context! Its
  * sole purpose is to provide an IMS core reference implementation for
  * IMS technology testing and IMS application prototyping for research
  * purposes, typically performed in IMS test-beds.
  *
  * Users of the Open Source IMS Core System have to be aware that IMS
  * technology may be subject of patents and licence terms, as being
  * specified within the various IMS-related IETF, ITU-T, ETSI, and 3GPP
  * standards. Thus all Open IMS Core users have to take notice of this
  * fact and have to agree to check out carefully before installing,
  * using and extending the Open Source IMS Core System, if related
  * patents and licenses may become applicable to the intended usage
  * context. 
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  
  * 
  */

package de.fhg.fokus.hss.web.form;

import org.apache.commons.collections.Factory;
import org.apache.commons.collections.list.LazyList;
import org.apache.struts.action.ActionErrors;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionMapping;
import org.apache.struts.action.ActionMessage;

import de.fhg.fokus.hss.cx.CxConstants;
import de.fhg.fokus.hss.web.util.WebConstants;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import javax.servlet.http.HttpServletRequest;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class TP_Form extends ActionForm implements Serializable{
	// TP properties
	private int id;
	private String name;
	private int condition_type_cnf;
	
	// SPT properties	
	private List spts;
	private int type;
	private int group;
	
	private int ifc_id;
	private int associated_ID;
	private List select_ifc;
	private List select_condition_type;
	private List select_spt;
	private String nextAction;
	
	public void reset(ActionMapping actionMapping, HttpServletRequest request){
    	this.id = -1;
    	this.name = null;
    	this.condition_type_cnf = CxConstants.ConditionType.CNF.code;

    	Factory factory = new Factory() {
                public Object create() {
                    return new SPT_Form();
                }
        };

        this.spts = LazyList.decorate(new ArrayList(), factory);

    	this.type = 0;
    	this.group = 1;

    	this.nextAction = null;
    	this.ifc_id = -1;
    	this.associated_ID = -1;
    	this.select_ifc = null;
    	this.select_condition_type = WebConstants.select_condition_type_cnf;

    }
	
    public ActionErrors validate(ActionMapping actionMapping, HttpServletRequest request){
        ActionErrors actionErrors = new ActionErrors();

        if (name == null || name.equals("")){
        	actionErrors.add("tp.name", new ActionMessage("tp_form.error.name"));
        }
        return actionErrors;
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

	public String getNextAction() {
		return nextAction;
	}

	public void setNextAction(String nextAction) {
		this.nextAction = nextAction;
	}

	public int getCondition_type_cnf() {
		return condition_type_cnf;
	}

	public void setCondition_type_cnf(int condition_type_cnf) {
		this.condition_type_cnf = condition_type_cnf;
	}

	public int getAssociated_ID() {
		return associated_ID;
	}

	public void setAssociated_ID(int associated_ID) {
		this.associated_ID = associated_ID;
	}

	public int getIfc_id() {
		return ifc_id;
	}

	public void setIfc_id(int ifc_id) {
		this.ifc_id = ifc_id;
	}

	public List getSelect_ifc() {
		return select_ifc;
	}

	public void setSelect_ifc(List select_ifc) {
		this.select_ifc = select_ifc;
	}

	public List getSelect_condition_type() {
		return select_condition_type;
	}

	public void setSelect_condition_type(List select_condition_type) {
		this.select_condition_type = select_condition_type;
	}

	public List getSelect_spt() {
		return select_spt;
	}

	public void setSelect_spt(List select_spt) {
		this.select_spt = select_spt;
	}

	public int getGroup() {
		return group;
	}

	public void setGroup(int group) {
		this.group = group;
	}

	public List getSpts() {
		return spts;
	}

	public void setSpts(List spts) {
		this.spts = spts;
	}

	public int getType() {
		return type;
	}

	public void setType(int type) {
		this.type = type;
	}

	
	
}
