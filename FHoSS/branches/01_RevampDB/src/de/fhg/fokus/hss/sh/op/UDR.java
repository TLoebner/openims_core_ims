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

import org.hibernate.Session;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;
import de.fhg.fokus.diameter.DiameterPeer.data.AVP;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.hss.db.hibernate.DatabaseException;
import de.fhg.fokus.hss.db.hibernate.HibernateUtil;
import de.fhg.fokus.hss.db.model.ApplicationServer;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.op.ApplicationServer_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
import de.fhg.fokus.hss.diam.DiameterConstants;
import de.fhg.fokus.hss.diam.UtilAVP;
import de.fhg.fokus.hss.sh.ShConstants;
import de.fhg.fokus.hss.sh.ShExperimentalResultException;
import de.fhg.fokus.hss.sh.ShFinalResultException;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class UDR {
	public static DiameterMessage processRequest(DiameterPeer diameterPeer, DiameterMessage request){
		DiameterMessage response = diameterPeer.newResponse(request);
		Session session = null;
		
		// add Auth-Session-State and Vendor-Specific-Application-ID
		UtilAVP.addAuthSessionState(response, DiameterConstants.AVPValue.ASS_No_State_Maintained);
		UtilAVP.addVendorSpecificApplicationID(response, DiameterConstants.Vendor.V3GPP, DiameterConstants.Application.Sh);

		try{

			// -0- check for mandatory fields in the message
			String vendor_specific_ID = UtilAVP.getVendorSpecificApplicationID(request);
			String auth_session_state = UtilAVP.getAuthSessionState(request);
			String origin_host = UtilAVP.getOriginatingHost(request);
			String origin_realm = UtilAVP.getOriginatingRealm(request);
			String dest_realm = UtilAVP.getDestinationRealm(request);
			AVP user_identity_avp = UtilAVP.getUserIdentity(request);
			String user_identity = new String(user_identity_avp.data);
			Vector data_ref_vector = UtilAVP.getAllDataReference(request);
			
			if (vendor_specific_ID == null || auth_session_state == null || origin_host == null || origin_realm == null ||
					dest_realm == null || user_identity_avp == null || data_ref_vector == null || data_ref_vector.size() == 0){
				throw new ShExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
			}

			// -1-
			ApplicationServer as = ApplicationServer_DAO.get_by_Server_Name(session, origin_host);
			for (int i = 0; i < data_ref_vector.size(); i++){
				int crt_data_ref = (Integer) data_ref_vector.get(i); 

				if ((crt_data_ref == ShConstants.Data_Ref_Repository_Data && as.getUdr_rep_data() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_IMS_Public_Identity && as.getUdr_impu() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_IMS_User_State && as.getUdr_ims_user_state() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_SCSCF_Name && as.getUdr_scscf_name() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_iFC && as.getUdr_ifc() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_Location_Info && as.getUdr_location() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_User_State && as.getUdr_user_state() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_Charging_Info && as.getUdr_charging_info() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_MSISDN && as.getUdr_msisdn() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_PSI_Activation && as.getUdr_psi_activation() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_DSAI && as.getUdr_dsai() == 0) ||
						(crt_data_ref == ShConstants.Data_Ref_Repository_Data && as.getUdr_rep_data() == 0)){
							throw new ShExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_DATA_CANNOT_BE_READ);
				}
			}
			
			// -2- check for user identity existence
			IMPU impu = IMPU_DAO.get_by_Identity(session, user_identity);
			if (impu == null){
				throw new ShExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_USER_UNKNOWN);				
			}
			
			// -3-
			// check if the user identity apply to the Data Reference according to table 7.6.1
			
			
			// -3a-
			// [to be completed]
			
			// -4- check for simultaneous update
			// [to be completed]
			
			// -5- include the data pertinent to the requested Data Reference
			//
			
			UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
			
		}
		catch (DatabaseException e){
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
			HibernateUtil.commitTransaction();
			session.close();
		}
		
		return response;
	}

}
