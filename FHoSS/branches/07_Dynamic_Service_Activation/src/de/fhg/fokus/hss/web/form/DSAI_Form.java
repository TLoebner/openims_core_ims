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

import org.apache.log4j.Logger;
import org.apache.struts.action.ActionErrors;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionMapping;
import org.apache.struts.action.ActionMessage;
import org.hibernate.HibernateException;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.hibernate.DatabaseException;
import de.fhg.fokus.hss.db.hibernate.HibernateUtil;
import de.fhg.fokus.hss.db.model.SP;
import de.fhg.fokus.hss.db.model.SP_IFC;
import de.fhg.fokus.hss.db.model.SP_Shared_IFC_Set;
import de.fhg.fokus.hss.db.op.SP_DAO;
import de.fhg.fokus.hss.db.op.SP_IFC_DAO;
import de.fhg.fokus.hss.db.op.SP_Shared_IFC_Set_DAO;

import java.io.Serializable;
import java.util.List;

import javax.servlet.http.HttpServletRequest;

/**
 * @author inycom.es
 */


public class DSAI_Form extends ActionForm implements Serializable{
	private static Logger logger = Logger.getLogger(SP_Form.class);
	private static final long serialVersionUID=1L;

	private int id;
	private String dsai_tag;

	private int ifc_id;
	private int impu_id;
	private List select_ifc;
	private List select_impu;
	private String nextAction;

	public void reset(ActionMapping actionMapping, HttpServletRequest request){
    	this.id = -1;
    	this.dsai_tag = null;

    	this.ifc_id = -1;
    	this.impu_id = -1;
    	this.select_ifc = null;
    	this.select_impu = null;
    	this.nextAction = null;
    }

    public ActionErrors validate(ActionMapping actionMapping, HttpServletRequest request){
        ActionErrors actionErrors = new ActionErrors();

    	boolean dbException = false;
    	try{
    		Session session = HibernateUtil.getCurrentSession();
    		HibernateUtil.beginTransaction();

    		//#### TO DO ####
    	}
    	catch(DatabaseException e){
    		logger.error("Database Exception occured!\nReason:" + e.getMessage());
    		e.printStackTrace();
    		dbException = true;
    	}
    	catch (HibernateException e){
    		logger.error("Hibernate Exception occured!\nReason:" + e.getMessage());
    		e.printStackTrace();
    		dbException = true;
    	}
    	finally{
    		if (!dbException){
    			HibernateUtil.commitTransaction();
    		}
    		HibernateUtil.closeSession();
    	}

        return actionErrors;
    }

    // getters & setters
	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public String getDsai_tag() {
		return dsai_tag;
	}

	public void setDsai_tag(String dsai_tag) {
		this.dsai_tag = dsai_tag;
	}

	public String getNextAction() {
		return nextAction;
	}

	public void setNextAction(String nextAction) {
		this.nextAction = nextAction;
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

	public List getSelect_impu() {
		return select_impu;
	}

	public void setSelect_impu(List select_impu) {
		this.select_impu = select_impu;
	}

	public int getImpu_id() {
		return impu_id;
	}

	public void setImpu_id(int impu_id) {
		this.impu_id = impu_id;
	}

}
