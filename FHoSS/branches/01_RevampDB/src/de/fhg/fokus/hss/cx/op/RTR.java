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
package de.fhg.fokus.hss.cx.op;

import java.util.Iterator;
import java.util.List;

import org.hibernate.Session;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.hss.cx.CxConstants;
import de.fhg.fokus.hss.cx.CxFinalResultException;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.op.DB_Op;
import de.fhg.fokus.hss.db.op.IMPI_IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMSU_DAO;
import de.fhg.fokus.hss.diam.DiameterConstants;
import de.fhg.fokus.hss.diam.UtilAVP;
import de.fhg.fokus.hss.db.hibernate.*;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class RTR {

	public static DiameterMessage prepareRequest(DiameterPeer diameterPeer,  
			List impuList, List impiList, int reasonCode, String reasonInfo){
		
		DiameterMessage request = diameterPeer.newRequest(DiameterConstants.Command.RTR, DiameterConstants.Application.Cx);
		// add vendor-specific-application-id & auth-session state
		UtilAVP.addAuthSessionState(request, DiameterConstants.AVPValue.ASS_No_State_Maintained);
		UtilAVP.addVendorSpecificApplicationID(request, DiameterConstants.Vendor.V3GPP, DiameterConstants.Application.Cx);

		Session session = null;
		
		try{
			session = HibernateUtil.getCurrentSession();
			HibernateUtil.beginTransaction();

			if (impiList == null || impiList.size() == 0){
				throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY);
			}
			IMPI defaultIMPI = (IMPI) impiList.get(0);
			
			// add destination host and realm
			String dest_host = IMSU_DAO.get_Diameter_Name_by_IMSU_ID(session, defaultIMPI.getId_imsu());
			
			UtilAVP.addDestinationHost(request, dest_host);
			UtilAVP.addDestinationRealm(request, dest_host.substring(dest_host.indexOf('.') + 1));
			
			// add user name
			UtilAVP.addUserName(request, defaultIMPI.getIdentity());
			
			if (impuList == null || impuList.size() == 0){
				if (reasonCode == CxConstants.Deregistration_Reason_New_Server_Assigned){
					throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY);
				}
				Iterator it1 = impiList.iterator();
				IMPI impi = null;
				while (it1.hasNext()){
					impi = (IMPI) it1.next();
					impuList = IMPI_IMPU_DAO.get_all_IMPU_by_IMPI_ID(session, impi.getId());
					IMPU impu = null;
					Iterator it = impuList.iterator();
					while (it.hasNext()){
						impu = (IMPU) it.next();
						int user_state = impu.getUser_state();

						switch (user_state){
							case CxConstants.IMPU_user_state_Registered:
								int reg_cnt = IMPI_IMPU_DAO.get_Registered_IMPU_Count(session, impu.getId());
								if (reg_cnt == 1){
									// set the user_state to Not-Registered
									DB_Op.setUserState(session, defaultIMPI.getId(), impu.getId_implicit_set(), 
											CxConstants.IMPU_user_state_Not_Registered, true);
									// 	clear the scscf_name & origin_host
									IMSU_DAO.update(session, defaultIMPI.getId_imsu(), "", "");
								}
								else{
									// Set the user_state to Not-Registered only on IMPI_IMPU association, 
									//IMPU registration state remain registered
									DB_Op.setUserState(session, defaultIMPI.getId(), impu.getId_implicit_set(), 
											CxConstants.IMPU_user_state_Not_Registered, false);
								}
								break;
								
							case CxConstants.IMPU_user_state_Unregistered:
								// set the user_state to Not-Registered
								DB_Op.setUserState(session, defaultIMPI.getId(), impu.getId_implicit_set(), 
										CxConstants.IMPU_user_state_Not_Registered, true);
								// clear the scscf_name & origin_host
								IMSU_DAO.update(session, defaultIMPI.getId_imsu(), "", "");
								break;
						}
					}
					UtilAVP.addAsssociatedIdentities(request, impiList);
				}
			}
			else{
			
				IMPU impu = null;
				Iterator it = impuList.iterator();
				while (it.hasNext()){
					impu = (IMPU) it.next();
					int user_state = impu.getUser_state();

					switch (user_state){
						case CxConstants.IMPU_user_state_Registered:
							int reg_cnt = IMPI_IMPU_DAO.get_Registered_IMPU_Count(session, impu.getId());
							if (reg_cnt == 1){
								// set the user_state to Not-Registered
								DB_Op.setUserState(session, defaultIMPI.getId(), impu.getId_implicit_set(), 
										CxConstants.IMPU_user_state_Not_Registered, true);
								// clear the scscf_name & origin_host
								IMSU_DAO.update(session, defaultIMPI.getId_imsu(), "", "");
							}
							else{
								// Set the user_state to Not-Registered only on IMPI_IMPU association, 
								//IMPU registration state remain registered
								DB_Op.setUserState(session, defaultIMPI.getId(), impu.getId_implicit_set(), 
										CxConstants.IMPU_user_state_Not_Registered, false);
							}
							break;
							
						case CxConstants.IMPU_user_state_Unregistered:
							// set the user_state to Not-Registered
							DB_Op.setUserState(session, defaultIMPI.getId(), impu.getId_implicit_set(), 
									CxConstants.IMPU_user_state_Not_Registered, true);
							// clear the scscf_name & origin_host
							IMSU_DAO.update(session, defaultIMPI.getId_imsu(), "", "");
							break;
					}
					// add public identities
					UtilAVP.addPublicIdentity(request, impu.getIdentity());
				}
			}
			// add deregistration reason
			UtilAVP.addDeregistrationReason(request, reasonCode, reasonInfo);
			// add result code
			UtilAVP.addResultCode(request, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
		}
/*		catch(CxExperimentalResultException e){
			
		}*/
		catch(CxFinalResultException e){
			UtilAVP.addResultCode(request, e.getErrorCode());
			e.printStackTrace();
		}
		finally{
			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
		}
		
		return request;
	}
	
	public static DiameterMessage processRequest(DiameterPeer diameterPeer, DiameterMessage request){
		DiameterMessage response = diameterPeer.newResponse(request);
		
		return response;
	}
}
