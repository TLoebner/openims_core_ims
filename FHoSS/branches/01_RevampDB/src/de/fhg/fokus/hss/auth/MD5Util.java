package de.fhg.fokus.hss.auth;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class MD5Util {
	public static final byte [] colon = ":".getBytes();
	public static byte [] av_HA1(byte [] username, byte [] realm, byte [] password)
	{
		byte [] ha1;
		MessageDigest md = null;
		//HASH HA1;
		try {
			md = MessageDigest.getInstance("MD5");
		}
		catch (NoSuchAlgorithmException nsae)
		{
			return null;
		}
		md.update(username);
		md.update(colon);
		md.update(realm);
		md.update(colon);
		md.update(password);
		
		ha1 = md.digest();
		
		return ha1;
	}
	/*response-digest */
	public static byte []  calcResponse(byte[] _ha1,      /* H(A1) */
	                    		   byte [] _nonce,       /* nonce from server */
	                    		   byte [] _method,      /* method from the request */
	                    		   byte []  _uri         /* requested URL */)
	                    		    
	{
    	byte [] HA2;
    	byte [] HA2Hex;
    	byte [] RespHash;
    	byte [] _response;
    	
    	MessageDigest md = null;
		//HASH HA1;
		try {
			md = MessageDigest.getInstance("MD5");
		}
		catch (NoSuchAlgorithmException nsae)
		{
			return null;
		}
    	
         /* calculate H(A2) */
		md.update(_method);
		md.update(colon);
		md.update(_uri);
    	
		HA2 = md.digest();
    	
		HA2Hex = HexCodec.encode(HA2).getBytes();
    	
    	 /* calculate response */
    	md.update(_ha1);
    	md.update(colon);
    	md.update(_nonce);
    	md.update(colon);
    	md.update(HA2Hex);
    	
    	RespHash = md.digest();
    	_response = HexCodec.encode(RespHash).getBytes();
    	return _response;
    } 
	
}
