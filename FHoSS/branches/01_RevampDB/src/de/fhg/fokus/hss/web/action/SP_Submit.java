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

import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.hibernate.Session;


import de.fhg.fokus.hss.db.model.SP;
import de.fhg.fokus.hss.db.op.SP_DAO;
import de.fhg.fokus.hss.db.hibernate.*;
import de.fhg.fokus.hss.web.form.SP_Form;
import de.fhg.fokus.hss.web.util.WebConstants;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */


public class SP_Submit extends Action{
	
	public ActionForward execute(ActionMapping actionMapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		String action = request.getParameter("action");
		SP_Form form = (SP_Form) actionForm;
		String nextAction = form.getNextAction();
		int id = form.getId();
		ActionForward forward = null;
		
		try{
			HibernateUtil.beginTransaction();
			Session session = HibernateUtil.getCurrentSession();
					
			if (id != -1){
				if (SP_Load.testForDelete(session, id)){
					request.setAttribute("deleteDeactivation", "false");
				}
				else{
					request.setAttribute("deleteDeactivation", "true");
				}
			}			
		
			if (nextAction.equals("save")){
				if (id == -1){
					// create
					SP sp = SP_DAO.insert(session, form.getName(), form.getCn_service_auth());
					form.setId(sp.getId());
				}
				else{
					// update
					SP sp = SP_DAO.update(session, form.getId(), form.getName(), form.getCn_service_auth());
				}	
				forward  = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() + "?id=" + id);
			}
			else if (nextAction.equals("refresh")){
				SP sp = (SP) SP_DAO.get_by_ID(session, id);
				if (sp != null){
					form.setName(sp.getName());
					form.setCn_service_auth(sp.getCn_service_auth());
				}
				forward  = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
				forward =  new ActionForward(forward.getPath() + "?id=" + id);
			}
			else if (nextAction.equals("delete")){
				SP_DAO.delete_by_ID(session, id);
				forward  = actionMapping.findForward(WebConstants.FORWARD_DELETE);
			}
		}
		catch(DatabaseException e){
			e.printStackTrace();
		}
		finally{
			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
		}
		return forward;
	}
}
