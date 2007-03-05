/*
 * $Id$
 *
 * Copyright (C) 2004-2006 FhG Fokus
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

package de.fhg.fokus.diameter.DiameterPeer.data;

import java.util.Vector;

import org.apache.log4j.Logger;


/**
 * This class defines the basic Diameter Message structure
 * 
 * @author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */
public class DiameterMessage {
	
	/** The logger */
	private static final Logger LOGGER = Logger.getLogger(DiameterMessage.class);

	/** The version of the message */
	int version=1;
	
	/** Command Code */
	public int commandCode;
	
	/** If the message is a Request*/
	public boolean flagRequest=true;
	
	/** If the message is proxiable */
	public boolean flagProxiable=true;
	
	/** If the message contains a protocol error */
	public boolean flagError=false;
	
	/** Potentially retransmission */
	public boolean flagRetransmission=false;
	
	/** Application ID */
	public int applicationID=0;
	
	/** Hop-by-Hop identifier */
	public long hopByHopID=0;

	/** End-to-End identifier */
	public long endToEndID=0;
	
	/** Contained AVPs */
	public Vector<AVP> avps;
	
	
	/** statistical time - close to media */
	public long networkTime=0;

	
	/**
	 * Dumb constructor.
	 */
	public DiameterMessage()
	{
		avps = new Vector<AVP>();
	}
	
	/**
	 * Constructor that initializes everything.
	 * @param Command_Code
	 * @param Request
	 * @param Proxiable
	 * @param Application_id
	 * @param HopByHop_id
	 * @param EndToEnd_id
	 */
	public DiameterMessage(int Command_Code,boolean Request,boolean Proxiable,
			int Application_id,long HopByHop_id, long EndToEnd_id)
	{
		this.commandCode = Command_Code;
		this.flagRequest = Request;
		this.flagProxiable = Proxiable;
		this.applicationID = Application_id;
		this.hopByHopID = HopByHop_id;
		this.endToEndID = EndToEnd_id;
		avps = new Vector<AVP>();
	}
	
	/**
	 * Constructor that initializes just the needed attributes.
	 * @param Command_Code
	 * @param Request
	 * @param Application_id
	 */
	public DiameterMessage(int Command_Code,boolean Request,int Application_id)
	{
		this.commandCode = Command_Code;
		this.flagRequest = Request;
		this.flagProxiable = false;
		this.applicationID = Application_id;
		avps = new Vector<AVP>();
	}
	
	/**
	 * Adds one AVP to the message.
	 * @param child
	 */
	public void addAVP(AVP child)
	{
		avps.add(child);
	}
	
	/**
	 * Returns the count of the AVPs.
	 * 
	 * @return The number of all available AVPs.
	 */
	public int getAVPCount()
	{
		return avps.size();
	}
	
	/**
	 * Returns the AVP. 
	 * @param index
	 * @return the found AVP or null if out of bounds
	 */
	public AVP getAVP(int index)
	{
		if (index<avps.size())
			return (AVP)avps.get(index);
		else return null;
	}
	
	/**
	 * Deletes the given AVP from the list of AVPs.
	 * @param avp
	 */
	public void deleteAVP(AVP avp)
	{
		avps.remove(avp);
	}
	
	/**
	 * Searches for an AVP inside the Vector of AVPs.
	 * @param Code
	 * @param Mandatory
	 * @param Vendor_id
	 * @return the found AVP, null if not found
	 */
	public AVP findAVP(int Code,boolean Mandatory,int Vendor_id)
	{
		AVP avp;
		for(int i=0;i<avps.size();i++){
			avp = (AVP) avps.get(i);
			if (avp.code == Code &&
				avp.flag_mandatory == Mandatory &&
				avp.vendor_id == Vendor_id) 
					return avp;
		}
		return null;
	}
	
	/**
	 * Searches for all AVPs with the same code inside the Vector of AVPs.
	 * @param Code
	 * @return the found AVP, null if not found
	 */
	public AVP[] findAVPs(int Code)
	{
		AVP[] avpset;
		int j = 0, count = 0;
		AVP avp;
		
		for(int i=0;i<avps.size();i++){
			avp = (AVP) avps.get(i);
			if (avp.code == Code)
				count++;
		}
		
		if (count == 0) return null;
		avpset = new AVP[count];
		for(int i=0;i<avps.size();i++){
			avp = (AVP) avps.get(i);
			if (avp.code == Code) {
				avpset[j++] = avp;
				if (j == count) break;
			}
		}
		
		return avpset;
	}
	
	/**
	 * Searches for an AVP inside the Vector of AVPs.
	 * @param Code
	 * @return the found AVP, null if not found
	 */
	public AVP findAVP(int Code) {
		AVP avp;
		for(int i=0;i<avps.size();i++){
			avp = (AVP) avps.get(i);
			if (avp.code == Code) {
				
				return avp;
			}
		}
		return null;	
		
	}
	
	/**
	 * Searches for the Session-Id AVP inside a message
	 * @return the found Session-Id AVP, null if not found
	 */
	public AVP getSessionId() {
		return findAVP(AVP.Session_Id);
	}
	
	/**
	 * Human readable version of the AVP for logging
	 */
	public String toString()
	{
		StringBuffer x = new StringBuffer();
		x.append("Diameter: Code=");x.append(commandCode);
		if (flagRequest) x.append(" R");
		if (flagProxiable) x.append(" P");
		if (flagError) x.append(" E");
		if (flagRetransmission) x.append(" T");
		
		x.append(" AppID=");x.append(applicationID);
		x.append(" HbHID=");x.append(hopByHopID);
		x.append(" E2EID=");x.append(endToEndID);
		
		x.append("\n");
		for(int i=0;i<avps.size();i++){
			x.append("\t");
			x.append(avps.get(i).toString());
			x.append("\n");
		}		
		return x.toString();
	}
	
	/** Command code of Capabilities-Exchange Request/Answer */
	public static final int Code_CE=257;
	
	/** Command code of Device-Watchdog Request/Answer */
	public static final int Code_DW=280;
	
	/** Command code of Disconnect-Peer-Request/Answer */
	public static final int Code_DP=282;
	
	/** 
	 * ResultCode returned is 2001, when the request was successfully completed.
	 */
	public static final int DIAMETER_SUCCESS = 2001;
	
	/** 
	 * ResultCode returned is 5010, when CER message is received, and there are no common
	 * applications supported between the peers.
	 */
	public static final int DIAMETER_NO_COMMON_APPLICATION = 5010;
	
	/**
	 * ResultCode returned is 5012, when a request was received, whose version
	 * number is unsupported.
	 */
	public static final int DIAMETER_UNABLE_TO_COMPLY = 5012;
	
}
