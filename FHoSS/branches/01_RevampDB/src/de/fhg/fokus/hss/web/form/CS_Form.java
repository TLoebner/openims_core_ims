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

import org.apache.struts.action.ActionErrors;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionMapping;
import org.apache.struts.action.ActionMessage;

import de.fhg.fokus.hss.cx.CxConstants;

import java.io.Serializable;
import javax.servlet.http.HttpServletRequest;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */


public class CS_Form extends ActionForm implements Serializable{

	private int id;
	private String name;
	private String pri_ecf;
	private String sec_ecf;
	private String pri_ccf;
	private String sec_ccf;
	
	private String nextAction;
	private String deleteActivation;
	
	public void reset(ActionMapping actionMapping, HttpServletRequest request){
    	this.id = -1;
    	this.name = null;
    	this.deleteActivation = "true";
    }
	
    public ActionErrors validate(ActionMapping actionMapping, HttpServletRequest request){
        ActionErrors actionErrors = new ActionErrors();

        if (name == null || name.equals("")){
        	actionErrors.add("cs.error.name", new ActionMessage("cs.error.name"));
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

	public String getPri_ccf() {
		return pri_ccf;
	}

	public void setPri_ccf(String pri_ccf) {
		this.pri_ccf = pri_ccf;
	}

	public String getPri_ecf() {
		return pri_ecf;
	}

	public void setPri_ecf(String pri_ecf) {
		this.pri_ecf = pri_ecf;
	}

	public String getSec_ccf() {
		return sec_ccf;
	}

	public void setSec_ccf(String sec_ccf) {
		this.sec_ccf = sec_ccf;
	}

	public String getSec_ecf() {
		return sec_ecf;
	}

	public void setSec_ecf(String sec_ecf) {
		this.sec_ecf = sec_ecf;
	}

	public String getDeleteActivation() {
		return deleteActivation;
	}

	public void setDeleteActivation(String deleteActivation) {
		this.deleteActivation = deleteActivation;
	}
	
}
