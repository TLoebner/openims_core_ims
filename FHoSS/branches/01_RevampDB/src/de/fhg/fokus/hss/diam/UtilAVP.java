/**
 * 
 */
package de.fhg.fokus.hss.diam;

import java.util.Iterator;
import java.util.List;

import de.fhg.fokus.diameter.DiameterPeer.data.AVP;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.hss.auth.AuthenticationVector;
import de.fhg.fokus.hss.cx.CxConstants;

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
	
	
}
