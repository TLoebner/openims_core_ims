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

import java.util.LinkedList;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.hibernate.Session;


import de.fhg.fokus.hss.db.model.CapabilitiesSet;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMSU;
import de.fhg.fokus.hss.db.model.PreferredSet;
import de.fhg.fokus.hss.db.op.CapabilitiesSet_DAO;
import de.fhg.fokus.hss.db.op.PreferredSet_DAO;
import de.fhg.fokus.hss.db.op.IMSU_DAO;
import de.fhg.fokus.hss.db.hibernate.*;
import de.fhg.fokus.hss.web.form.IMSU_Form;
import de.fhg.fokus.hss.web.util.WebConstants;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */


public class IMSU_Load extends Action {
	
	public ActionForward execute(ActionMapping actionMapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		IMSU_Form form = (IMSU_Form) actionForm;
		int id = form.getId();
		Session session = null;
		try{
			List l1;
			HibernateUtil.beginTransaction();
			session = HibernateUtil.getCurrentSession();
			
			l1 = CapabilitiesSet_DAO.get_all_sets(session);
			form.setSelect_capabilities_set(l1);
			
			l1 = PreferredSet_DAO.get_all_sets(session);
			form.setSelect_preferred_scscf(l1);
		
			if (id != -1) {
				// load
				IMSU imsu = IMSU_DAO.get_by_ID(session, id);
				IMSU_Load.setForm(form, imsu);
			}
		}
		catch(DatabaseException e){
			e.printStackTrace();
		}
		finally{
			HibernateUtil.commitTransaction();
			session.close();
		}
		
		ActionForward forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
		forward = new ActionForward(forward.getPath() + "?id=" + id);
		return forward;
	}
	
	public static boolean setForm(IMSU_Form form, IMSU imsu){
		boolean exitCode = false;
		
		if (imsu != null){
			exitCode = true;
			form.setDiameter_name(imsu.getDiameter_name());
			form.setScscf_name(imsu.getScscf_name());
			form.setName(imsu.getName());
			form.setId_capabilities_set(imsu.getId_capabilities_set());
			form.setId_preferred_scscf(imsu.getId_preferred_scscf());
		}
		return exitCode;
	}	

	public static boolean testForDelete(Session session, int id){
		return true;
	}
}
