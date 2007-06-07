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

package de.fhg.fokus.hss.sh.op;

import java.util.Vector;

import org.apache.log4j.Logger;
import org.hibernate.HibernateException;
import org.hibernate.Session;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.hss.cx.CxConstants;
import de.fhg.fokus.hss.db.hibernate.HibernateUtil;
import de.fhg.fokus.hss.db.model.AliasesRepositoryData;
import de.fhg.fokus.hss.db.model.ApplicationServer;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.RepositoryData;
import de.fhg.fokus.hss.db.model.ShSubscription;
import de.fhg.fokus.hss.db.op.AliasesRepositoryData_DAO;
import de.fhg.fokus.hss.db.op.ApplicationServer_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
import de.fhg.fokus.hss.db.op.RepositoryData_DAO;
import de.fhg.fokus.hss.db.op.ShSubscription_DAO;
import de.fhg.fokus.hss.diam.DiameterConstants;
import de.fhg.fokus.hss.diam.UtilAVP;
import de.fhg.fokus.hss.sh.ShConstants;
import de.fhg.fokus.hss.sh.ShExperimentalResultException;
import de.fhg.fokus.hss.sh.data.ShDataElement;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class SNR {
	private static Logger logger = Logger.getLogger(SNR.class);
	
	public static DiameterMessage processRequest(DiameterPeer diameterPeer, DiameterMessage request){
		if (request.flagProxiable == false){
			logger.warn("You should notice that the Proxiable flag for SNR request was not set!");
		}
		DiameterMessage response = diameterPeer.newResponse(request);
		// set the proxiable flag for the response
		response.flagProxiable = true;
		
		boolean dbException = false;
		try{
			// -0- check for mandatory fields in the message
			String vendor_specific_ID = UtilAVP.getVendorSpecificApplicationID(request);
			String auth_session_state = UtilAVP.getAuthSessionState(request);
			String origin_host = UtilAVP.getOriginatingHost(request);
			String origin_realm = UtilAVP.getOriginatingRealm(request);
			String dest_realm = UtilAVP.getDestinationRealm(request);
			String user_identity = UtilAVP.getShUserIdentity(request);
			int subs_req_type = UtilAVP.getSubsReqType(request);
			Vector data_ref_vector = UtilAVP.getAllDataReference(request);
			Vector service_indication_vector = UtilAVP.getAllServiceIndication(request);
			
			if (vendor_specific_ID == null || auth_session_state == null || origin_host == null || origin_realm == null ||
					dest_realm == null || user_identity == null || data_ref_vector == null || data_ref_vector.size() == 0 ||
					subs_req_type == -1){
				throw new ShExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
			}

			// add Auth-Session-State and Vendor-Specific-Application-ID to Response
			UtilAVP.addAuthSessionState(response, DiameterConstants.AVPValue.ASS_No_State_Maintained);
			UtilAVP.addVendorSpecificApplicationID(response, DiameterConstants.Vendor.V3GPP, DiameterConstants.Application.Sh);
		
			// get the current session and open a hibernate transaction
			Session session = HibernateUtil.getCurrentSession();
			HibernateUtil.beginTransaction();
			
			// - 1 -
			ApplicationServer application_server = ApplicationServer_DAO.get_by_Diameter_Address(session, origin_host);
			if (application_server == null){
				throw new ShExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_DATA_CANNOT_BE_NOTIFIED);
			}
			
			// check if the request is allowed for the specified application server
			if (application_server.getSnr() == 0){
				throw new ShExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_DATA_CANNOT_BE_NOTIFIED);				
			}
			
			// check if the AS is allowed to subscribe to notifications for the requested data 
			//    (check the combination of the AS identity and the supplied Data Reference)
			
			for (int i = 0; i < data_ref_vector.size(); i++){
				int crt_data_ref = (Integer) data_ref_vector.get(i); 

				if ((crt_data_ref == ShConstants.Data_Ref_Repository_Data && application_server.getSnr_rep_data() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_IMS_Public_Identity && application_server.getSnr_impu() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_IMS_User_State && application_server.getSnr_ims_user_state() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_SCSCF_Name && application_server.getSnr_scscf_name() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_iFC && application_server.getSnr_ifc() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_PSI_Activation && application_server.getSnr_psi_activation() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_DSAI && application_server.getSnr_dsai() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_Aliases_Repository_Data && application_server.getSnr_aliases_rep_data() == 0)){
							throw new ShExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_DATA_CANNOT_BE_NOTIFIED);
				}
				
				// check if the service_indication is present in the request (only for RepositoryData)
				if ((crt_data_ref == ShConstants.Data_Ref_Aliases_Repository_Data || crt_data_ref == ShConstants.Data_Ref_Repository_Data)
						&& (service_indication_vector == null || service_indication_vector.size() == 0)){
					throw new ShExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
				}
			}
			
			// -2- check if the IMPU or PSI from the request exists in the HSS
			IMPU impu = IMPU_DAO.get_by_Identity(session, user_identity);
			if (impu == null){
				throw new ShExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_USER_UNKNOWN);				
			}

			// get all the rest of the headers from the SNR request
			String server_name = UtilAVP.getServerName(request);
			String dsai_tag = UtilAVP.getDSAITag(request);
			
			
			// -3- check if the user identity apply or not to the Data Reference indicated in the request, according to table 7.6.1 TS 29.328
			for (int i = 0; i < data_ref_vector.size(); i++){
				int crt_data_ref = (Integer) data_ref_vector.get(i);
				
				if (impu.getType() != CxConstants.Identity_Type_Public_User_Identity){
					if ((crt_data_ref == ShConstants.Data_Ref_IMS_Public_Identity) || (crt_data_ref == ShConstants.Data_Ref_IMS_User_State) ||
							(crt_data_ref == ShConstants.Data_Ref_Aliases_Repository_Data)	){
						throw new ShExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_OPERATION_NOT_ALLOWED);
					}
				}
				else if (impu.getType() == CxConstants.Identity_Type_Public_User_Identity){
					if (crt_data_ref == ShConstants.Data_Ref_PSI_Activation){
						throw new ShExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_OPERATION_NOT_ALLOWED);
					}
				}
				
				// -3a- 
				if (crt_data_ref == ShConstants.Data_Ref_DSAI){
					//...to be completed
				}
				
				// -3b-
				if (crt_data_ref == ShConstants.Data_Ref_Aliases_Repository_Data){
					// ... to be completed
				}
			}

			// -4- get the Expiry Time
			int expiry_time = UtilAVP.getExpiryTime(request);
			if (expiry_time != -1){
				UtilAVP.addExpiryTime(response, expiry_time);
			}

			// -5- if the Data Reference is RepositoryData or Aliases Repository Data
			for (int i = 0; i < data_ref_vector.size(); i++){
				int crt_data_ref = (Integer) data_ref_vector.get(i);
				
				for (int j = 0; j < service_indication_vector.size(); j++){
					// iterate through all the service indications
					if (crt_data_ref == ShConstants.Data_Ref_Repository_Data){
						String crt_service_indication = (String) service_indication_vector.get(j);
						RepositoryData rep_data = RepositoryData_DAO.get_by_IMPU_and_ServiceIndication(
								session, impu.getId(), crt_service_indication);
						if (rep_data == null){
							throw new ShExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_SUBS_DATA_ABSENT);	
						}
					}
					else if (crt_data_ref == ShConstants.Data_Ref_Aliases_Repository_Data){
						String crt_service_indication = (String) service_indication_vector.get(j);
						AliasesRepositoryData aliases_rep_data = AliasesRepositoryData_DAO.get_by_setID_and_ServiceIndication(
								session, impu.getId_implicit_set(), crt_service_indication);
						if (aliases_rep_data == null){
							throw new ShExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_SUBS_DATA_ABSENT);	
						}
					}					
				}
			}
			
			// -6- ==> -10-
			ShDataElement shData = null;			
			for (int i = 0; i < data_ref_vector.size(); i++){
				int crt_data_ref = (Integer) data_ref_vector.get(i);
				
				if (crt_data_ref != ShConstants.Data_Ref_Aliases_Repository_Data && 
						crt_data_ref != ShConstants.Data_Ref_Repository_Data){
				
					if (subs_req_type == 0){
						// subscribe
						ShSubscription sh_subs = ShSubscription_DAO.get_by_AS_IMPU_and_DataRef(session, 
								application_server.getId(), impu.getId(), crt_data_ref);
						if (sh_subs == null){
							sh_subs = new ShSubscription();
							sh_subs.setId_application_server(application_server.getId());
							sh_subs.setData_ref(crt_data_ref);
							sh_subs.setId_impu(impu.getId());
							if (expiry_time != -1){
								sh_subs.setExpires(expiry_time);
							}
						}
						
						if (crt_data_ref == ShConstants.Data_Ref_DSAI){
							// test if dsai_tag is null....
							sh_subs.setDsai_tag(dsai_tag);
						}
						else if (crt_data_ref == ShConstants.Data_Ref_iFC){
							//the same....
							sh_subs.setServer_name(server_name);
						}
					
						// for both situations (update or new insert) we are updating the fields
						ShSubscription_DAO.update(session, sh_subs);
					}
					else{
						// un-subscribe
						if (crt_data_ref == ShConstants.Data_Ref_DSAI){
							ShSubscription_DAO.delete_by_AS_IMPU_DataRef_and_DSAITag(session, application_server.getId(), impu.getId(), 
									crt_data_ref, dsai_tag);
						}
						else if (crt_data_ref == ShConstants.Data_Ref_iFC){
							ShSubscription_DAO.delete_by_AS_IMPU_DataRef_and_ServerName(session, application_server.getId(), impu.getId(),
									crt_data_ref, server_name);
						}
						else{
							ShSubscription_DAO.delete_by_AS_IMPU_and_DataRef(session, application_server.getId(), impu.getId(), crt_data_ref);
						}
					}

					if (UtilAVP.getSendDataIndication(request) == ShConstants.User_Data_Requested){
						// we have to add the user data in the response
						if (shData == null){
							shData = new ShDataElement();
						}
						UDR.addShData(shData, crt_data_ref, impu, null, server_name);
					}
				}
				else{
					// we have Repository Data or Aliases Repository Data
					for (int j = 0; j < service_indication_vector.size(); j++){
						String crt_service_indication = (String) service_indication_vector.get(j);
						if (subs_req_type == 0){
							// subscribe
							ShSubscription sh_subs = ShSubscription_DAO.get_by_AS_IMPU_DataRef_and_ServInd(session, 
									application_server.getId(), impu.getId(), crt_data_ref, crt_service_indication);
							if (sh_subs == null){
								sh_subs = new ShSubscription();
								sh_subs.setId_application_server(application_server.getId());
								sh_subs.setData_ref(crt_data_ref);
								sh_subs.setId_impu(impu.getId());
								sh_subs.setService_indication(crt_service_indication);
								if (expiry_time != -1){
									sh_subs.setExpires(expiry_time);
								}
								ShSubscription_DAO.update(session, sh_subs);
							}
						}
						else{
							// unsubscribe
							ShSubscription_DAO.delete_by_AS_IMPU_DataRef_and_ServInd(session, application_server.getId(), 
									impu.getId(), crt_data_ref, crt_service_indication);
						}
						
						if (UtilAVP.getSendDataIndication(request) == ShConstants.User_Data_Requested){
							// we have to add the user data in the response
							if (shData == null){
								shData = new ShDataElement();
							}
							UDR.addShData(shData, crt_data_ref, impu, crt_service_indication, server_name);
						}
					}
					
				}
				
			} // end loop data_ref
			
			if (shData != null){
				// add the Sh User Data to the response
				UtilAVP.addShData(response, shData.toString());
			}
			UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
			
		}
		catch (HibernateException e){
			dbException = true;
			UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY.getCode());
			e.printStackTrace();
		}
		catch(ShExperimentalResultException e){
			UtilAVP.addExperimentalResultCode(response, e.getErrorCode());
			e.printStackTrace();
		}
/*		catch(ShFinalResultException e){
			UtilAVP.addResultCode(response, e.getErrorCode());
			e.printStackTrace();
		}*/
		finally{
			// commit transaction only when a Database doesn't occured
			if (!dbException){
				HibernateUtil.commitTransaction();
			}
			HibernateUtil.closeSession();
		}
		
		
		return response;
	}

}
