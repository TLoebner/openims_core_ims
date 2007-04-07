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

import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import java.util.*;

import org.apache.struts.Globals;
import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.apache.struts.action.ActionMessage;
import org.apache.struts.action.ActionMessages;
import org.hibernate.Session;


import de.fhg.fokus.hss.auth.HexCodec;
import de.fhg.fokus.hss.cx.CxConstants;
import de.fhg.fokus.hss.cx.op.RTR;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.IMSU;
import de.fhg.fokus.hss.db.op.IMPI_DAO;
import de.fhg.fokus.hss.db.op.IMPI_IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMSU_DAO;
import de.fhg.fokus.hss.main.HSSContainer;
import de.fhg.fokus.hss.db.hibernate.*;
import de.fhg.fokus.hss.web.form.IMPI_Form;
import de.fhg.fokus.hss.web.util.WebConstants;
import de.fhg.fokus.hss.diam.*;


/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class IMPI_Submit extends Action{
	
	public ActionForward execute(ActionMapping actionMapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		// get the input parameters
		
		String test = (String) request.getAttribute("associated_IMPU_ID");
		
		IMPI_Form form = (IMPI_Form) actionForm;
		String nextAction = form.getNextAction();
		ActionForward forward = null;
		int id = form.getId();
		
		try{
			HibernateUtil.beginTransaction();
			Session session = HibernateUtil.getCurrentSession();

			if (nextAction.equals("save")){
				int auth_scheme;
				IMPI impi;
				
				if (id == -1){
					// create
					impi = new IMPI();
				}
				else{
					// update
					impi = IMPI_DAO.get_by_ID(session, id);
				}
				auth_scheme = IMPI.generateAuthScheme(form.isAka1(), form.isAka2(), form.isMd5(), form.isDigest(), form.isHttp_digest(),
					form.isEarly(), form.isNass_bundle(), form.isAll());	
				
				impi.setIdentity(form.getIdentity());
				impi.setK(form.getSecretKey());
				impi.setAuth_scheme(auth_scheme);
				impi.setIp(form.getIp());
				impi.setAmf(HexCodec.decode(form.getAmf()));
				impi.setOp(HexCodec.decode(form.getOp()));
				impi.setSqn(new String(HexCodec.decode(form.getSqn())));
				impi.setId_imsu(form.getId_imsu());
				//impi.setDefault_auth_scheme()
				impi.setLine_identifier(form.getLine_identifier());

				if (id == -1){
					if (form.getAlready_assigned_imsu_id() > 0){
						impi.setId_imsu(form.getAlready_assigned_imsu_id());
						form.setAlready_assigned_imsu_id(-1);
					}
					
					IMPI_DAO.insert(session, impi);
					id = impi.getId();
					form.setId(id);
				}
				else{
					IMPI_DAO.update(session, impi);
				}
				
				if ((auth_scheme & 127) == 127){
					form.setAll(true);
					form.setAka1(false);
					form.setAka2(false);
					form.setMd5(false);
					form.setDigest(false);
					form.setHttp_digest(false);
					form.setEarly(false);
					form.setNass_bundle(false);
				}
				
				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
				
			}
			else if (nextAction.equals("refresh")){
				IMPI impi = (IMPI) IMPI_DAO.get_by_ID(session, id);
				//List associated_IMPUs = IMPI_IMPU_DAO.get_all_IMPU_by_IMPI_ID(session, id);
				
				IMPI_Load.setForm(form, impi);
				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
				
			}
			else if (nextAction.equals("add_impu")){
				IMPU impu = IMPU_DAO.get_by_Identity(session, form.getImpu_identity());	
				if (impu != null){
					IMPI_IMPU_DAO.insert(session, form.getId(), impu.getId(), CxConstants.IMPU_user_state_Not_Registered);
				}
				else{
					ActionMessages actionMessages = new ActionMessages();
					actionMessages.add(Globals.MESSAGE_KEY, new ActionMessage("impi.error.associated_impu_not_found"));
					saveMessages(request, actionMessages);
				}
				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
				
				// reload the associated IMPUs
//				List list = IMPI_IMPU_DAO.get_all_IMPU_by_IMPI_ID(session, id);
//				form.setAssociated_impu_set(list);
			}
			else if (nextAction.equals("add_imsu")){
				IMPI impi = (IMPI) IMPI_DAO.get_by_ID(session, id);
				IMSU imsu = IMSU_DAO.get_by_Name(session, form.getImsu_name());
				
				if (imsu != null){
					impi.setId_imsu(imsu.getId());
					IMPI_DAO.update(session, impi);
				}
				else{
					ActionMessages actionMessages = new ActionMessages();
					actionMessages.add(Globals.MESSAGE_KEY, new ActionMessage("impi.error.associated_imsu_not_found"));
					saveMessages(request, actionMessages);
				}
				
				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
			}
			else if (nextAction.equals("delete")){
				IMPI_DAO.delete_by_ID(session, form.getId());
				forward = actionMapping.findForward(WebConstants.FORWARD_DELETE);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
			}
			else if (nextAction.equals("delete_associated_IMSU")){
				IMPI impi = IMPI_DAO.get_by_ID(session, id);
				impi.setId_imsu(null);
				IMPI_DAO.update(session, impi);
				
				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
			}
			
			else if (nextAction.equals("delete_associated_IMPU")){
				IMPI_IMPU_DAO.delete_by_IMPI_and_IMPU_ID(session, id, form.getAssociated_ID());
				
				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
			}
		
			// reload the associated IMPUs
			List list = IMPI_IMPU_DAO.get_all_IMPU_by_IMPI_ID(session, id);
			if (list == null){
				list = new ArrayList();
			}
			request.setAttribute("associated_IMPUs", list);
			IMSU associated_IMSU = null;

			if (id != -1){
				IMPI impi = IMPI_DAO.get_by_ID(session, id);
				if (impi != null)
					associated_IMSU = IMSU_DAO.get_by_ID(session, impi.getId_imsu());
			}
			request.setAttribute("associated_IMSU", associated_IMSU);
	    	form.setSelect_auth_scheme(WebConstants.select_auth_scheme);
	    	
			if (IMPI_Load.testForDelete(session, form.getId())){
				request.setAttribute("deleteDeactivation", "false");
			}
			else{
				request.setAttribute("deleteDeactivation", "true");
			}
	    	
		}
		catch(DatabaseException e){
			e.printStackTrace();
			forward = actionMapping.findForward(WebConstants.FORWARD_FAILURE);
			return forward;
		}
		finally{
			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
		}
		
		if (nextAction.equals("ppr")){
			System.out.println("We have the ppr here!");
			forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
			forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
		}
		else if (nextAction.equals("rtr")){
			System.out.println("We have the rtr here!");
			
			Session session = HibernateUtil.getCurrentSession();
			HibernateUtil.beginTransaction();
			
			DiameterStack stack = HSSContainer.getInstance().diamStack;
			IMPI impi = IMPI_DAO.get_by_ID(session, form.getId());
			List impiList = new ArrayList();
			impiList.add(impi);
			RTR.sendRequest(stack.diameterPeer, null, impiList,
					CxConstants.Deregistration_Reason_Permanent_Termination, "permanent termination");
			forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
			forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
		}		
		
		
		return forward;
	}
}
