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

import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.op.IMPI_IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
import de.fhg.fokus.hss.web.form.DeleteForm;
import de.fhg.fokus.hss.db.hibernate.*;
import de.fhg.fokus.hss.web.util.WebConstants;

import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.hibernate.Session;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */


public class IMPU_Delete extends Action {
	
	public ActionForward execute(ActionMapping mapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		try{
			DeleteForm form = (DeleteForm) actionForm;
			HibernateUtil.beginTransaction();
			Session session = HibernateUtil.getCurrentSession();
			int id_impu = form.getId();

			// delete from IMPI_IMPU & IMPU
			IMPI_IMPU_DAO.delete_by_IMPU_ID(session, id_impu);
			IMPU_DAO.delete_by_ID(session, id_impu);
		} 
		finally{
			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
		}
		return mapping.findForward(WebConstants.FORWARD_SUCCESS);
	}
}
