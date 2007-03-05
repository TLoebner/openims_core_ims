/**
 * 
 */
package de.fhg.fokus.hss.cx.op;

import java.net.Inet4Address;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Vector;

import org.apache.log4j.Logger;
import org.hibernate.Session;

import org.apache.log4j.Logger;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;
import de.fhg.fokus.diameter.DiameterPeer.data.AVP;
import de.fhg.fokus.diameter.DiameterPeer.data.AVPDecodeException;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.hss.cx.CxConstants;
import de.fhg.fokus.hss.cx.CxExperimentalResultException;
import de.fhg.fokus.hss.cx.CxFinalResultException;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPI_IMPU;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.op.IMPI_DAO;
import de.fhg.fokus.hss.db.op.IMPI_IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMSU_DAO;
import de.fhg.fokus.hss.diam.DiameterConstants;
import de.fhg.fokus.hss.diam.UtilAVP;
import de.fhg.fokus.hss.main.HSSProperties;

import de.fhg.fokus.hss.util.HibernateUtil;
import de.fhg.fokus.hss.auth.*;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class MAR {
	
	private static Logger logger = Logger.getLogger(MAR.class);
	
	public static DiameterMessage processRequest(DiameterPeer diameterPeer, DiameterMessage request){
		Session session = null;
		DiameterMessage response = diameterPeer.newResponse(request);
		UtilAVP.addAuthSessionState(response, DiameterConstants.AVPValue.ASS_No_State_Maintained);
		UtilAVP.addVendorSpecificApplicationID(response, DiameterConstants.Vendor.V3GPP, DiameterConstants.Application.Cx);
		
		try{
			HibernateUtil.beginTransaction();
			session = HibernateUtil.getCurrentSession();
			
			String publicIdentity = UtilAVP.getPublicIdentity(request);
			String privateIdentity = UtilAVP.getUserName(request);
			if (publicIdentity == null || privateIdentity == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);	
			}

			// 1. check if the identities exist in HSS
			IMPU impu = IMPU_DAO.getByIdentity(session, publicIdentity);
			IMPI impi = IMPI_DAO.getByIdentity(session, privateIdentity);
			if (impu == null || impi == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_USER_UNKNOWN);
			}
			
			// 2. check association
			IMPI_IMPU impi_impu = IMPI_IMPU_DAO.getByIMPI_IMPU(session, impi.getId(), impu.getId());
			if (impi_impu == null){
				throw new CxExperimentalResultException(
						DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_IDENTITIES_DONT_MATCH);
			}
			
			int auth_scheme = -1;
			byte[] authorization = null;
			AVP authDataItem = UtilAVP.getSipAuthDataItem(request);
			if (authDataItem == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
			}
			
			try {
				authDataItem.ungroup();
			} catch (AVPDecodeException e) {
				e.printStackTrace();
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
			}
			System.out.println("Child count: " + authDataItem.getChildCount());
			Vector childs = authDataItem.childs;
			if (childs == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
			}
			
			Iterator it = childs.iterator();			
			while (it.hasNext()){
				AVP child = (AVP)it.next();
				if (child.code == DiameterConstants.AVPCode.IMS_SIP_AUTHENTICATION_SCHEME){
					String data = new String(child.getData());
					if (data.equals(CxConstants.AuthScheme.Auth_Scheme_AKAv1.getName())){
						auth_scheme = CxConstants.AuthScheme.Auth_Scheme_AKAv1.getCode(); 
					}
					else if (data.equals(CxConstants.AuthScheme.Auth_Scheme_AKAv2.getName())){
						auth_scheme = CxConstants.AuthScheme.Auth_Scheme_AKAv2.getCode();
					}
					else if (data.equals(CxConstants.AuthScheme.Auth_Scheme_MD5.getName())){
						auth_scheme = CxConstants.AuthScheme.Auth_Scheme_MD5.getCode();
					}
					else if (data.equals(CxConstants.AuthScheme.Auth_Scheme_Early.getName())){
						auth_scheme = CxConstants.AuthScheme.Auth_Scheme_Early.getCode();
					}
				}
				else if (child.code == DiameterConstants.AVPCode.IMS_SIP_AUTHORIZATION){
					authorization = child.data;
				}
			}
			// 3. check if the Authentication-Scheme is supported
			
			if (auth_scheme == -1){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
			}
			
			if ((auth_scheme & impi.getAuth_scheme()) == 0){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.RC_IMS_DIAMETER_ERROR_AUTH_SCHEME_NOT_SUPPORTED);
			}
			
			// 4. Synchronisation
			String scscf_name = impi.getImsu().getScscf_name();
			String server_name = UtilAVP.getServerName(request);
			if (server_name == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
			}
			
			if (authorization != null){
				if (scscf_name == null || scscf_name.equals("")) {
					logger.error("Synchronization for: " + impi.getIdentity() + " but no scscf_name in db!");
					throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY);
				}
				 
				if (server_name.equals(scscf_name)){
					AuthenticationVector av = null;
					if ((av = synchronize(session, authorization, impi)) == null){
						throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY);
					}
					UtilAVP.addPublicIdentity(response, publicIdentity);
					UtilAVP.addUserName(response, privateIdentity);
					List avList = new LinkedList();
					avList.add(av);
					UtilAVP.addAuthVectors(response, avList);
					UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
					return response;
				}
			}
			
			
			// 5. check the registration status
			
			short user_state = impu.getUser_state();
			int av_count = UtilAVP.getSipNumberAuthItems(request);
			if (av_count == -1){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
			}
			String orig_host = UtilAVP.getOriginatingHost(request);
			if (orig_host == null){
				throw new CxExperimentalResultException(DiameterConstants.ResultCode.DIAMETER_MISSING_AVP);
			}
			switch (user_state){
			
				case CxConstants.IMPU_user_state_Registered:
					if (scscf_name == null || scscf_name.equals("")) {
						throw new CxFinalResultException(DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY);
					}
					if (!scscf_name.equals(server_name)){
						IMSU_DAO.update(session, impi.getImsu().getId(), server_name, orig_host);
						IMPU_DAO.update(session, impu.getId(), CxConstants.IMPU_user_state_Auth_Pending);
						UtilAVP.addPublicIdentity(response, publicIdentity);
						UtilAVP.addUserName(response, privateIdentity);
						
						List avList = MAR.generateAuthVectors(session, impi, auth_scheme, av_count);
						if (avList != null){
							UtilAVP.addAuthVectors(response, avList);
							UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
						}
						else{
							UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY.getCode());
						}
					}
					else{
						UtilAVP.addPublicIdentity(response, publicIdentity);
						UtilAVP.addUserName(response, privateIdentity);

						List avList = MAR.generateAuthVectors(session, impi, auth_scheme, av_count);
						if (avList != null){
							UtilAVP.addAuthVectors(response, avList);
							UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
						}
						else{
							UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY.getCode());
						}
					}
					break;
					
				case CxConstants.IMPU_user_state_Unregistered:
				case CxConstants.IMPU_user_state_Not_Registered:
				case CxConstants.IMPU_user_state_Auth_Pending:
					System.out.println("AICI: 1");
					System.out.println("Scscf_name: " + scscf_name);
					System.out.println("server_name: " + server_name);
					if (scscf_name == null || scscf_name.equals("") || !scscf_name.equals(server_name)){
						System.out.println("AICI: 11");
						IMSU_DAO.update(session, impi.getImsu().getId(), server_name, orig_host);
						IMPU_DAO.update(session, impu.getId(), CxConstants.IMPU_user_state_Auth_Pending);
						UtilAVP.addPublicIdentity(response, publicIdentity);
						UtilAVP.addUserName(response, privateIdentity);
						
						List avList = MAR.generateAuthVectors(session, impi, auth_scheme, av_count);
						if (avList != null){
							UtilAVP.addAuthVectors(response, avList);
							UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
						}
						else{
							System.out.println("AICI: 1111");
							UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY.getCode());
						}
					}
					else{
						UtilAVP.addPublicIdentity(response, publicIdentity);
						UtilAVP.addUserName(response, privateIdentity);
						System.out.println("AICI: 21");
						List avList = MAR.generateAuthVectors(session, impi, auth_scheme, av_count);
						if (avList != null){
							UtilAVP.addAuthVectors(response, avList);
							UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_SUCCESS.getCode());
						}
						else{
							System.out.println("AICI: 2");
							UtilAVP.addResultCode(response, DiameterConstants.ResultCode.DIAMETER_UNABLE_TO_COMPLY.getCode());
						}
					}
					break;
			}
		}	
		catch(CxExperimentalResultException e){
			e.printStackTrace();
		}
		catch(CxFinalResultException e){
			e.printStackTrace();
		}
		
		finally{
			HibernateUtil.commitTransaction();
			session.close();
		}
		
		return response;
	}
	
	private static AuthenticationVector synchronize(Session session, byte [] authorization, IMPI impi){
    	logger.info("Handling Synchronization between Mobile Station and Home Environment!");

        byte [] secretKey = HexCodec.getBytes(impi.getK(), CxConstants.Auth_Parm_Secret_Key_Size);

        try {
	            // get op and generate opC	
	            byte [] op = impi.getOp();
				byte [] opC = Milenage.generateOpC(secretKey, op);
				byte [] amf = impi.getAmf();
		        int authScheme = impi.getAuth_scheme();

		        // sqnHE - represent the SQN from the HSS
		        // sqnMS - represent the SQN from the client side
		        byte[] sqnHe = impi.getSqn().getBytes();
		        sqnHe = DigestAKA.getNextSQN(sqnHe, HSSProperties.IND_LEN);

		        byte [] nonce = new byte[32];
		        byte [] auts = new byte[14];
		        int k = 0;
		        for (int i = 0; i < 32; i++, k++){
	        		nonce[k] = authorization[i]; 
		        }
		        k = 0;
		        for (int i = 32; i < 46; i++, k++){
	        		auts[k] = authorization[i]; 
		        }
		        
		        byte [] rand = new byte[16];
		        k = 0;
		        for (int i = 0; i < 16; i++, k++){
	        		rand[k] = nonce[i]; 
		        }

		        byte[] ak = null;
		        if (HSSProperties.USE_AK){
	                ak = Milenage.f5star(secretKey, rand, opC);
	            }
		        byte [] sqnMs = new byte[6];
		        k = 0;
		        if (HSSProperties.USE_AK){
			        for (int i = 0; i < 6; i++, k++){
		        		sqnMs[k] = (byte) (auts[i] ^ ak[i]); 
			        }
			        logger.warn("USE_AK is enabled and will be used in Milenage algorithm!");
		        }
		        else{
			        for (int i = 0; i < 6; i++, k++){
		        		sqnMs[k] = auts[i]; 
			        }
			        logger.warn("USE_AK is NOT enabled and will NOT be used in Milenage algorithm!");
		        }
		        //System.out.println("SQN_MS is: " + codec.encode(sqnMs));
		        //System.out.println("SQN_HE is: " + codec.encode(sqnHe));
		        
		        if (DigestAKA.SQNinRange(sqnMs, sqnHe, HSSProperties.IND_LEN, HSSProperties.delta, HSSProperties.L)){
		        	logger.info("The new generated SQN value shall be accepted on the client, abort synchronization!");
		        	k = 0;
		        	byte[] copySqnHe = new byte[6];
			        for (int i = 0; i < 6; i++, k++){
		        		copySqnHe[k] = sqnHe[i]; 
			        }
		        	
		            AuthenticationVector aVector = DigestAKA.getAuthenticationVector(authScheme, 
		            		secretKey, opC, amf, copySqnHe);
		            IMPI_DAO.update(session, impi.getId(), new String(sqnHe));
		            
		            return aVector;
		        }
		        	
	        	byte xmac_s[] = Milenage.f1star(secretKey, rand, opC, sqnMs, amf);
	        	byte mac_s[] = new byte[8];
	        	k = 0;
	        	for (int i = 6; i < 14; i++, k++){
	        		mac_s[k] = auts[i];
	        	}
		        	
	        	for (int i = 0; i < 8; i ++)
	        		if (xmac_s[i] != mac_s[i]){
	        			logger.error("XMAC and MAC are different! User not authorized in performing synchronization!");
	        			return null;
	               }

	            sqnHe = sqnMs;
	            sqnHe = DigestAKA.getNextSQN(sqnHe, HSSProperties.IND_LEN);
	     		logger.info("Synchronization of SQN_HE with SQN_MS was completed successfully!");
		        
		        byte[] copySqnHe = new byte[6];
		        k = 0;
		        for (int i = 0; i < 6; i++, k++){
	        		copySqnHe[k] = sqnHe[i]; 
		        }
	            AuthenticationVector aVector = DigestAKA.getAuthenticationVector(authScheme, 
	            		secretKey, opC, amf, copySqnHe);
	            
	            // update Cxdata
	            IMPI_DAO.update(session, impi.getId(), new String(sqnHe));
	            return aVector;
	            
			} 
	        catch (InvalidKeyException e) {
				e.printStackTrace();
				return null;
			}
	}

	public static List generateAuthVectors(Session session, IMPI impi, int auth_scheme, int av_cnt){
        List avList = null;

        byte [] secretKey = HexCodec.getBytes(impi.getK(), CxConstants.Auth_Parm_Secret_Key_Size);
        byte [] amf = impi.getAmf();

        // op and generate opC	
        byte [] op = impi.getOp();
        
        byte[] opC;
		try {
			opC = Milenage.generateOpC(secretKey, op);
		} catch (InvalidKeyException e1) {
			e1.printStackTrace();
			return avList;
		}
        byte [] sqn = impi.getSqn().getBytes();

        // MD5 Authentication Scheme
        if (auth_scheme == CxConstants.AuthScheme.Auth_Scheme_MD5.getCode()){
        	// Authentication Scheme is Digest-MD5
        	logger.debug("Auth-Scheme is Digest-MD5");
            SecureRandom randomAccess;
			try {
				randomAccess = SecureRandom.getInstance("SHA1PRNG");
			} 
			catch (NoSuchAlgorithmException e) {
				e.printStackTrace();
				return avList;
			}
            avList = new LinkedList();
            for (long ix = 0; ix < av_cnt; ix++){
                byte[] randBytes = new byte[16];
            	randomAccess.setSeed(System.currentTimeMillis());
                randomAccess.nextBytes(randBytes);
                
            	AuthenticationVector av = new AuthenticationVector(auth_scheme, randBytes, secretKey);
            	avList.add(av);
            }
        	return avList;
        }
        else if (auth_scheme == CxConstants.AuthScheme.Auth_Scheme_AKAv1.getCode() || 
        		auth_scheme == CxConstants.AuthScheme.Auth_Scheme_AKAv2.getCode() ){
        	// Authentication Scheme is AKAv1 or AKAv2
        	logger.debug("Auth-Scheme is Digest-AKA");
        	avList = new LinkedList();
        	
            for (long ix = 0; ix < av_cnt; ix++){
            	sqn = DigestAKA.getNextSQN(sqn, HSSProperties.IND_LEN);
    	        byte[] copySqnHe = new byte[6];
    	        int k = 0;
    	        for (int i = 0; i < 6; i++, k++){
            		copySqnHe[k] = sqn[i]; 
    	        }
    	        
            	AuthenticationVector av;
            	System.out.println("auth:" + auth_scheme);
            	System.out.println("secret:" + secretKey.length);
            	System.out.println("opC:" + opC.length);
            	System.out.println("amf:" + amf.length);
            	System.out.println("SQN:" + copySqnHe.length);
            	
				av = DigestAKA.getAuthenticationVector(auth_scheme, secretKey, opC, amf, copySqnHe);
            	System.out.println("authenticate:" + av.getSipAuthenticate().length);
            	System.out.println("auhtorization:" + av.getSipAuthorization().length);
            	System.out.println("ck:" + av.getConfidentialityityKey().length);
            	System.out.println("ik:" + av.getIntegrityKey().length);
				
				if (av != null){
					avList.add(av);
				}
				else {
					break;
				}
            }
            
            if (avList != null && avList.size() != 0){
            	IMPI_DAO.update(session, impi.getId(), new String(sqn));
            }
        }
        
        return avList;
	}
}
