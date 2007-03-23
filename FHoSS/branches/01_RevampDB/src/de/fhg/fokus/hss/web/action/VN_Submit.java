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

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.log4j.Logger;
import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.VisitedNetwork;
import de.fhg.fokus.hss.db.op.VisitedNetwork_DAO;
import de.fhg.fokus.hss.db.hibernate.*;
import de.fhg.fokus.hss.web.form.VN_Form;
import de.fhg.fokus.hss.web.util.WebConstants;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */


public class VN_Submit extends Action{
	
	private static Logger logger = Logger.getLogger(AS_Submit.class);
	
	public ActionForward execute(ActionMapping actionMapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		String action = request.getParameter("action");
		VN_Form form = (VN_Form) actionForm;
		String nextAction = form.getNextAction();
		ActionForward forward = null;
		int id = form.getId();

		try{
			HibernateUtil.beginTransaction();
			Session session = HibernateUtil.getCurrentSession();
					
			if (id != -1){
				if (VN_Load.testForDelete(session, id)){
					request.setAttribute("deleteDeactivation", "false");
				}
				else{
					request.setAttribute("deleteDeactivation", "true");
				}
			}			
			
			if (nextAction.equals("save")){
				VisitedNetwork visited_network;

				if (id == -1){
					// create
					visited_network = new VisitedNetwork();
				}	
				else{
					// update
					visited_network = VisitedNetwork_DAO.get_by_ID(session, id);
				}	
				
				// make the changes
				visited_network.setIdentity(form.getIdentity());
				
				if (id == -1){
					VisitedNetwork_DAO.insert(session, visited_network);
					id = visited_network.getId();
					form.setId(id);
				}
				else{
					VisitedNetwork_DAO.update(session, visited_network);
				}
				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward =  new ActionForward(forward.getPath() + "?id=" + id);
				
			}
			else if (nextAction.equals("refresh")){
				VisitedNetwork visited_network = (VisitedNetwork) VisitedNetwork_DAO.get_by_ID(session, id);

				if (!VN_Load.setForm(form, visited_network)){
					logger.error("The VisitedNetwork withe the ID:" + id + " was not loaded from database!");
				}
				forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward =  new ActionForward(forward.getPath() + "?id=" + id);
				
			}
			else if (nextAction.equals("delete")){
				VisitedNetwork_DAO.delete_by_ID(session, id);
				forward = actionMapping.findForward(WebConstants.FORWARD_DELETE);
			}
		}
		catch(DatabaseException e){
			forward = actionMapping.findForward(WebConstants.FORWARD_FAILURE);
		}
		finally{
			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
		}
		return forward;
	}
}
