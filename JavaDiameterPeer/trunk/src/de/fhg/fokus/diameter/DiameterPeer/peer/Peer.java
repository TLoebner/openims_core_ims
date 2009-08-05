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

package de.fhg.fokus.diameter.DiameterPeer.peer;

import java.util.Vector;

import org.apache.log4j.Logger;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.diameter.DiameterPeer.transport.Communicator;
/**
 * \package de.fhg.fokus.diameter.DiameterPeer.peer
 * Provides classes for creating peer, peer state machine and peer manager which 
 * manages a list of peers.   
 */
/**
 * This class defines a Diameter Peer
 * 
 * @author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */
public class Peer {
	
	/** The logger */
	private static final Logger LOGGER = Logger.getLogger(Peer.class);
	
	/** DiameterPeer API reference */
	public DiameterPeer diameterPeer;
	
	/** Fully Qualified Domain Name of the peer */
	public String FQDN;
	
	/** Port number to connect to */
	public int port;

	/** Realm */
	public String Realm;
	
	/** State in the state machine */
	public int state=StateMachine.Closed;
		
	/** Communicator connected to the peer */
	public Communicator I_comm;
		
	/** Communicator for parallel first Receiver Connection */
	public Communicator R_comm; 
	
	/** Last time we received a message from this peer */
	public long lastReceiveTime;
	
	/** If we send a DWR and we are waiting for DWA */
	public boolean waitingDWA=false;
	
	/** If the peer is dynamic and should be removed on disconnect */
	public boolean isDynamicPeer=false;
	
	/** Supported auth-application ids */
	public Vector<Application> AuthApp;
	
	/** Supported acct-application ids */
	public Vector<Application> AcctApp;

	
	
	/**
	 * Creates a peer given the parameteres.
	 * 
	 * @param fqdn
	 * @param realm
	 * @param port
	 */
	public Peer(String fqdn, String realm, int port) {
		FQDN = fqdn;
		Realm = realm;
		this.port = port;
		this.AuthApp = new Vector<Application>();
		this.AcctApp = new Vector<Application>();
	}
	
	
	/**
	 * Send a Diameter messager. A Peer found out in the Peer list according to
	 * the destination host or realm in the DiameterMessage will handle this 
	 * message. 
	 *  
	 * @param msg DiameterMessage being sent.
	 * @return true if send successfully, otherwise false.
	 */
	public boolean sendMessage(DiameterMessage msg)
	{
		switch (this.state){
			case StateMachine.I_Open: 
				//commented because it is processed by peer... anyway, useless...
				//StateMachine.process(this,StateMachine.Send_Message,msg,this.I_comm);
				return I_comm.sendMessage(msg);
			case StateMachine.R_Open:
				//commented because it is processed by peer... anyway, useless...
				//StateMachine.process(this,StateMachine.Send_Message,msg,this.R_comm);
				return R_comm.sendMessage(msg);
			default:
				System.err.println("Peer: Can't send message when not connected");
				return false;
		}
	}
	
	/**
	 * Renew this timer, so that a DWR/DWA exchange will be postponed.
	 *
	 */
	public void refreshTimer()
	{
		this.lastReceiveTime = System.currentTimeMillis();
	}
	
	
	public boolean handlesApplication(Application app)
	{
		int i;
		for(i=0;i<AuthApp.size();i++)
			if (AuthApp.get(i).equals(app)) return true;
		for(i=0;i<AcctApp.size();i++)
			if (AcctApp.get(i).equals(app)) return true;
		return false;
	}
}
