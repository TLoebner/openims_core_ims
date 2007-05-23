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

import org.apache.log4j.Logger;
import org.hibernate.HibernateException;
import org.hibernate.Session;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.hss.cx.CxConstants;
import de.fhg.fokus.hss.db.hibernate.DatabaseException;
import de.fhg.fokus.hss.db.hibernate.HibernateUtil;
import de.fhg.fokus.hss.db.model.ChargingInfo;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.IMSU;
import de.fhg.fokus.hss.db.op.ChargingInfo_DAO;
import de.fhg.fokus.hss.db.op.IMPI_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMSU_DAO;
import de.fhg.fokus.hss.db.op.RTR_PPR_DAO;
import de.fhg.fokus.hss.diam.DiameterConstants;
import de.fhg.fokus.hss.diam.DiameterStack;
import de.fhg.fokus.hss.diam.UtilAVP;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class PPR {
	private static Logger logger = Logger.getLogger(PPR.class);
	
	public static void sendRequest(DiameterPeer diameterPeer, DiameterStack diameterStack, int id_impi, int id_implicit_set, int type, int grp){		
		DiameterMessage request = diameterPeer.newRequest(DiameterConstants.Command.PPR, DiameterConstants.Application.Cx);
		request.flagProxiable = true;
		
		// add Auth-Session-State and Vendor-Specific-Application-ID
		UtilAVP.addAuthSessionState(request, DiameterConstants.AVPValue.ASS_No_State_Maintained);
		UtilAVP.addVendorSpecificApplicationID(request, DiameterConstants.Vendor.V3GPP, DiameterConstants.Application.Cx);
		
		boolean dbException = false;
		try{
        	Session session = HibernateUtil.getCurrentSession();
        	HibernateUtil.beginTransaction();
        	IMPU impu = IMPU_DAO.get_one_from_set(session, id_implicit_set);
        	if (impu.getUser_state() == CxConstants.IMPU_user_state_Not_Registered || impu.getUser_state() == CxConstants.IMPU_user_state_Auth_Pending){
        		logger.error("Implicit Registration Set: " + id_implicit_set + " is in Not-Registered state!");
        		logger.error("PPR aborted!");
        	}
        	else{
        		IMPI impi = IMPI_DAO.get_by_ID(session, id_impi);
        		IMSU imsu = IMSU_DAO.get_by_ID(session, impi.getId_imsu());
        		UtilAVP.addDestinationHost(request, imsu.getDiameter_name());
        		
        		String userData = SAR.downloadUserData(impi.getIdentity(), id_implicit_set);
        		ChargingInfo chgInfo = ChargingInfo_DAO.get_by_ID(session, impu.getId_charging_info());
			
        		UtilAVP.addUserName(request, impi.getIdentity());
        		if (type == 1){
        			// add only User Data
        			UtilAVP.addUserData(request, userData);
        		}
        		else if (type == 2){
        			// 	add only Charging Info
        			UtilAVP.addChargingInformation(request, chgInfo);
        		}
        		else{
        			// 	add both
        			UtilAVP.addUserData(request, userData);
        			UtilAVP.addChargingInformation(request, chgInfo);
        		}
        		
        		// add hopbyhop and endtoend ID into the rtr_ppr table
        		RTR_PPR_DAO.update_by_grp(session, grp, request.hopByHopID, request.endToEndID);
        		
        		// send the request
        		diameterPeer.sendRequestTransactional(imsu.getDiameter_name(), request, diameterStack);
        	}
		}
		catch(DatabaseException e){
			logger.error("Database Exception occured!\nReason:" + e.getMessage());
			e.printStackTrace();
			dbException = true;
		}
		catch (HibernateException e){
			logger.error("Hibernate Exception occured!\nReason:" + e.getMessage());
			e.printStackTrace();
			dbException = true;
		}
		finally{
			if (!dbException){
				HibernateUtil.commitTransaction();
			}
			HibernateUtil.closeSession();
		}		
	}
	
	public static void processResponse(DiameterPeer diameterPeer, DiameterMessage response){
		logger.info("Processing PPR Response!");
		
		String vendorSpecificAppID = UtilAVP.getVendorSpecificApplicationID(response);
		String authSessionState = UtilAVP.getAuthSessionState(response);
		String originHost = UtilAVP.getOriginatingHost(response);
		String originRealm = UtilAVP.getOriginatingRealm(response);
			
		if (vendorSpecificAppID == null || authSessionState == null || originHost == null || originRealm == null){
			logger.warn("Required Parameters are null in PPA!");
		}
		
		// get the response code...
		// to be implemented
		
		// delete the coresponding row from rtr_ppr table
		boolean dbException = false;
		try{
        	Session session = HibernateUtil.getCurrentSession();
        	HibernateUtil.beginTransaction();
        	RTR_PPR_DAO.delete(session, response.hopByHopID, response.endToEndID);
		}
		catch (HibernateException e){
			logger.error("Hibernate Exception occured!\nReason:" + e.getMessage());
			e.printStackTrace();
			dbException = true;
		}
		finally{
			if (!dbException){
				HibernateUtil.commitTransaction();
			}
			HibernateUtil.closeSession();
		}		        	
	}
	
	public static void processTimeout(DiameterMessage request){
		
	}

}
