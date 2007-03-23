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

package de.fhg.fokus.hss.web.action;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.hibernate.Session;


import de.fhg.fokus.hss.cx.CxConstants;
import de.fhg.fokus.hss.db.model.ChargingInfo;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.SP;
import de.fhg.fokus.hss.db.hibernate.*;
import de.fhg.fokus.hss.web.form.IMPU_Form;
import de.fhg.fokus.hss.web.util.WebConstants;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */


public class IMPU_Submit extends Action{
	
	public ActionForward execute(ActionMapping actionMapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		IMPU_Form form = (IMPU_Form) actionForm;
		String nextAction = form.getNextAction();
		int id = form.getId();
		
		HibernateUtil.beginTransaction();
		Session session = HibernateUtil.getCurrentSession();
		
		// refresh the SP and the Charging-Info list
		List list;
		list = session.createCriteria(SP.class).list();
		form.setSelect_sp(list);
		list = session.createCriteria(ChargingInfo.class).list();
		form.setSelect_charging_info(list);

		// refresh the select_identity_type
		List select_identity_type = new ArrayList();
		select_identity_type.add(CxConstants.Identity_Type.Public_User_Identity);
		select_identity_type.add(CxConstants.Identity_Type.Wildcarded_PSI);
		select_identity_type.add(CxConstants.Identity_Type.Distinct_PSI);
		form.setSelect_identity_type(select_identity_type);
		
		
		
		
		if (nextAction.equals("save")){
			if (id == -1){
				// create
				IMPU impu = new IMPU();
				impu.setIdentity(form.getIdentity());
				impu.setType(0);
				if (form.isBarring()){
					impu.setBarring(1);
				}
				else{
					impu.setBarring(0);
				}
								
				impu.setId_impu_implicitset(0);
				impu.setWildcard_psi("");
				impu.setDisplay_name("");
				
				Iterator it = form.getSelect_sp().iterator();
				while (it.hasNext()){
					SP sp = (SP) it.next();
					if (sp.getId()== form.getId_sp()){
						impu.setId_sp(sp.getId());
						break;
					}
				}
				it = form.getSelect_charging_info().iterator();
				while (it.hasNext()){
					ChargingInfo chargingInfo = (ChargingInfo) it.next();
					if (chargingInfo.getId()== form.getId_charging_info()){
						impu.setId_charging_info(chargingInfo.getId());
						break;
					}
				}

				session.save(impu);
				impu.setId_impu_implicitset(impu.getId());
				session.saveOrUpdate(impu);
			
				form.setId(impu.getId());
				form.setId_impu_implicitset(impu.getId_impu_implicitset());
			}
			else{
				// update
				IMPU impu = (IMPU)session.load(IMPU.class, form.getId());
				impu.setIdentity(form.getIdentity());
				impu.setType(0);
				if (form.isBarring()){
					impu.setBarring(1);
				}
				else{
					impu.setBarring(0);
				}
				Iterator it = form.getSelect_sp().iterator();
				while (it.hasNext()){
					SP sp = (SP) it.next();
					if (sp.getId()== form.getId_sp()){
						impu.setId_sp(sp.getId());
						break;
					}
				}
				it = form.getSelect_charging_info().iterator();
				while (it.hasNext()){
					ChargingInfo chargingInfo = (ChargingInfo) it.next();
					if (chargingInfo.getId()== form.getId_charging_info()){
						impu.setId_charging_info(chargingInfo.getId());
						break;
					}
				}
				impu.setId_impu_implicitset(form.getId_impu_implicitset());
				impu.setWildcard_psi("");
				impu.setDisplay_name("");
				session.saveOrUpdate(impu);
			}
			
			//System.out.println("We have the save here!");
		}
		else if (nextAction.equals("refresh")){
			
			IMPU impu = (IMPU) session.load(IMPU.class, form.getId());

			if (impu != null){
				form.set_Id_sp(impu.getId_sp());
				
				if (impu.getBarring() == 1){
					form.setBarring(true);
				}
				else{
					form.setBarring(false);
				}
				
				form.setIdentity(impu.getIdentity());
				form.setId_charging_info(impu.getId_charging_info());
				form.setId_impu_implicitset(impu.getId_impu_implicitset());
			}
			System.out.println("We have the refresh here!");
		}
		else if (nextAction.equals("ppr")){
			System.out.println("We have the ppr here!");
		}
		else if (nextAction.equals("rtr")){
			System.out.println("We have the rtr here!");
		}		

		HibernateUtil.commitTransaction();
		HibernateUtil.closeSession();
		
		ActionForward forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
		forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
		return forward;
	}
}
