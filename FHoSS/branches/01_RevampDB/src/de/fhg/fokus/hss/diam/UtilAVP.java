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

package de.fhg.fokus.hss.diam;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;

import org.hibernate.Session;


import de.fhg.fokus.diameter.DiameterPeer.data.AVP;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.hss.auth.AuthenticationVector;
import de.fhg.fokus.hss.cx.CxConstants;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.op.IMPU_DAO;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class UtilAVP {
	
	public static String getPublicIdentity(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.IMS_PUBLIC_IDENTITY, true, 
				DiameterConstants.Vendor.V3GPP);

		if (avp != null){
			return new String(avp.data);
		}
		return null;
	}
	
	public static String getUserName(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.USER_NAME, true, DiameterConstants.Vendor.DIAM);
		if (avp != null){
			return new String(avp.data);
		}
		return null;
	}
	
	public static int getUserAuthorizationType(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.IMS_USER_AUTHORIZATION_TYPE, true, 
				DiameterConstants.Vendor.V3GPP);
		if (avp != null){
			return avp.getIntData();
		}
		return 0;
	}

	public static String getVisitedNetwork(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.IMS_VISITED_NETWORK_IDENTIFIER, true, 
				DiameterConstants.Vendor.V3GPP);
		System.out.println("AVP is:"  + avp);
		if (avp != null){
			return new String(avp.data);
		}
		return null;
	}
	
	public static AVP getSipAuthDataItem(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.IMS_SIP_AUTH_DATA_ITEM, true, 
				DiameterConstants.Vendor.V3GPP);
		return avp;
	}
	
	public static String getServerName(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.IMS_SERVER_NAME, true, 
				DiameterConstants.Vendor.V3GPP);		
		if (avp != null){
			return new String(avp.data);
		}
		return null;
	}

	public static int getSipNumberAuthItems(DiameterMessage message){

		AVP avp = message.findAVP(DiameterConstants.AVPCode.IMS_SIP_NUMBER_AUTH_ITEMS, true, 
				DiameterConstants.Vendor.V3GPP);
		if (avp != null){
			return avp.int_data;
		}
		return -1;
	}
	
	public static String getOriginatingHost(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.ORIGIN_HOST, true, 
				DiameterConstants.Vendor.DIAM);
		if (avp != null){
			return new String (avp.data);
		}
		return null;
	}
	public static String getOriginatingRealm(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.ORIGIN_REALM, true, 
				DiameterConstants.Vendor.DIAM);
		if (avp != null){
			return new String(avp.data);
		}
		return null;
	}
	
	public static int getOriginatingRequest(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.IMS_ORIGINATING_REQUEST, false, 
				DiameterConstants.Vendor.V3GPP);
		if (avp != null){
			return avp.getIntData();
		}
		return -1;
		
	}
	
	public static String getDestinationHost(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.DESTINATION_HOST, true, 
				DiameterConstants.Vendor.DIAM);
		if (avp != null){
			return new String(avp.data);
		}
		return null;
	}
	
	public static String getDestinationRealm(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.DESTINATION_REALM, true, 
				DiameterConstants.Vendor.DIAM);
		if (avp != null){
			return new String(avp.data);
		}
		return null;
	}
	
	public static String getVendorSpecificApplicationID(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.VENDOR_SPECIFIC_APPLICATION_ID, true, 
				DiameterConstants.Vendor.DIAM);
		if (avp != null){
			return new String(avp.data);
		}
		return null;
	}
	
	public static String getAuthSessionState(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.AUTH_SESSION_STATE, true, 
				DiameterConstants.Vendor.DIAM);
		if (avp != null){
			return new String(avp.data);
		}
		return null;
	}
	
	public static int getServerAssignmentType(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.IMS_SERVER_ASSIGNMENT_TYPE, true, 
				DiameterConstants.Vendor.V3GPP);
		if (avp != null){
			return avp.getIntData();
		}
		return -1;
	}
	
	public static int getUserDataAlreadyAvailable(DiameterMessage message){
		AVP avp = message.findAVP(DiameterConstants.AVPCode.IMS_USER_DATA_ALREADY_AVAILABLE, true, 
				DiameterConstants.Vendor.V3GPP);
		if (avp != null){
			return avp.getIntData();
		}
		return -1;
	}

	public static List<IMPU> getAllIMPU(Session session, DiameterMessage message){
		List<IMPU> impuIdentitiesList = null;
		Vector avpVector = message.avps;
		if (avpVector != null){
			Iterator it = avpVector.iterator();
			while (it.hasNext()){
				AVP currentAVP = (AVP) it.next();
				if (currentAVP.code == DiameterConstants.AVPCode.IMS_PUBLIC_IDENTITY){
					if (impuIdentitiesList == null){
						impuIdentitiesList = new ArrayList<IMPU>();
					}
					String identity = new String(currentAVP.data);
					IMPU impu = IMPU_DAO.get_by_Identity(session, identity);
					impuIdentitiesList.add(impu);
				}
			}
			return impuIdentitiesList;
		}
		return null;
	}
	
	
	
	public static void addServerCapabilities(DiameterMessage message){
		//....
	}

	public static void addResultCode(DiameterMessage message, int resultCode){
		AVP avp = new AVP(DiameterConstants.AVPCode.RESULT_CODE, true, DiameterConstants.Vendor.DIAM);
		avp.setData(resultCode);
		message.addAVP(avp);
	}
	
	public static void addServerName(DiameterMessage message, String serverName){
		AVP avp = new AVP(DiameterConstants.AVPCode.IMS_SERVER_NAME, true, DiameterConstants.Vendor.V3GPP);
		avp.setData(serverName);
		message.addAVP(avp);
	}
	
	public static void addExperimentalResultCode(DiameterMessage message, int resultCode){
		AVP group = new AVP(DiameterConstants.AVPCode.EXPERIMENTAL_RESULT, true, DiameterConstants.Vendor.DIAM);
		AVP vendorID = new AVP(DiameterConstants.AVPCode.VENDOR_ID, true, DiameterConstants.Vendor.DIAM);
		vendorID.setData(DiameterConstants.Vendor.V3GPP);
		group.addChildAVP(vendorID);
		
		AVP expResult = new AVP(DiameterConstants.AVPCode.EXPERIMENTAL_RESULT_CODE, true, DiameterConstants.Vendor.DIAM); 
		expResult.setData(resultCode);
		group.addChildAVP(expResult);
		message.addAVP(group);
	}
	
	public static void addVendorSpecificApplicationID(DiameterMessage message, int vendorID, int appID){
        AVP vendorAppID_AVP = new AVP(DiameterConstants.AVPCode.VENDOR_SPECIFIC_APPLICATION_ID, true, DiameterConstants.Vendor.DIAM);
        
        AVP vendorID_AVP = new AVP(DiameterConstants.AVPCode.VENDOR_ID, true, DiameterConstants.Vendor.DIAM);
        vendorID_AVP.setData(vendorID);
        vendorAppID_AVP.addChildAVP(vendorID_AVP);
        
        AVP appID_AVP = new AVP(DiameterConstants.AVPCode.AUTH_APPLICATION_ID, true,  DiameterConstants.Vendor.DIAM);
        appID_AVP.setData(appID);
        vendorAppID_AVP.addChildAVP(appID_AVP);
        
        message.addAVP(vendorAppID_AVP);
	}

	public static void addAuthSessionState(DiameterMessage message, int state){
	    AVP authSession = new AVP(DiameterConstants.AVPCode.AUTH_SESSION_STATE, true, DiameterConstants.Vendor.DIAM);
	    authSession.setData(state);
	    message.addAVP(authSession);
	}

	public static void addPublicIdentity(DiameterMessage message, String publicIdentity){
	    AVP publicIdentityAVP = new AVP(DiameterConstants.AVPCode.IMS_PUBLIC_IDENTITY, 
	    		true, DiameterConstants.Vendor.V3GPP);
	    publicIdentityAVP.setData(publicIdentity);
	    message.addAVP(publicIdentityAVP);
	}
	
	public static void addUserName(DiameterMessage message, String userName){
	    AVP userNameAVP = new AVP(DiameterConstants.AVPCode.USER_NAME, 
	    		true, DiameterConstants.Vendor.DIAM);
	    userNameAVP.setData(userName);
	    message.addAVP(userNameAVP);
	}
	
	public static void addAuthVectors(DiameterMessage message, List avList){
		if (avList == null){
			return;
		}
		
		Iterator it = avList.iterator();
		int itemNo = 1;
		while (it.hasNext()){
			AuthenticationVector av = (AuthenticationVector) it.next();
			
            AVP authDataItem = new AVP(DiameterConstants.AVPCode.IMS_SIP_AUTH_DATA_ITEM, true, DiameterConstants.Vendor.V3GPP);
            
            AVP itemNumber = new AVP(DiameterConstants.AVPCode.IMS_SIP_ITEM_NUMBER, true, DiameterConstants.Vendor.V3GPP);
            itemNumber.setData(itemNo++);
            authDataItem.addChildAVP(itemNumber);

            AVP authScheme = new AVP(DiameterConstants.AVPCode.IMS_SIP_AUTHENTICATION_SCHEME, 
            		true, DiameterConstants.Vendor.V3GPP);
            
            switch (av.getAuth_scheme()){
            case 1:
            	authScheme.setData(CxConstants.AuthScheme.Auth_Scheme_AKAv1.getName());
            	break;
            case 2:
            	authScheme.setData(CxConstants.AuthScheme.Auth_Scheme_AKAv2.getName());
            	break;
            case 3:
            	authScheme.setData(CxConstants.AuthScheme.Auth_Scheme_MD5.getName());
            	break;
            case 4:
            	authScheme.setData(CxConstants.AuthScheme.Auth_Scheme_Early.getName());
            	break;
            }
            
            authDataItem.addChildAVP(authScheme);
            
            if(((av.getAuth_scheme() & CxConstants.AuthScheme.Auth_Scheme_Early.getCode()) != 0)){
                AVP ip = new AVP(DiameterConstants.AVPCode.FRAMED_IP_ADDRESS, true, DiameterConstants.Vendor.V3GPP);
                ip.setData((av.getIp()));
                authDataItem.addChildAVP(ip);
            }
            else{
                AVP authenticate = new AVP(DiameterConstants.AVPCode.IMS_SIP_AUTHENTICATE, true, 
                		DiameterConstants.Vendor.V3GPP);
                authenticate.setData(av.getSipAuthenticate());
                authDataItem.addChildAVP(authenticate);

                AVP authorization = new AVP(DiameterConstants.AVPCode.IMS_SIP_AUTHORIZATION, true, 
                		DiameterConstants.Vendor.V3GPP);
                authorization.setData(av.getSipAuthorization());
                authDataItem.addChildAVP(authorization);

                AVP confidentialityKey = new AVP(DiameterConstants.AVPCode.IMS_CONFIDENTIALITY_KEY, true, 
                		DiameterConstants.Vendor.V3GPP);
                confidentialityKey.setData(av.getConfidentialityityKey());
                authDataItem.addChildAVP(confidentialityKey);

                AVP integrityKey = new AVP(DiameterConstants.AVPCode.IMS_INTEGRITY_KEY, true, 
                		DiameterConstants.Vendor.V3GPP);
                integrityKey.setData(av.getIntegrityKey());
                authDataItem.addChildAVP(integrityKey);

                message.addAVP(authDataItem);
            }
		}
	}
	
	public static void addUserData(DiameterMessage message, String data){
		AVP userData = new AVP(DiameterConstants.AVPCode.IMS_USER_DATA, true, DiameterConstants.Vendor.V3GPP);
		userData.setData(data);
		message.addAVP(userData);
	}
	
	public static void addAsssociatedIdentities(DiameterMessage message, List identitiesList){
		AVP associatedIdentities = new AVP(DiameterConstants.AVPCode.IMS_ASSOCIATED_IDENTITIES, false, 
			DiameterConstants.Vendor.V3GPP);
		
		AVP userName;
		Iterator it = identitiesList.iterator();
		while (it.hasNext()){
			IMPI impi =  (IMPI) it.next();
			userName = new AVP(DiameterConstants.AVPCode.USER_NAME, true, DiameterConstants.Vendor.DIAM);
			userName.setData(impi.getIdentity());
			associatedIdentities.addChildAVP(userName);
		}
		message.addAVP(associatedIdentities);
	}

	public static void addDeregistrationReason(DiameterMessage message, int reasonCode, String reasonInfo){
		AVP deregistrationReasonAVP = new AVP(DiameterConstants.AVPCode.IMS_DEREGISTRATION_REASON, true, 
				DiameterConstants.Vendor.V3GPP);

		AVP reasonCodeAVP = new AVP(DiameterConstants.AVPCode.IMS_REASON_CODE, true, 
				DiameterConstants.Vendor.V3GPP);
		reasonCodeAVP.setData(reasonCode);
		deregistrationReasonAVP.addChildAVP(reasonCodeAVP);
		
		AVP reasonInfoAVP = new AVP(DiameterConstants.AVPCode.IMS_REASON_INFO, true, 
				DiameterConstants.Vendor.V3GPP);
		reasonInfoAVP.setData(reasonInfo);
		deregistrationReasonAVP.addChildAVP(reasonInfoAVP);
		
		message.addAVP(deregistrationReasonAVP);
	}
	
	public static void addDestinationHost(DiameterMessage message, String host){
		AVP destHostAVP = new AVP(DiameterConstants.AVPCode.DESTINATION_HOST, true, 
				DiameterConstants.Vendor.DIAM);
		destHostAVP.setData(host);
		message.addAVP(destHostAVP);
	}
	
	public static void addDestinationRealm(DiameterMessage message, String realm){
		AVP destRealm = new AVP(DiameterConstants.AVPCode.DESTINATION_REALM, true,
				DiameterConstants.Vendor.DIAM);
		destRealm.setData(realm);
		message.addAVP(destRealm);
	}
}
