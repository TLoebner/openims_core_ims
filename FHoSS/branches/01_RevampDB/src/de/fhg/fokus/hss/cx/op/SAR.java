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

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.log4j.Logger;
import org.hibernate.Session;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;
import de.fhg.fokus.diameter.DiameterPeer.data.AVP;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.hss.cx.CxConstants;
import de.fhg.fokus.hss.cx.CxExperimentalResultException;
import de.fhg.fokus.hss.cx.CxFinalResultException;
import de.fhg.fokus.hss.db.model.ApplicationServer;
import de.fhg.fokus.hss.db.model.ChargingInfo;
import de.fhg.fokus.hss.db.model.IFC;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPI_IMPU;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.IMSU;
import de.fhg.fokus.hss.db.model.SP;
import de.fhg.fokus.hss.db.model.SPT;
import de.fhg.fokus.hss.db.model.SP_IFC;
import de.fhg.fokus.hss.db.model.TP;
import de.fhg.fokus.hss.db.op.ApplicationServer_DAO;
import de.fhg.fokus.hss.db.op.ChargingInfo_DAO;
import de.fhg.fokus.hss.db.op.DB_Op;
import de.fhg.fokus.hss.db.op.IMPI_DAO;
import de.fhg.fokus.hss.db.op.IMPI_IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMSU_DAO;
import de.fhg.fokus.hss.db.op.SPT_DAO;
import de.fhg.fokus.hss.db.op.SP_IFC_DAO;
import de.fhg.fokus.hss.db.op.SP_Shared_IFC_Set_DAO;
import de.fhg.fokus.hss.db.op.TP_DAO;
import de.fhg.fokus.hss.diam.DiameterConstants;
import de.fhg.fokus.hss.diam.UtilAVP;
import de.fhg.fokus.hss.db.hibernate.*;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class SAR {
	private static Logger logger = Logger.getLogger(SAR.class);
	
	public static DiameterMessage processRequest(DiameterPeer diameterPeer, DiameterMessage request){
		DiameterMessage response = diameterPeer.newResponse(request);

		// add Auth-Session-State and Vendor-Specific-Application-ID
		UtilAVP.addAuthSessionState(response, DiameterConstants.AVPValue.ASS_No_State_Maintained);
		UtilAVP.addVendorSpecificApplicationID(response, DiameterConstants.Vendor.V3GPP, DiameterConstants.Application.Cx);
		Session session = null;
		
		try{
			String vendorSpecificAppID = UtilAVP.getVendorSpecificApplicationID(request);
			String authSessionState = UtilAVP.getAuthSessionState(request);
			String originHost = UtilAVP.getOriginatingHost(request);
			String originRealm = UtilAVP.getOriginatingRealm(request);
			String destinationRealm = UtilAVP.getDestinationRealm(request);
			String serverName = UtilAVP.getServerName(request);
			int serverAssignmentType = UtilAVP.getServerAssignmentType(request);
			int userDataAlreadyAvailable = UtilAVP.getUserDataAlreadyAvailable(request);
		
			// 0. check for missing of mandatory AVPs
			if (vendorSpecificAppID == null || authSessionState == null || originHost == null || originRealm == null ||
				destinationRealm == null || serverName == null || serverAssignmentType == -1 || 
				userDataAlreadyAvailable == -1){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
			}
			
			String publicIdentity = UtilAVP.getPublicIdentity(request);
			String privateIdentity = UtilAVP.getUserName(request);
			if (publicIdentity == null && privateIdentity == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_MISSING_USER_ID);
			}
			
			// 1. check that the public identity & privateIdentity are known in HSS
			session = HibernateUtil.getCurrentSession();
			HibernateUtil.beginTransaction();

			IMPU impu = IMPU_DAO.get_by_Identity(session, publicIdentity);
			IMPI impi = IMPI_DAO.get_by_Identity(session, privateIdentity);
			if (publicIdentity != null && impu == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_UNKNOWN); 
			}
			if (privateIdentity != null && impi == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_UNKNOWN); 
			}
			
			
			IMPI_IMPU impi_impu;
			// 2. check association
			if (impi != null && impu != null){
				impi_impu = IMPI_IMPU_DAO.get_by_IMPI_and_IMPU_ID(session, impi.getId(), impu.getId());
				if (impi_impu == null){
					throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_IDENTITIES_DONT_MATCH);
				}
			}

			// 3. 
			switch (serverAssignmentType){
				case CxConstants.Server_Assignment_Type_Timeout_Deregistration:
				case CxConstants.Server_Assignment_Type_User_Deregistration:
				case CxConstants.Server_Assignment_Type_Deregistration_Too_Much_Data:
				case CxConstants.Server_Assignment_Type_Timeout_Deregistration_Store_Server_Name:					
				case CxConstants.Server_Assignment_Type_User_Deregistration_Store_Server_Name:
				case CxConstants.Server_Assignment_Type_Administrative_Deregistration:
					break;
					
				default:
					AVP avp = request.findAVP(DiameterConstants.AVPCode.IMS_PUBLIC_IDENTITY, true, DiameterConstants.Vendor.V3GPP);
					if (avp != null){
						AVP next_public_identity = UtilAVP.getNextPublicIdentityAVP(request, avp);
						if (next_public_identity != null){
							throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_AVP_OCCURS_TOO_MANY_TIMES);
						}
					}
			}
			
			
			// 4. check if the public identity is a Pubic Service Identifier; if yes, check the activation
			int impu_type  = impu.getType();
			if (impu_type == CxConstants.IMPU_type_Distinct_PSI || impu_type == CxConstants.IMPU_type_Wildcarded_PSI){
				if (impu.getPsi_activation() == 0){
					throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_UNKNOWN);
				}
			}
			// 5. check the server assignment type received in the request
			switch (serverAssignmentType){
				
				case CxConstants.Server_Assignment_Type_Registration:
				case CxConstants.Server_Assignment_Type_Re_Registration:
					// clear the auth pending if neccessary
					impi_impu = IMPI_IMPU_DAO.get_by_IMPI_and_IMPU_ID(session, impi.getId(), impu.getId());
					
					if (impi_impu.getUser_state() == CxConstants.IMPU_user_state_Auth_Pending){
						impi_impu.setUser_state(CxConstants.IMPU_user_state_Registered);
					}
					
					// set registration state to registered, if neccessary
					// set the registration state on all impu from the same implicitset
					DB_Op.setUserState(session, impi.getId(), impu.getId_implicit_set(), 
							CxConstants.IMPU_user_state_Registered, true);
					UtilAVP.addUserName(response, privateIdentity);
					
					//download the profile data
					String user_data = SAR.downloadUserData(privateIdentity, publicIdentity, impu.getId_implicit_set());
					if (user_data == null){
						throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY);
					}
					UtilAVP.addUserData(response, user_data);
					
					// add charging information
					ChargingInfo chargingInfo = ChargingInfo_DAO.get_by_ID(session, impu.getId_charging_info());
					UtilAVP.addChargingInformation(response, chargingInfo);
					
					// if more private are associated to the IMSU,  AssociatedIdentities AVP are added
					List privateIdentitiesList = IMPI_DAO.get_all_by_IMSU_ID(session, impi.getId_imsu());
					if (privateIdentitiesList != null && privateIdentitiesList.size() > 1){
						UtilAVP.addAsssociatedIdentities(response, privateIdentitiesList);
					}
					UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
					break;
					
				case CxConstants.Server_Assignment_Type_Unregistered_User:
					// store the scscf_name & orgiin_host
					privateIdentitiesList = IMPI_IMPU_DAO.get_all_IMPI_by_IMPU_ID(session, impu.getId());
					if (privateIdentitiesList == null || privateIdentitiesList.size() == 0){
						throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY);
					}
					
					IMPI first_IMPI = (IMPI) privateIdentitiesList.get(0);
					IMSU_DAO.update(session, first_IMPI.getId_imsu(), serverName, originHost);

					// set the user_state to Unregistered
					DB_Op.setUserState(session, first_IMPI.getId(), impu.getId_implicit_set(), 
							CxConstants.IMPU_user_state_Unregistered, true);
					
					//download the profile data
					user_data = SAR.downloadUserData(privateIdentity, publicIdentity, impu.getId_implicit_set());
					if (user_data == null){
						throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY);
					}
					UtilAVP.addUserData(response, user_data);
					
					// add charging Info
					chargingInfo = ChargingInfo_DAO.get_by_ID(session, impu.getId_charging_info());
					UtilAVP.addChargingInformation(response, chargingInfo);
					
					// add a private to the response (the first private found, if more than one are available)					
					UtilAVP.addUserName(response, first_IMPI.getIdentity());
					
					//AssociatedIdentities if neccessary
					if (privateIdentitiesList.size() > 1){
						UtilAVP.addAsssociatedIdentities(response, privateIdentitiesList);
					}
					
					// result code = diameter success
					UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
					break;
					
				case CxConstants.Server_Assignment_Type_Timeout_Deregistration:
				case CxConstants.Server_Assignment_Type_User_Deregistration:
				case CxConstants.Server_Assignment_Type_Deregistration_Too_Much_Data:
				case CxConstants.Server_Assignment_Type_Administrative_Deregistration:
					
					List impuList = UtilAVP.getAllIMPU(session, request);
					if (impuList == null){
						impuList = IMPI_IMPU_DAO.get_all_Default_IMPU_of_Set_by_IMPI(session, impi.getId());
					}
					if (impuList == null){
						throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY);
					}
					Iterator iterator = impuList.iterator();
					IMPU crt_impu;
					while (iterator.hasNext()){
						crt_impu = (IMPU) iterator.next();
						
						switch (crt_impu.getUser_state()){
							case CxConstants.IMPU_user_state_Registered:
								int reg_cnt = IMPI_IMPU_DAO.get_Registered_IMPU_Count(session, crt_impu.getId());
								if (reg_cnt == 1){
									// set the user_state to Not-Registered
									DB_Op.setUserState(session, impi.getId(), crt_impu.getId_implicit_set(), 
											CxConstants.IMPU_user_state_Not_Registered, true);
									// clear the scscf_name & origin_host
									IMSU_DAO.update(session, impi.getId_imsu(), "", "");
								}
								else{
									DB_Op.setUserState(session, impi.getId(), crt_impu.getId_implicit_set(), 
											CxConstants.IMPU_user_state_Not_Registered, false);
									
								}
								UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
								break;
								
							case CxConstants.IMPU_user_state_Unregistered:
								// set the user_state to Not-Registered
								DB_Op.setUserState(session, impi.getId(), crt_impu.getId_implicit_set(), 
										CxConstants.IMPU_user_state_Not_Registered, true);
								// clear the scscf_name & origin_host
								IMSU_DAO.update(session, impi.getId_imsu(), "", "");
								UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
								break;
						}
					}
					break;
					
				case CxConstants.Server_Assignment_Type_Timeout_Deregistration_Store_Server_Name:
				case CxConstants.Server_Assignment_Type_User_Deregistration_Store_Server_Name:		
					// HSS decides not to keep the S-CSCF name
					if (impi == null){
						throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_MISSING_USER_ID);
					}	
					impuList = UtilAVP.getAllIMPU(session, request);
					if (impuList == null){
						impuList = IMPI_IMPU_DAO.get_all_Default_IMPU_of_Set_by_IMPI(session, impi.getId());
					}
					if (impuList == null){
						throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY);
					}

					iterator = impuList.iterator();
					while (iterator.hasNext()){
						crt_impu = (IMPU) iterator.next();
						
						int reg_cnt = IMPI_IMPU_DAO.get_Registered_IMPU_Count(session, crt_impu.getId());
						if (reg_cnt == 1){
							// set the user_state to Not-Registered
							DB_Op.setUserState(session, impi.getId(), crt_impu.getId_implicit_set(), 
									CxConstants.IMPU_user_state_Not_Registered, true);
							// clear the scscf_name & origin_host
							IMSU_DAO.update(session, impi.getId_imsu(), "", "");
						}
						else{
							// Set the user_state to Not-Registered only on IMPI_IMPU association, IMPU registration state
							//remain registered
							DB_Op.setUserState(session, impi.getId(), crt_impu.getId_implicit_set(), 
									CxConstants.IMPU_user_state_Not_Registered, false);
						}
					}
					
					UtilAVP.addExperimentalResultCode(response, 
							DiameterConstants.ResultCode.RC_IMS_DIAMETER_SUCCESS_SERVER_NAME_NOT_STORED.getCode());
					break;
					
				case CxConstants.Server_Assignment_Type_No_Assignment:
					if (impi == null || impu == null){
						throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_MISSING_USER_ID);
					}
					IMSU imsu = IMSU_DAO.get_by_ID(session, impi.getId_imsu());
					String scscf_name = imsu.getScscf_name();
					
					if (!scscf_name.equals(serverName)){
						if (!scscf_name.equals("")){
							UtilAVP.addExperimentalResultCode(response, 
									DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_IDENTITY_ALREADY_REGISTERED.getCode());	
						}
						else{
							UtilAVP.addResultCode(response, 
									DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY.getCode());
						}
					}
					else{
						
						user_data = SAR.downloadUserData(privateIdentity, publicIdentity, impu.getId_implicit_set());
						if (user_data == null){
							throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY);
						}
						UtilAVP.addUserData(response, user_data);
						// add charging information
						chargingInfo = ChargingInfo_DAO.get_by_ID(session, impu.getId_charging_info());
						UtilAVP.addChargingInformation(response, chargingInfo);
												
						UtilAVP.addUserName(response, impi.getIdentity());
						
						privateIdentitiesList = IMPI_DAO.get_all_by_IMSU_ID(session, impi.getId_imsu());
						if (privateIdentitiesList != null && privateIdentitiesList.size() > 1){
							UtilAVP.addAsssociatedIdentities(response, privateIdentitiesList);
						}

						UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
					}
					break;
					
				case CxConstants.Server_Assignment_Type_Authentication_Failure:
				case CxConstants.Server_Assignment_Type_Authentication_Timeout:	
					if (impi == null || impu == null){
						throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_MISSING_USER_ID);
					}

					impi_impu = IMPI_IMPU_DAO.get_by_IMPI_and_IMPU_ID(session, impi.getId(), impu.getId());
					int user_state = impi_impu.getUser_state();
					switch (user_state){
					
						case CxConstants.IMPU_user_state_Registered:
							int reg_cnt = IMPI_IMPU_DAO.get_Registered_IMPU_Count(session, impu.getId());
							if (reg_cnt == 1){
								// set the user_state to Not-Registered
								DB_Op.setUserState(session, impi.getId(), impu.getId_implicit_set(), 
										CxConstants.IMPU_user_state_Not_Registered, true);
								// clear the scscf_name & origin_host
								IMSU_DAO.update(session, impi.getId_imsu(), "", "");
							}
							else{
								// Set the user_state to Not-Registered only on IMPI_IMPU association, 
								//IMPU registration state remain registered
								DB_Op.setUserState(session, impi.getId(), impu.getId_implicit_set(), 
										CxConstants.IMPU_user_state_Not_Registered, false);
							}
							break;
							
						case CxConstants.IMPU_user_state_Unregistered:
							// set the user_state to Not-Registered
							DB_Op.setUserState(session, impi.getId(), impu.getId_implicit_set(), 
									CxConstants.IMPU_user_state_Not_Registered, true);
							// clear the scscf_name & origin_host
							IMSU_DAO.update(session, impi.getId_imsu(), "", "");
							break;
							
						case CxConstants.IMPU_user_state_Auth_Pending:
							// reset Auth-Pending
							DB_Op.resetAuthPending(session, impi.getId(), impu.getId_implicit_set());
							break;
					}
					
					UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
					break;
			}
		}
		catch(CxExperimentalResultException e){
			UtilAVP.addExperimentalResultCode(response, e.getErrorCode());
			e.printStackTrace();
		}
		catch(CxFinalResultException e){
			UtilAVP.addResultCode(response, e.getErrorCode());
			e.printStackTrace();
		}
		finally{
			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
		}
		return response;
	}
	
	public static final String xml_version="<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	public static final String ims_subscription_s="<IMSSubscription>";
	public static final String ims_subscription_e="</IMSSubscription>";
	public static final String private_id_s="<PrivateID>";
	public static final String private_id_e="</PrivateID>";
	public static final String service_profile_s="<ServiceProfile>";
	public static final String service_profile_e="</ServiceProfile>";
	public static final String public_id_s="<PublicIdentity>";
	public static final String public_id_e="</PublicIdentity>";
	public static final String barring_indication_s="<BarringIndication>";
	public static final String barring_indication_e="</BarringIndication>";
	public static final String identity_s="<Identity>";
	public static final String identity_e="</Identity>";

	public static final String ifc_s="<InitialFilterCriteria>";
	public static final String ifc_e="</InitialFilterCriteria>";
	public static final String priority_s="<Priority>";
	public static final String priority_e="</Priority>";
	public static final String tp_s="<TriggerPoint>";
	public static final String tp_e="</TriggerPoint>";
	public static final String cnf_s="<ConditionTypeCNF>";
	public static final String cnf_e="</ConditionTypeCNF>";
	public static final String spt_s="<SPT>";
	public static final String spt_e="</SPT>";
	public static final String condition_negated_s="<ConditionNegated>";
	public static final String condition_negated_e="</ConditionNegated>";
	public static final String group_s="<Group>";
	public static final String group_e="</Group>";
	public static final String req_uri_s="<RequestURI>";
	public static final String req_uri_e="</RequestURI>";
	public static final String method_s="<Method>";	
	public static final String method_e="</Method>";	
	public static final String sip_hdr_s="<SIPHeader>";
	public static final String sip_hdr_e="</SIPHeader>";
	public static final String session_case_s="<SessionCase>";
	public static final String session_case_e="</SessionCase>";
	public static final String session_desc_s="<SessionDescription>";
	public static final String session_desc_e="</SessionDescription>";
	public static final String registration_type_s="<RegistrationType>";
	public static final String registration_type_e="</RegistrationType>";
	public static final String header_s="<Header>";
	public static final String header_e="</Header>";
	public static final String content_s="<Content>";
	public static final String content_e="</Content>";
	public static final String line_s="<Line>";
	public static final String line_e="</Line>";

	public static final String app_server_s="<ApplicationServer>";
	public static final String app_server_e="</ApplicationServer>";
	public static final String server_name_s="<ServerName>";
	public static final String server_name_e="</ServerName>";
	public static final String default_handling_s="<DefaultHandling>";
	public static final String default_handling_e="</DefaultHandling>";
	public static final String service_info_s="<ServiceInfo>";
	public static final String service_info_e="</ServiceInfo>";

	public static final String profile_part_ind_s="<ProfilePartIndicator>";
	public static final String profile_part_ind_e="</ProfilePartIndicator>";

	public static final String cn_services_auth_s="<CoreNetworkServicesAuthorization>";
	public static final String cn_services_auth_e="</CoreNetworkServicesAuthorization>";
	public static final String subs_media_profile_id_s="<SubscribedMediaProfileId>";
	public static final String subs_media_profile_id_e="</SubscribedMediaProfileId>";
	public static final String shared_ifc_set_id_s="<Extension><SharedIFCSetID>";
	public static final String shared_ifc_set_id_e="</SharedIFCSetID></Extension>";

	public static final String extension_s = "<Extension>";
	public static final String extension_e = "</Extension>";
	
	public static final String zero="0";
	public static final String one="1";
	public static final String two="2";

	private static String downloadUserData(String privateIdentity, String publicIdentity, int id_implicit_set){
		Session session = HibernateUtil.getCurrentSession();
		List<IMPU> [] impu_array;
		SP[] sp_array;
		
		int sp_cnt = 0;
		List queryResult = IMPU_DAO.get_all_sp_for_set(session, id_implicit_set);

		Iterator it = queryResult.iterator();
		int last_id_sp = -1; 
		while (it.hasNext()){
			SP sp = null;
			Object [] row = (Object[]) it.next();
			sp = (SP) row[0];
			int current_id_sp = sp.getId();
			if (current_id_sp != last_id_sp){
				sp_cnt++;
				last_id_sp = current_id_sp;
			}
		}
		
		sp_array = new SP[sp_cnt];
		impu_array = new ArrayList[sp_cnt]; 
		it = queryResult.iterator();
		last_id_sp = -1;
		int idx = -1;
		while (it.hasNext()){
			Object [] row = (Object []) it.next();
			SP sp = (SP)row[0];
			IMPU impu = (IMPU) row[1];
			
			int current_id_sp = sp.getId();
			if (current_id_sp != last_id_sp){
				last_id_sp = current_id_sp;
				idx++;
				sp_array[idx] = sp;
				impu_array[idx] = new ArrayList();
			}
			impu_array[idx].add(impu);
		}

		// begin to write the data in the buffer
		StringBuffer sb = new StringBuffer();
		sb.append(xml_version);
		sb.append(ims_subscription_s);
		
		// PrivateID
		sb.append(private_id_s);
		sb.append(privateIdentity);
		sb.append(private_id_e);
		
		System.out.println("\nBefore SP:\n" + sb.toString());
		//SP
		for (int i = 0; i < sp_array.length; i++){
			System.out.println("\nCurrent Iteration:\n" + i);
			sb.append(service_profile_s);
			// PublicIdentity 					=> 1 to n
			
			System.out.println("\nBefore Public identity:\n" + sb.toString());
			it = impu_array[i].iterator();
			while (it.hasNext()){
				IMPU impu = (IMPU)it.next();
				sb.append(public_id_s);
				sb.append(identity_s);
				sb.append(impu.getIdentity());
				sb.append(identity_e);
				sb.append(public_id_e);
			}
			
			System.out.println("\nAfter Public identity:\n" + sb.toString());
			
			// InitialFilterCriteria 			=> 0 to n
			
			List list_ifc = SP_IFC_DAO.get_all_SP_IFC_by_SP_ID(session, sp_array[i].getId());
			if (list_ifc != null && list_ifc.size() > 0){
				Iterator it_ifc;
				it_ifc = list_ifc.iterator();
				Object[] crt_row;
				
				while (it_ifc.hasNext()){
					sb.append(ifc_s);
					crt_row = (Object[]) it_ifc.next();
					SP_IFC crt_sp_ifc= (SP_IFC) crt_row[0];
					IFC crt_ifc = (IFC) crt_row[1];
					
					if (crt_ifc.getId_application_server() == -1 || crt_sp_ifc.getPriority() == -1){
						// error, application server and priority are mandatory!
						return null;
					}
					
					// adding priority
					sb.append(priority_s);
					sb.append(crt_sp_ifc.getPriority());
					sb.append(priority_e);
					
					// add trigger
					if (crt_ifc.getId_tp() != -1){
						// we have a trigger to add
						sb.append(tp_s);
						
						TP tp = TP_DAO.get_by_ID(session, crt_ifc.getId_tp());
						sb.append(cnf_s);
						sb.append(tp.getCondition_type_cnf());
						sb.append(cnf_e);
						
						List list_spt = SPT_DAO.get_all_by_TP_ID(session, tp.getId());
						if (list_spt == null){
							logger.error("[SAR][Download-User-Data] SPT list is NULL! Should be at least one SPT asssociated to the TP!");
							return null;
						}
						
						// add SPT
						Iterator it_spt = list_spt.iterator();
						SPT crt_spt;
						while (it_spt.hasNext()){
							crt_spt = (SPT) it_spt.next();
							sb.append(spt_s);
							
							// condition negated
							sb.append(condition_negated_s);
							sb.append(crt_spt.getCondition_negated());
							sb.append(condition_negated_e);
							
							// group
							sb.append(group_s);
							sb.append(crt_spt.getGrp());
							sb.append(group_e);
							
							switch (crt_spt.getType()){
								
								case CxConstants.SPT_Type_RequestURI:
									sb.append(req_uri_s);
									sb.append(crt_spt.getRequesturi());
									sb.append(req_uri_e);
									break;

								case CxConstants.SPT_Type_Method:
									sb.append(method_s);
									sb.append(crt_spt.getMethod());
									sb.append(method_e);
									break;
								
								case CxConstants.SPT_Type_SIPHeader:
									sb.append(sip_hdr_s);
									
									sb.append(header_s);
									sb.append(crt_spt.getHeader());
									sb.append(header_e);
									sb.append(content_s);
									sb.append(crt_spt.getHeader_content());
									sb.append(content_e);

									sb.append(sip_hdr_e);
									break;
								
								case CxConstants.SPT_Type_SessionCase:
									sb.append(session_case_s);
									sb.append(crt_spt.getSession_case());
									sb.append(session_case_e);
									break;
								
								case CxConstants.SPT_Type_SessionDescription:
									sb.append(session_desc_s);
									
									sb.append(line_s);
									sb.append(crt_spt.getSdp_line());
									sb.append(line_e);
									sb.append(content_s);
									sb.append(crt_spt.getSdp_line_content());
									sb.append(content_e);
									
									sb.append(session_desc_e);
									break;
							}
							
							// add Extension if available
							
							if (crt_spt.getRegistration_type() != -1){
								sb.append(extension_s);
								sb.append(registration_type_s);
								switch (crt_spt.getRegistration_type()){
									 case CxConstants.Registration_Type_Initial_Registration:
										 sb.append(zero);
										 break;
									 case CxConstants.Registration_Type_Initial_Re_Registration:
										 sb.append(one);
										 break;
									 case CxConstants.Registration_Type_Initial_De_Registration:
										 sb.append(two);
										 break;
										 
								}
								sb.append(registration_type_e);
								sb.append(extension_e);
							}
							
							sb.append(spt_e);
						}

						sb.append(tp_e);
					}

					// add the Application Server
					ApplicationServer crt_as = ApplicationServer_DAO.get_by_ID(session, crt_ifc.getId_application_server());
					sb.append(app_server_s);
					
					sb.append(server_name_s);
					System.out.println("Server name:" + crt_as.getServer_name());
					sb.append(crt_as.getServer_name());
					sb.append(server_name_e);
					if (crt_as.getDefault_handling() != -1){
						sb.append(default_handling_s);
						sb.append(crt_as.getDefault_handling());
						sb.append(default_handling_e);
					}
					
					if (crt_as.getService_info() != null && !crt_as.getService_info().equals("")){
						sb.append(service_info_s);
						sb.append(crt_as.getService_info());
						sb.append(service_info_e);
					}

					sb.append(app_server_e);
					
					if (crt_ifc.getProfile_part_ind() != -1){
						// add the profile part indicator
						sb.append(profile_part_ind_s);
						sb.append(crt_ifc.getProfile_part_ind());
						sb.append(profile_part_ind_e);

					}
					
					sb.append(ifc_e);
				}
				
			}
			/*
			// CoreNetworkServiceAuthorization	=> 0 to n
			if (sp_array[i].getCn_service_auth() != -1){
				sb.append(cn_services_auth_s);
				sb.append(subs_media_profile_id_s);
				sb.append(sp_array[i].getCn_service_auth());
				sb.append(subs_media_profile_id_e);
				sb.append(cn_services_auth_e);
			}
			*/
			
			
			// Extension						=> 0 to 1
			List all_IDs = SP_Shared_IFC_Set_DAO.get_all_shared_IFC_set_IDs_by_SP_ID(session, sp_array[i].getId());
			if (all_IDs != null && all_IDs.size() > 0){
				sb.append(extension_s);
				Iterator all_IDs_it = all_IDs.iterator();
				while (all_IDs_it.hasNext()){
					int crt_ID = ((Integer) all_IDs_it.next()).intValue();
					sb.append(shared_ifc_set_id_s);
					sb.append(crt_ID);
					sb.append(shared_ifc_set_id_e);
				}
				sb.append(extension_e);
			}
			sb.append(service_profile_e);
		}
		sb.append(ims_subscription_e);
		System.out.println("The XML document which is prepared to be sent to the S-CSCF:\n" + sb.toString());
		return sb.toString();
	}
}
