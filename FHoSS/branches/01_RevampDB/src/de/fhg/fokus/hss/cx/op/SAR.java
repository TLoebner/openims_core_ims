/**
 * 
 */
package de.fhg.fokus.hss.cx.op;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.hibernate.Session;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.hss.cx.CxConstants;
import de.fhg.fokus.hss.cx.CxExperimentalResultException;
import de.fhg.fokus.hss.cx.CxFinalResultException;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPI_IMPU;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.SP;
import de.fhg.fokus.hss.db.op.DB_Op;
import de.fhg.fokus.hss.db.op.IMPI_DAO;
import de.fhg.fokus.hss.db.op.IMPI_IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMSU_DAO;
import de.fhg.fokus.hss.diam.DiameterConstants;
import de.fhg.fokus.hss.diam.UtilAVP;
import de.fhg.fokus.hss.util.HibernateUtil;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class SAR {
	
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
			
			IMPU impu = IMPU_DAO.getByIdentity(session, publicIdentity);
			IMPI impi = IMPI_DAO.getByIdentity(session, privateIdentity);
			if (impu == null || impi == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_UNKNOWN); 
			}
			
			IMPI_IMPU impi_impu;
			// 2. check association
			if (impi != null && impu != null){
				impi_impu = IMPI_IMPU_DAO.getByIMPI_IMPU(session, impi.getId(), impu.getId());
				if (impi_impu == null){
					throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_IDENTITIES_DONT_MATCH);
				}
			}
			
			// 3. skiped
			// to be completed
			
			// 4. check if the public identity is a Pubic Service Identifier; if yes, check the activation
			short impu_type  = impu.getType();
			if (impu_type == CxConstants.IMPU_type_Distinct_PSI || impu_type == CxConstants.IMPU_type_Wildcarded_PSI){
				if (impu.getPsi_activation() == 0){
					throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_UNKNOWN);
				}
			}
			
			switch (serverAssignmentType){
				
				case CxConstants.Server_Assignment_Type_Registration:
				case CxConstants.Server_Assignment_Type_Re_Registration:
					// clear the auth pending if neccessary
					impi_impu = IMPI_IMPU_DAO.getByIMPI_IMPU(session, impi.getId(), impu.getId());
					
					if (impi_impu.getUser_state() == CxConstants.IMPU_user_state_Auth_Pending){
						impi_impu.setUser_state(CxConstants.IMPU_user_state_Registered);
					}
					
					// set registration state to registered, if neccessary
					// set the registration state on all impu from the same implicitset
					DB_Op.setUserState(session, impi.getId(), impu.getId_impu_implicitset(), 
							CxConstants.IMPU_user_state_Registered, true);
					UtilAVP.addUserName(response, privateIdentity);
					
					//download the profile data
					String user_data = SAR.downloadUserData(privateIdentity, publicIdentity, impu.getId_impu_implicitset());
					UtilAVP.addUserData(response, user_data);

					// if more private are associated to the IMSU,  AssociatedIdentities AVP are added
					List privateIdentitiesList = IMPI_DAO.getAllByIMSU(session, impi.getImsu().getId());
					if (privateIdentitiesList.size() > 1){
						UtilAVP.addAsssociatedIdentities(response, privateIdentitiesList);
					}
					UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
					break;
					
				case CxConstants.Server_Assignment_Type_Unregistered_User:
					// store the scscf_name & orgiin_host
					IMSU_DAO.update(session, impi.getImsu().getId(), serverName, originHost);

					// set the user_state to Unregistered
					DB_Op.setUserState(session, impi.getId(), impu.getId_impu_implicitset(), 
							CxConstants.IMPU_user_state_Unregistered, true);
					
					// add a private to the response (the first private found, if are more than one available)
					privateIdentitiesList = IMPI_IMPU_DAO.get_all_IMPI_by_IMPU(session, impu.getId());
					if (privateIdentitiesList == null || privateIdentitiesList.size() == 0){
						throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY);
					}
					UtilAVP.addUserName(response, ((IMPI)privateIdentitiesList.get(0)).getIdentity());
					
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
								int reg_cnt = IMPI_IMPU_DAO.get_IMPU_Registered_cnt(session, crt_impu.getId());
								if (reg_cnt == 1){
									// set the user_state to Not-Registered
									DB_Op.setUserState(session, impi.getId(), crt_impu.getId_impu_implicitset(), 
											CxConstants.IMPU_user_state_Not_Registered, true);
									// clear the scscf_name & origin_host
									IMSU_DAO.update(session, impi.getImsu().getId(), "", "");
								}
								else{
									DB_Op.setUserState(session, impi.getId(), crt_impu.getId_impu_implicitset(), 
											CxConstants.IMPU_user_state_Not_Registered, false);
									
								}
								UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
								break;
								
							case CxConstants.IMPU_user_state_Unregistered:
								// set the user_state to Not-Registered
								DB_Op.setUserState(session, impi.getId(), crt_impu.getId_impu_implicitset(), 
										CxConstants.IMPU_user_state_Not_Registered, true);
								// clear the scscf_name & origin_host
								IMSU_DAO.update(session, impi.getImsu().getId(), "", "");
								UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
								break;
						}
					}
					break;
					
				case CxConstants.Server_Assignment_Type_Timeout_Deregistration_Store_Server_Name:
				case CxConstants.Server_Assignment_Type_User_Deregistration_Store_Server_Name:					
					break;
					
				case CxConstants.Server_Assignment_Type_No_Assignment:
					break;
					
				case CxConstants.Server_Assignment_Type_Authentication_Failure:
				case CxConstants.Server_Assignment_Type_Authentication_Timeout:	
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
			session.close();
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

	public static final String cn_service_auth_s="<CoreNetworkServicesAuthorization>";
	public static final String cn_service_auth_e="</CoreNetworkServicesAuthorization>";
	public static final String subs_media_profile_id_s="<SubscribedMediaProfileId>";
	public static final String subs_media_profile_id_e="</SubscribedMediaProfileId>";
	public static final String shared_ifc_set_id_s="<Extension><SharedIFCSetID>";
	public static final String shared_ifc_set_id_e="</SharedIFCSetID></Extension>";

	public static final String zero="0";
	public static final String one="1";
	public static final String two="2";

	private static String downloadUserData(String privateIdentity, String publicIdentity, int id_implicit_set){
		Session session = HibernateUtil.getCurrentSession();
		List<IMPU> [] impu_array;
		SP[] sp_array;
		
		int sp_cnt = 0;
		List queryResult = IMPU_DAO.get_All_SP_For_Set(session, id_implicit_set);

		Iterator it = queryResult.iterator();
		int last_id_sp = -1; 
		while (it.hasNext()){
			Object [] row = (Object []) it.next();
			SP sp = (SP)row[1];
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
			IMPU impu = (IMPU) row[0];
			SP sp = (SP)row[1];
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
		
		//SP
		for (int i = 0; i < sp_array.length; i++){
			sb.append(service_profile_s);
			// PublicIdentity 					=> 1 to n
			
			it = impu_array[i].iterator();
			while (it.hasNext()){
				IMPU impu = (IMPU)it.next();
				sb.append(public_id_s);
				sb.append(identity_s);
				sb.append(impu.getIdentity());
				sb.append(identity_e);
				sb.append(public_id_e);
			}
			
			// InitialFilterCriteria 			=> 0 to n
			
			// CoreNetworkServiceAuthorization	=> 0 to n
			
			// Extension						=> 0 to 1
			sb.append(service_profile_e);
		}
		sb.append(ims_subscription_e);
		System.out.println("The XML document:\n" + sb.toString());
		return sb.toString();
	}
}
