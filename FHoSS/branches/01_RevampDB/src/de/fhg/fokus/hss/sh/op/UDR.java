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

import java.util.Iterator;
import java.util.List;
import java.util.Vector;

import org.apache.log4j.Logger;
import org.hibernate.Session;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;
import de.fhg.fokus.diameter.DiameterPeer.data.AVP;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.hss.db.hibernate.DatabaseException;
import de.fhg.fokus.hss.db.hibernate.HibernateUtil;
import de.fhg.fokus.hss.db.model.ApplicationServer;
import de.fhg.fokus.hss.db.model.ChargingInfo;
import de.fhg.fokus.hss.db.model.IFC;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.RepositoryData;
import de.fhg.fokus.hss.db.model.SPT;
import de.fhg.fokus.hss.db.model.SP_IFC;
import de.fhg.fokus.hss.db.model.TP;

import de.fhg.fokus.hss.db.op.ApplicationServer_DAO;
import de.fhg.fokus.hss.db.op.ChargingInfo_DAO;
import de.fhg.fokus.hss.db.op.IFC_DAO;
import de.fhg.fokus.hss.db.op.IMPI_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMSU_DAO;
import de.fhg.fokus.hss.db.op.RepositoryData_DAO;
import de.fhg.fokus.hss.db.op.SPT_DAO;
import de.fhg.fokus.hss.db.op.SP_IFC_DAO;
import de.fhg.fokus.hss.db.op.TP_DAO;
import de.fhg.fokus.hss.diam.DiameterConstants;
import de.fhg.fokus.hss.diam.UtilAVP;
import de.fhg.fokus.hss.sh.ShConstants;
import de.fhg.fokus.hss.sh.ShExperimentalResultException;
import de.fhg.fokus.hss.sh.ShFinalResultException;

import de.fhg.fokus.hss.sh.data.*;
/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class UDR {
	private static Logger logger = Logger.getLogger(UDR.class);
	
	public static DiameterMessage processRequest(DiameterPeer diameterPeer, DiameterMessage request){
		DiameterMessage response = diameterPeer.newResponse(request);
		Session session = null;
		boolean dbException = false;
		
		// add Auth-Session-State and Vendor-Specific-Application-ID
		UtilAVP.addAuthSessionState(response, DiameterConstants.AVPValue.ASS_No_State_Maintained);
		UtilAVP.addVendorSpecificApplicationID(response, DiameterConstants.Vendor.V3GPP, DiameterConstants.Application.Sh);

		try{
			if (request.flagProxiable == false){
				logger.warn("You should notice that the Proxiable flag for UDR request was not set!");
			}
			// set the proxiable flag for the response
			response.flagProxiable = true;
			
			// -0- check for mandatory fields in the message
			String vendor_specific_ID = UtilAVP.getVendorSpecificApplicationID(request);
			String auth_session_state = UtilAVP.getAuthSessionState(request);
			String origin_host = UtilAVP.getOriginatingHost(request);
			String origin_realm = UtilAVP.getOriginatingRealm(request);
			String dest_realm = UtilAVP.getDestinationRealm(request);
			
			String user_identity = UtilAVP.getShUserIdentity(request);;
			Vector data_ref_vector = UtilAVP.getAllDataReference(request);
			
			if (vendor_specific_ID == null || auth_session_state == null || origin_host == null || origin_realm == null ||
					dest_realm == null || user_identity == null || data_ref_vector == null || data_ref_vector.size() == 0){
				throw new ShExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
			}
			session = HibernateUtil.getCurrentSession();
			HibernateUtil.beginTransaction();

			// -1-
			ApplicationServer as = ApplicationServer_DAO.get_by_Diameter_Address(session, origin_host);
			if (as == null){
				throw new ShExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_DATA_CANNOT_BE_READ);
			}
			
			if (as.getUdr() == 0){
				throw new ShExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_DATA_CANNOT_BE_READ);				
			}
			
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
						(crt_data_ref == ShConstants.Data_Ref_Aliases_Repository_Data && as.getUdr_aliases_rep_data() == 0)){
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
			
			ShDataElement shData = new ShDataElement();
			
			for (int i = 0; i < data_ref_vector.size(); i++){
				int crt_data_ref = (Integer) data_ref_vector.get(i); 
				ShIMSDataElement shIMSData = null;
				
				switch (crt_data_ref){
					case  ShConstants.Data_Ref_Repository_Data:
						String service_indication = UtilAVP.getServiceIndication(request);
						if (service_indication == null){
							throw new ShExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
						}
						RepositoryData repData = RepositoryData_DAO.get_by_IMPU_and_ServiceIndication(session, user_identity, service_indication);
						RepositoryDataElement repDataElement = new RepositoryDataElement();
						repDataElement.setServiceData(repData.getRep_data());
						repDataElement.setSqn(repData.getSqn());
						repDataElement.setServiceIndication(service_indication);
						shData.addRepositoryData(repDataElement);
						break;
						
					case  ShConstants.Data_Ref_IMS_Public_Identity:
						
						break;
						
					case  ShConstants.Data_Ref_IMS_User_State:
						shIMSData = shData.getShIMSData();
						if (shIMSData == null){
							shIMSData = new ShIMSDataElement();
							shData.setShIMSData(shIMSData);
						}
						shIMSData.setImsUserState(impu.getUser_state());
						break;
						
					case  ShConstants.Data_Ref_SCSCF_Name:
						shIMSData = shData.getShIMSData();
						if (shIMSData == null){
							shIMSData = new ShIMSDataElement();
							shData.setShIMSData(shIMSData);
						}
						
						List impiList = IMPU_DAO.get_all_IMPI_for_IMPU_ID(session, impu.getId());
						if (impiList != null && impiList.size() > 0){
							IMPI impi = (IMPI)impiList.get(0);
							 shIMSData.setScscfName(IMSU_DAO.get_SCSCF_Name_by_IMSU_ID(session, impi.getId_imsu()));
						}
						break;
						
					case  ShConstants.Data_Ref_iFC:
						shIMSData = shData.getShIMSData();
						if (shIMSData == null){
							shIMSData = new ShIMSDataElement();
							shData.setShIMSData(shIMSData);
						}
						
						String server_name = UtilAVP.getServerName(request);
						ApplicationServer serviceAS = ApplicationServer_DAO.get_by_Server_Name(session, server_name);
						
						List ifcList = SP_IFC_DAO.get_all_IFC_by_SP_ID(session, impu.getId_sp());
						if (ifcList != null){
							ApplicationServerElement asElement = new ApplicationServerElement();
							asElement.setDefaultHandling(serviceAS.getDefault_handling());
							asElement.setServerName(serviceAS.getServer_name());
							asElement.setServiceInfo(serviceAS.getService_info());
							
							Iterator it = ifcList.iterator();
							while (it.hasNext()){
								IFC crt_ifc = (IFC) it.next();
								if (crt_ifc.getId_application_server() == serviceAS.getId()){
									InitialFilterCriteriaElement ifcElement = new InitialFilterCriteriaElement();
									
									ifcElement.setApplicationServer(asElement);
									SP_IFC sp_ifc = SP_IFC_DAO.get_by_SP_and_IFC_ID(session, impu.getId_sp(), crt_ifc.getId());
									ifcElement.setPriority(sp_ifc.getPriority());
									ifcElement.setProfilePartIndicator(crt_ifc.getProfile_part_ind());

									// set the trigger point
									TP tp = TP_DAO.get_by_ID(session, crt_ifc.getId_tp());
									TriggerPointElement tpElement = new TriggerPointElement();
									tpElement.setConditionTypeCNF(tp.getCondition_type_cnf());
									
									List sptList = SPT_DAO.get_all_by_TP_ID(session, tp.getId());
									if (sptList != null){
										Iterator it2 = sptList.iterator();
										SPT crt_spt;
										SPTElement sptElement;
										while (it2.hasNext()){
											crt_spt = (SPT) it2.next();
											sptElement = new SPTElement();
											sptElement.setConditionNegated(crt_spt.getCondition_negated());
											sptElement.setGroupID(crt_spt.getGrp());
											sptElement.setMethod(crt_spt.getMethod());
											sptElement.setRequestURI(crt_spt.getRequesturi());
											sptElement.setSessionCase(crt_spt.getSession_case());
											sptElement.setSessionDescLine(crt_spt.getSdp_line());
											sptElement.setSessionDescContent(crt_spt.getSdp_line_content());
											sptElement.setSipHeader(crt_spt.getHeader());
											sptElement.setSipHeaderContent(crt_spt.getHeader_content());
											
											// extension
											sptElement.addRegistrationType(crt_spt.getRegistration_type());
											tpElement.addSPT(sptElement);
										}
									}
									ifcElement.setTriggerPoint(tpElement);
									shIMSData.addInitialFilterCriteria(ifcElement);
								}
							}
						}
						break;
						
					case  ShConstants.Data_Ref_Location_Info:
						break;

					case  ShConstants.Data_Ref_User_State:
						break;

					case  ShConstants.Data_Ref_Charging_Info:
						if (shIMSData == null){
							shIMSData = new ShIMSDataElement();
							shData.setShIMSData(shIMSData);
						}
						ChargingInfo chgInfo = ChargingInfo_DAO.get_by_ID(session, impu.getId());
						
						ChargingInformationElement chgInfoElement = new ChargingInformationElement();
						chgInfoElement.setPriCCFName(chgInfo.getPri_ccf());
						chgInfoElement.setSecCCFName(chgInfo.getSec_ccf());
						chgInfoElement.setPriECFName(chgInfo.getPri_ecf());
						chgInfoElement.setSecECFName(chgInfo.getSec_ecf());
						shIMSData.setChgInformation(chgInfoElement);
						break;
						
					case  ShConstants.Data_Ref_MSISDN:
						break;
						
					case  ShConstants.Data_Ref_PSI_Activation:
						if (shIMSData == null){
							shIMSData = new ShIMSDataElement();
							shData.setShIMSData(shIMSData);
						}
						shIMSData.setPsiActivation(impu.getPsi_activation());
						break;
						
					case  ShConstants.Data_Ref_DSAI:
						
						break;

					case  ShConstants.Data_Ref_Aliases_Repository_Data:
						service_indication = UtilAVP.getServiceIndication(request);
						if (service_indication == null){
							throw new ShExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
						}
						repData = RepositoryData_DAO.get_by_IMPU_and_ServiceIndication(session, user_identity, service_indication);
						repDataElement = new RepositoryDataElement();
						repDataElement.setServiceData(repData.getRep_data());
						repDataElement.setSqn(repData.getSqn());
						repDataElement.setServiceIndication(service_indication);
						shData.addRepositoryData(repDataElement);
						break;
				}
				
			}
			UtilAVP.addUserDataSh(response, shData.toString());
			UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
		}
		catch (DatabaseException e){
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
