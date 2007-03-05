/**
 * 
 */
package de.fhg.fokus.hss.cx.op;
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
import de.fhg.fokus.hss.db.model.IMPU_VisitedNetwork;
import de.fhg.fokus.hss.db.model.VisitedNetwork;
import de.fhg.fokus.hss.db.op.IMPI_DAO;
import de.fhg.fokus.hss.db.op.IMPI_IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMPU_VisitedNetwork_DAO;
import de.fhg.fokus.hss.db.op.VisitedNetworkDAO;
import de.fhg.fokus.hss.diam.DiameterConstants;
import de.fhg.fokus.hss.diam.UtilAVP;
import de.fhg.fokus.hss.util.HibernateUtil;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class UAR {

	public static DiameterMessage processRequest(DiameterPeer diameterPeer, DiameterMessage request){
		
		DiameterMessage response = diameterPeer.newResponse(request);
		UtilAVP.addAuthSessionState(response, DiameterConstants.AVPValue.ASS_No_State_Maintained);
		UtilAVP.addVendorSpecificApplicationID(response, DiameterConstants.Vendor.V3GPP, DiameterConstants.Application.Cx);
		
		try{
			long time1 = System.currentTimeMillis();
			System.out.println("Start:" + time1);
			HibernateUtil.beginTransaction();
			Session session = HibernateUtil.getCurrentSession();
						
			String publicIdentity = UtilAVP.getPublicIdentity(request);
			String privateIdentity = UtilAVP.getUserName(request);
			if (publicIdentity == null || privateIdentity == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);	
			}
			
			String visited_network_name = UtilAVP.getVisitedNetwork(request);
			if (visited_network_name == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
			}
			VisitedNetwork visited_network = VisitedNetworkDAO.getByIdentity(session, visited_network_name);

			// 1. check if the identities exist in hss
			IMPU impu = IMPU_DAO.getByIdentity(session, publicIdentity);
			IMPI impi = IMPI_DAO.getByIdentity(session, privateIdentity);
			if (impu == null || impi == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_UNKNOWN);
			}
			long time2 = System.currentTimeMillis();
			System.out.println("\n\nDelta:" + (time2-time1));

			// 2. check association
			IMPI_IMPU impi_impu = IMPI_IMPU_DAO.getByIMPI_IMPU(session, impi.getId(), impu.getId());
			if (impi_impu == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_IDENTITIES_DONT_MATCH);
			}
			
			// 3. check for IMPU if is barred
			if (impu.getBarring() == 1){
				List impuList = IMPU_DAO.getOthersFromSet(session, impu.getId(), impu.getId_impu_implicitset());
				if (impuList == null || impuList.size() == 0){
					throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_AUTHORIZATION_REJECTED);
				}
				Iterator it = impuList.iterator();
				boolean notBarred = false;
				while (it.hasNext()){
					IMPU nextImpu = (IMPU) it.next();
					if (nextImpu.getBarring() == 0){
						notBarred = true;
						break;
					}
				}
				if (!notBarred){
					throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_AUTHORIZATION_REJECTED);
				}
			}
			
			// 4. check the User Authorization Type
			int authorizationType = UtilAVP.getUserAuthorizationType(request); 
			switch (authorizationType){
			
				case DiameterConstants.AVPValue.UAT_Registration:
					IMPU_VisitedNetwork impu_visited_network = 
						IMPU_VisitedNetwork_DAO.getByIMPU_VisitedNetwork(session, impu.getId(), visited_network.getId());
					if (impu_visited_network == null){
						throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_ROAMING_NOT_ALLOWED);
					}
					if (impu.getCan_register() == 0){
						throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_AUTHORIZATION_REJECTED);
					}
					break;
					
				case DiameterConstants.AVPValue.UAT_De_Registration:
					break;
					
				case DiameterConstants.AVPValue.UAT_Registration_and_Capabilities:
					impu_visited_network = 
						IMPU_VisitedNetwork_DAO.getByIMPU_VisitedNetwork(session, impu.getId(), visited_network.getId());
					if (impu_visited_network == null){
						throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_ROAMING_NOT_ALLOWED);
					}
					if (impu.getCan_register() == 0){
						throw new CxFinalResultException("", 1);
					}
					UtilAVP.addServerCapabilities(response);
					UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
					break;	
			}

			String serverName = impi.getImsu().getScscf_name();
			
			// 5. check the state of the public identity
			switch (impu.getUser_state()){
			
				case CxConstants.IMPU_user_state_Registered:
					
					if (serverName != null && !serverName.equals(""))
						UtilAVP.addServerName(response, serverName);
					if (authorizationType == DiameterConstants.AVPValue.UAT_Registration){
						UtilAVP.addExperimentalResultCode(response, 
								DiameterConstants.ResultCode.RC_IMS_DIAMETER_SUBSEQUENT_REGISTRATION.getCode());
					}
					else if (authorizationType == DiameterConstants.AVPValue.UAT_De_Registration){
						UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
					}
					break;
					
				case CxConstants.IMPU_user_state_Unregistered:
					
					if (authorizationType == DiameterConstants.AVPValue.UAT_De_Registration){
						UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
					}
					else if (authorizationType == DiameterConstants.AVPValue.UAT_Registration){
						if (serverName != null && !serverName.equals(""))						
							UtilAVP.addServerName(response, serverName);	
						UtilAVP.addExperimentalResultCode(response, 
								DiameterConstants.ResultCode.RC_IMS_DIAMETER_SUBSEQUENT_REGISTRATION.getCode());
					}
					break;
					
				case CxConstants.IMPU_user_state_Not_Registered:
				case CxConstants.IMPU_user_state_Auth_Pending:
					
					if (authorizationType == DiameterConstants.AVPValue.UAT_De_Registration){
						UtilAVP.addExperimentalResultCode(response, 
								DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_IDENTITY_NOT_REGISTERED.getCode());
					}
					else if (authorizationType == DiameterConstants.AVPValue.UAT_Registration){
						
						List list = IMPI_IMPU_DAO.get_All_IMPU_of_IMSU_user_state(session, impi.getImsu().getId(), CxConstants.IMPU_user_state_Registered);
						if (list.size() > 0){
							if (serverName != null && !serverName.equals(""))						
								UtilAVP.addServerName(response, serverName);
							UtilAVP.addExperimentalResultCode(response, 
									DiameterConstants.ResultCode.RC_IMS_DIAMETER_SUBSEQUENT_REGISTRATION.getCode());
							break;
						}
						
						list = IMPI_IMPU_DAO.get_All_IMPU_of_IMSU_user_state(session, impi.getImsu().getId(), 
								CxConstants.IMPU_user_state_Unregistered);
						if (list.size() > 0){
							if (serverName != null && !serverName.equals(""))						
								UtilAVP.addServerName(response, serverName);
							UtilAVP.addExperimentalResultCode(response, 
									DiameterConstants.ResultCode.RC_IMS_DIAMETER_SUBSEQUENT_REGISTRATION.getCode());
							break;
						}
						list = IMPI_IMPU_DAO.get_All_IMPU_of_IMSU_user_state(session, impi.getImsu().getId(), CxConstants.IMPU_user_state_Auth_Pending);
						if (list.size() > 0 && (serverName != null && !serverName.equals(""))){
							
							UtilAVP.addServerName(response, serverName);
							UtilAVP.addExperimentalResultCode(response, 
									DiameterConstants.ResultCode.RC_IMS_DIAMETER_SUBSEQUENT_REGISTRATION.getCode());
							break;
						}
						
						//else add capabilities ....
						UtilAVP.addServerCapabilities(response);
						UtilAVP.addExperimentalResultCode(response, 
								DiameterConstants.ResultCode.RC_IMS_DIAMETER_FIRST_REGISTRATION.getCode());
					}
					break;
			}
		}
		catch(CxExperimentalResultException e){
			UtilAVP.addExperimentalResultCode(response, e.getErrorCode());
			e.printStackTrace();
		} 
		catch (CxFinalResultException e) {
			UtilAVP.addResultCode(response, e.getErrorCode());
			e.printStackTrace();
		}
		finally{
			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
		}
		return response;
	}
}
