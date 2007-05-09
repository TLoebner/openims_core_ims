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

import org.apache.struts.Globals;
import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.apache.struts.action.ActionMessage;
import org.apache.struts.action.ActionMessages;
import org.hibernate.Session;


import de.fhg.fokus.hss.cx.CxConstants;
import de.fhg.fokus.hss.cx.op.RTR;
import de.fhg.fokus.hss.db.model.ChargingInfo;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.IMPU_VisitedNetwork;
import de.fhg.fokus.hss.db.model.SP;
import de.fhg.fokus.hss.db.model.VisitedNetwork;
import de.fhg.fokus.hss.db.op.ChargingInfo_DAO;
import de.fhg.fokus.hss.db.op.IMPI_DAO;
import de.fhg.fokus.hss.db.op.IMPI_IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMPU_VisitedNetwork_DAO;
import de.fhg.fokus.hss.db.op.SP_DAO;
import de.fhg.fokus.hss.db.op.VisitedNetwork_DAO;
import de.fhg.fokus.hss.db.hibernate.*;
import de.fhg.fokus.hss.diam.DiameterStack;
import de.fhg.fokus.hss.main.HSSContainer;
import de.fhg.fokus.hss.web.form.IMPU_Form;
import de.fhg.fokus.hss.web.util.WebConstants;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */


public class IMPU_Submit extends Action{
	
	public ActionForward execute(ActionMapping actionMapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		ActionForward forward = null;
		
		IMPU_Form form = (IMPU_Form) actionForm;
		String nextAction = form.getNextAction();
		int id = form.getId();
		
				
		try{
			Session session = HibernateUtil.getCurrentSession();
			HibernateUtil.beginTransaction();

			// refresh the SP and the Charging-Info list
			List sp_list = SP_DAO.get_all(session);
			form.setSelect_sp(sp_list);
			
			List chg_list = ChargingInfo_DAO.get_all(session);
			form.setSelect_charging_info(chg_list);
			
			List vn_list = VisitedNetwork_DAO.get_all(session);
			form.setSelect_vn(vn_list);

			// refresh the select_identity_type
			form.setSelect_identity_type(WebConstants.select_identity_type);		
			
			// apply the coresponding action
			if (nextAction.equals("save")){
				IMPU impu = null;
				if (id == -1){
					// create
					impu = new IMPU();
				}
				else{
					impu = IMPU_DAO.get_by_ID(session, id);
				}
			
				impu.setIdentity(form.getIdentity());
				if (form.isBarring()){
					impu.setBarring(1);
				}
				else{
					impu.setBarring(0);
				}
				impu.setType(form.getType());
				
				impu.setWildcard_psi(form.getWildcard_psi());
				impu.setDisplay_name(form.getDisplay_name());
			
				if (form.getPsi_activation()){
					impu.setPsi_activation(1);
				}
				else{
					impu.setPsi_activation(0);
				}
			
				if (form.getCan_register()){
					impu.setCan_register(1);
				}
				else{
					impu.setCan_register(0);
				}
			
				//SP
				SP sp = SP_DAO.get_by_ID(session, form.getId_sp());
				if (sp != null){
					impu.setId_sp(sp.getId());
				}
				
				ChargingInfo chg_info = ChargingInfo_DAO.get_by_ID(session, form.getId_charging_info());
				if (chg_info != null){
					impu.setId_charging_info(chg_info.getId());
				}
				
				if (id == -1){
					//create
					IMPU_DAO.insert(session, impu);
					impu.setId_implicit_set(impu.getId());
					IMPU_DAO.update(session, impu);
					
					if (form.getAlready_assigned_impi_id() > 0){
						IMPI impi = IMPI_DAO.get_by_ID(session, form.getAlready_assigned_impi_id());	
						if (impi != null){
							IMPI_IMPU_DAO.insert(session, impi.getId(), impu.getId(), CxConstants.IMPU_user_state_Not_Registered);
						}
						form.setAlready_assigned_impi_id(-1);
					}
					id = impu.getId();
					form.setId(id);
					
					form.setId_impu_implicitset(impu.getId_implicit_set());
					
				}
				else{
					//update
					IMPU_DAO.update(session, impu);
				}
				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());				
			}
			else if (nextAction.equals("refresh")){
				IMPU impu = (IMPU) IMPU_DAO.get_by_ID(session, id);
				IMPU_Load.setForm(form, impu);

				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
				
			}
			else if (nextAction.equals("delete")){
				IMPU_DAO.delete_by_ID(session, id);
				forward = actionMapping.findForward(WebConstants.FORWARD_DELETE);
			}
			else if (nextAction.equals("delete_associated_impi")){
				System.out.println("ASS IMPI:" + form.getAssociated_ID());
				IMPI_IMPU_DAO.delete_by_IMPI_and_IMPU_ID(session, form.getAssociated_ID(), id);
				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
				
			}	
			else if (nextAction.equals("delete_impu_from_implicitset")){
				
				IMPU impu = IMPU_DAO.get_by_ID(session, form.getAssociated_ID());
				if (impu != null){
					if (impu.getId() == impu.getId_implicit_set()){
						IMPU_DAO.update_others_from_implicit_set_ID(session, impu.getId(), impu.getId_implicit_set());

						// change the set id to the default one (self)
						impu.setId_implicit_set(impu.getId());
						IMPU_DAO.update(session, impu);
					}
					else{
						// only, change the set id to the default one (self)
						impu.setId_implicit_set(impu.getId());
						IMPU_DAO.update(session, impu);
					}
				}
				
				// load the current IMPU and update the form
				impu = IMPU_DAO.get_by_ID(session, id);
				form.setId_impu_implicitset(impu.getId_implicit_set());
				
				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
			
			}	
			else if (nextAction.equals("delete_visited_network")){
				IMPU_DAO.delete_VisitedNetwork_for_IMPU(session, id, form.getAssociated_ID());
				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
			}
			
			else if (nextAction.equals("add_impi")){
				IMPI impi = IMPI_DAO.get_by_Identity(session, form.getImpi_identity());	
				if (impi != null){
					IMPI_IMPU_DAO.insert(session, impi.getId(), form.getId(), CxConstants.IMPU_user_state_Not_Registered);
				}
				else{
/*					ActionMessages actionMessages = new ActionMessages();
					actionMessages.add(Globals.MESSAGE_KEY, new ActionMessage("impu.error.associated_impu_not_found"));
					saveMessages(request, actionMessages);*/
				}
				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());			
			}
			else if (nextAction.equals("add_impu_to_implicitset")){
				IMPU crt_impu = IMPU_DAO.get_by_ID(session, id);
				IMPU impu = IMPU_DAO.get_by_Identity(session, form.getImpu_implicitset_identity());
				if (impu != null){
					//doar dc e posibil, de pus conditia
					impu.setId_implicit_set(crt_impu.getId_implicit_set());
					IMPU_DAO.update(session, impu);
				}
				else{
				/*	ActionMessages actionMessages = new ActionMessages();
					actionMessages.add(Globals.MESSAGE_KEY, new ActionMessage("impu.error.failed_add_impu_to_implicitset"));
					saveMessages(request, actionMessages);*/
				}

				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
			}
			else if (nextAction.equals("add_vn")){
				IMPU_VisitedNetwork impu_vn = new IMPU_VisitedNetwork();
				impu_vn.setId_impu(id);
				impu_vn.setId_visited_network(form.getVn_id());
				IMPU_VisitedNetwork_DAO.insert(session, impu_vn);

				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
				
			}
			
			// add parameters to request & refresh select properties
			if (IMPU_Load.testForDelete(session, form.getId())){
				request.setAttribute("deleteDeactivation", "false");
			}
			else{
				request.setAttribute("deleteDeactivation", "true");
			}
			
			IMPU crt_impu = IMPU_DAO.get_by_ID(session, id);
			if (crt_impu != null){
				List implicitset_IMPUs = IMPU_DAO.get_all_from_set(session, crt_impu.getId_implicit_set());
				request.setAttribute("implicitset_IMPUs", implicitset_IMPUs);
				
				List associated_IMPIs = IMPI_IMPU_DAO.get_all_IMPI_by_IMPU_ID(session, id);
				request.setAttribute("associated_IMPIs", associated_IMPIs);
				
				List visitedNetworks = IMPU_DAO.get_all_VisitedNetworks_by_IMPU_ID(session, id);
				request.setAttribute("visitedNetworks", visitedNetworks);
			}
						
			HibernateUtil.commitTransaction();
		}
		catch(DatabaseException e){
			e.printStackTrace();
			forward = actionMapping.findForward(WebConstants.FORWARD_FAILURE);
			forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
		}
		finally{
			HibernateUtil.closeSession();
		}
		
		if (nextAction.equals("ppr")){
			forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
			forward = new ActionForward(forward.getPath() +"?id=" + form.getId());

			System.out.println("We have the ppr here!");
		}
		else if (nextAction.equals("rtr")){
			
			Session session = HibernateUtil.getCurrentSession();
			HibernateUtil.beginTransaction();
			
			IMPI impi = IMPI_DAO.get_by_ID(session, form.getId());
			List impiList = new ArrayList();
			impiList.add(impi);
			HibernateUtil.commitTransaction();
			session.close();
			
			DiameterStack stack = HSSContainer.getInstance().diamStack;
			RTR.sendRequest(stack.diameterPeer, null, impiList,
					CxConstants.Deregistration_Reason_Permanent_Termination, "permanent termination");
			forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
			forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
			
			System.out.println("We have the rtr here!");
		}		
		
		
		
		return forward;
	}
}
