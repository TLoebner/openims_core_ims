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
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.IMSU;
import de.fhg.fokus.hss.db.op.IMPI_IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
import de.fhg.fokus.hss.db.op.SP_IFC_DAO;
import de.fhg.fokus.hss.diam.DiameterConstants;
import de.fhg.fokus.hss.diam.UtilAVP;
import de.fhg.fokus.hss.util.HibernateUtil;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class LIR {
	public static DiameterMessage processRequest(DiameterPeer diameterPeer, DiameterMessage request){
		DiameterMessage response = diameterPeer.newResponse(request);

		// add Auth-Session-State and Vendor-Specific-Application-ID
		UtilAVP.addAuthSessionState(response, DiameterConstants.AVPValue.ASS_No_State_Maintained);
		UtilAVP.addVendorSpecificApplicationID(response, DiameterConstants.Vendor.V3GPP, DiameterConstants.Application.Cx);
		
		Session session = null;
		try{
			// obtain the hibernate session
			session = HibernateUtil.getCurrentSession();
			
			// get the needed AVPs
			String publicIdentity = UtilAVP.getPublicIdentity(request);
			if (publicIdentity == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
			}
			int originatingRequest = UtilAVP.getOriginatingRequest(request);
			
			// 1. check that the public identity is known
			IMPU impu = IMPU_DAO.getByIdentity(session, publicIdentity);
			if (impu == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_UNKNOWN); 
			}
			
			// 2. check if public identity is PSI; if PSI then test activation
			int type = impu.getType();
			if (type == CxConstants.IMPU_type_Distinct_PSI || type == CxConstants.IMPU_type_Wildcarded_PSI){
				if (impu.getPsi_activation() == 0){
					throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_UNKNOWN); 
				}
			}
			
			// 3. check the state of public identity
			int user_state = impu.getUser_state();

			List impi_impu_list = IMPI_IMPU_DAO.getJoinByIMPU(session, impu.getId());
			if (impi_impu_list == null){
				throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY);
			}
				
			Iterator it = impi_impu_list.iterator();
			String scscf_name = null;
			while (it.hasNext()){
				Object [] resultRow = (Object []) it.next();
				IMPI impi = (IMPI) resultRow[1];
				//one impi is enough to find out the associated imsu
				IMSU imsu = impi.getImsu();
				scscf_name = imsu.getScscf_name();
				break;
			}
			
			switch (user_state){
			
				case CxConstants.IMPU_user_state_Registered:
					if (scscf_name == null){
						throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY);
					}
					UtilAVP.addServerName(response, scscf_name);
					UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
					break;
			
				case CxConstants.IMPU_user_state_Unregistered:
					
					boolean unregistered_services = false;
					if (SP_IFC_DAO.getUnregisteredServicesCount(session, impu.getSp().getId()) > 0){
						unregistered_services = true;
					}
					
					if (originatingRequest == 1 || unregistered_services == true){
						UtilAVP.addServerName(response, scscf_name);
						UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
					}
					break;

				case CxConstants.IMPU_user_state_Not_Registered:
					unregistered_services = false;
					if (SP_IFC_DAO.getUnregisteredServicesCount(session, impu.getSp().getId()) > 0){
						unregistered_services = true;
					}
					
					if (originatingRequest == 1 || unregistered_services == true){
						if (scscf_name != null){
							UtilAVP.addServerName(response, scscf_name);
							UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
							break;
						}
						else{
							UtilAVP.addExperimentalResultCode(response,
									DiameterConstants.ResultCode.RC_IMS_DIAMETER_UNREGISTERED_SERVICE.getCode());
						}
					}
					else{
						UtilAVP.addExperimentalResultCode(response, 
								DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_IDENTITY_NOT_REGISTERED.getCode());
					}
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
}
