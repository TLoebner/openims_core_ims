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
		
		String action = request.getParameter("action");
		IMPI_Form form = (IMPI_Form) actionForm;
		String nextAction = form.getNextAction();
		
		int id = form.getId();

		HibernateUtil.beginTransaction();
		Session session = HibernateUtil.getCurrentSession();
					
		if (nextAction.equals("save")){
			int auth_scheme;
			if (id == -1){
				// create
				auth_scheme = IMPI.generateAuthScheme(form.isAka1(), form.isAka2(), form.isMd5(), 
						form.isEarly(), form.isAll());	
				
				IMSU imsu = IMSU_DAO.get_by_ID(session, form.getId_imsu());
				IMPI impi = IMPI_DAO.insert(session, form.getIdentity(), form.getSecretKey(), auth_scheme, form.getIp(), 
						HexCodec.decode(form.getAmf()), HexCodec.decode(form.getOp()), new String (HexCodec.decode(form.getSqn())),
						imsu.getId());
				form.setId(impi.getId());
			}
			else{
				// update
				 auth_scheme = IMPI.generateAuthScheme(form.isAka1(), form.isAka2(), form.isMd5(), 
						form.isEarly(), form.isAll());				
				
				IMSU imsu = IMSU_DAO.get_by_ID(session, form.getId_imsu());
				IMPI_DAO.update(session, form.getId(), form.getIdentity(), form.getSecretKey(), auth_scheme, form.getIp(),
						HexCodec.decode(form.getAmf()), HexCodec.decode(form.getOp()), new String (HexCodec.decode(form.getSqn())),
						imsu.getId());
			}	
			
			if ((auth_scheme & 15) == 15){
				form.setAll(true);
				form.setAka1(false);
				form.setAka2(false);
				form.setMd5(false);
				form.setEarly(false);
			}
		}
		else if (nextAction.equals("refresh")){
			IMPI impi = (IMPI) IMPI_DAO.get_by_ID(session, id);
			if (impi != null){
				form.setIdentity(impi.getIdentity());
				form.setSecretKey(impi.getK());
				int auth_scheme = impi.getAuth_scheme();
				if ((auth_scheme & 15) == 15){
					form.setAll(true);
					form.setAka1(false);
					form.setAka2(false);
					form.setMd5(false);
					form.setEarly(false);
				}
				else{
					if ((auth_scheme & 1) == 1){
						form.setAka1(true);
					}
					if ((auth_scheme & 2) == 1){
						form.setAka2(true);
					}
					if ((auth_scheme & 4) == 1){
						form.setMd5(true);
					}
					if ((auth_scheme & 8) == 1){
						form.setEarly(true);
					}
				}
				form.setIp(impi.getIp());
				
				form.setAmf(HexCodec.encode(impi.getAmf()));
				form.setOp(HexCodec.encode(impi.getOp()));
				form.setSqn(HexCodec.encode(impi.getSqn()));
				
				form.setId_imsu(impi.getId_imsu());
			}
		}
		else if (nextAction.equals("add_impu")){
			IMPU impu = IMPU_DAO.get_by_Identity(session, form.getImpu_identity());	
			if (impu != null){
				IMPI impi = IMPI_DAO.get_by_ID(session, form.getId());
				IMPI_IMPU_DAO.insert(session, impi.getId(), impu.getId(), (short)0);
			}
			else{
				ActionMessages actionMessages = new ActionMessages();
				actionMessages.add(Globals.MESSAGE_KEY, new ActionMessage("impi.error.associated_impu_not_found"));
				saveMessages(request, actionMessages);
			}
		}
		else if (nextAction.equals("delete")){
			System.out.println("Delete!!!!!!!!!!");
		}
		else if (nextAction.equals("delete2")){
			System.out.println("Delete2222222!!!!!!!!!!");
		}
		
		//reload the imsuList
		List imsuList= IMSU_DAO.get_all(session);		
		form.setSelect_imsu(imsuList);
		
		// reload the associated IMPUs
		IMPI impi = IMPI_DAO.get_by_ID(session, id);
		List list = IMPI_IMPU_DAO.get_all_IMPU_by_IMPI_ID(session, id);
		form.setAssociated_impu_set(list);

		HibernateUtil.commitTransaction();
		HibernateUtil.closeSession();

		if (nextAction.equals("ppr")){
			System.out.println("We have the ppr here!");
		}
		else if (nextAction.equals("rtr")){
			System.out.println("We have the rtr here!");
			session = HibernateUtil.getCurrentSession();
			HibernateUtil.beginTransaction();
			DiameterStack stack = HSSContainer.getInstance().diamStack;
			impi = IMPI_DAO.get_by_ID(session, form.getId());
			List impiList = new ArrayList();
			impiList.add(impi);
			RTR.sendRequest(stack.diameterPeer, null, impiList,
					CxConstants.Deregistration_Reason_Permanent_Termination, "permanent termination");
		}		
		
		ActionForward forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
		forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
		return forward;
	}
}
