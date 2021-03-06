/*
 * $Id$
 *
 * Copyright (C) 2004-2006 FhG Fokus
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
package de.fhg.fokus.hss.action;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.log4j.Logger;
import org.apache.struts.Globals;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.apache.struts.action.ActionMessage;
import org.apache.struts.action.ActionMessages;
import org.hibernate.LockMode;

import de.fhg.fokus.hss.form.ImpiForm;
import de.fhg.fokus.hss.model.Chrginfo;
import de.fhg.fokus.hss.model.Impi;
import de.fhg.fokus.hss.model.ImpiBO;
import de.fhg.fokus.hss.model.Imsu;
import de.fhg.fokus.hss.util.HibernateUtil;

/**
 * @author Andre Charton(dev -at- open-ims dot org)
 */
public class ImpiSubmitAction extends HssAction
{
	private static final Logger LOGGER = Logger
			.getLogger(ImpuSubmitAction.class);

	public ActionForward execute(ActionMapping mapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse)
			throws Exception{
		
		LOGGER.debug("entering");

		ImpiForm form = (ImpiForm) actionForm;
		LOGGER.debug(form);

		ActionMessages actionMessages = null;
		ActionForward forward = null;
		
		Impi impi = null;
		ImpiBO impiBO = new ImpiBO();
		try{
			Integer primaryKey = form.getPrimaryKey();
			HibernateUtil.beginTransaction();
			// if pk id = -1 create a new impi and concat them to current imsu
			if (primaryKey.intValue() == -1){
				// check for existing impi with the same impi string
				if (((Integer) HibernateUtil.getCurrentSession()
						.createQuery("select count(impi) from de.fhg.fokus.hss.model.Impi as impi where impi.impiString = ?")
						.setString(0, form.getImpiString()).uniqueResult())
						.intValue() == 0){
					
					impi = new Impi();

					// store assigned imsu if exists
					if (form.getImsuId() != null){
						Integer assignedSubscriptionId = new Integer(form.getImsuId());
						impi.setImsu((Imsu) HibernateUtil.getCurrentSession().load(Imsu.class, assignedSubscriptionId, LockMode.READ));
					}
				}
				else{
					actionMessages = new ActionMessages();
					actionMessages.add(Globals.MESSAGE_KEY, new ActionMessage("impi.error.duplicated"));
					saveMessages(request, actionMessages);
					forward = mapping.findForward(FORWARD_FAILURE);
				}
			} 
			else{
				impi = impiBO.load(primaryKey);
			}

			// on errors dont store anything, forward to error page.
			if (forward == null){
				copyValues(form, impi);
				impiBO.saveOrUpdate(impi);
				forward = mapping.findForward(FORWARD_SUCCESS);
				forward = new ActionForward(forward.getPath() + "?impiId="+ impi.getImpiId(), true);
			}
			
			HibernateUtil.commitTransaction();
		} 
		finally{
			HibernateUtil.closeSession();
		}


		LOGGER.debug("exiting");

		return forward;
	}

	/**
	 * CopyValues
	 * @param form
	 * @param impi
	 */
	private void copyValues(ImpiForm form, Impi impi){
		
		impi.setImsi(form.getImsi());
		impi.setImpiString(form.getImpiString());
		impi.setScscfName(form.getScscfName());

		impi.setAuthScheme(form.getAuthScheme());
		impi.setSkey(form.getSkey());
		impi.setOperatorId(form.getOperatorId());
		impi.setAmf(form.getAmf());
		
		if (form.getSqnUpdate().equals(ImpiForm.SQD_UPDATE_TRUE)){
			impi.setSqn(form.getSqn());
		}

		impi.setChrginfo((Chrginfo) HibernateUtil.getCurrentSession().get(Chrginfo.class, Integer.valueOf(form.getChrgInfoId())));

		// TODO: Read this values
		impi.setUiccType(new Integer(1));
		impi.setKeyLifeTime(new Integer(3600));

	}
}
