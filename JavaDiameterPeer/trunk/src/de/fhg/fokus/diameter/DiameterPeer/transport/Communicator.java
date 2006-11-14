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

package de.fhg.fokus.diameter.DiameterPeer.transport;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

import org.apache.log4j.Logger;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;
import de.fhg.fokus.diameter.DiameterPeer.data.AVP;
import de.fhg.fokus.diameter.DiameterPeer.data.Codec;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessageDecodeException;
import de.fhg.fokus.diameter.DiameterPeer.peer.Peer;
import de.fhg.fokus.diameter.DiameterPeer.peer.StateMachine;

/**
 * This class defines the Diameter Connection Receiver.
 * <p>
 * A communicator maintains a connection with a peer. After its creation, it 
 * will be managed by the PeerManager.
 * 
 * @author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */
public class Communicator extends Thread {
	
	/** The logger */
	private static final Logger LOGGER = Logger.getLogger(Communicator.class);

	/** DiameterPeer API reference */
	public DiameterPeer diameterPeer;
		
	/** peer it is comunicating for */
	public Peer peer=null;
	
	/** indicator if still active */
	public boolean running; 
	
	/** Direction of socket opening */
	public int direction;
	public static final int Initiator 	= 0;
	public static final int Receiver 	= 1;
	
	/** socket connected to */
	public Socket socket;

	private static int MAX_MESSAGE_LENGTH=1048576;
	
	
	/**
	 * Constructor giving the opened socket.
	 * 
	 * @param socket    Socket should be opened.
	 * @param dp		DiameterPeer, which contains several Peers  
	 * @param direction 1 for initiator, 0 for receiver
	 */
	public Communicator(Socket socket, DiameterPeer dp,int direction) {
		this.socket = socket;
		this.direction = direction;
		running = true;
		this.diameterPeer = dp;
		this.start();		
	}
	 
	/**
	 * Constructor giving the opened socket.
	 * 
	 * @param socket    Socket should be opened.
	 * @param p         Peer, for which the socket is opened.
	 * @param direction 1 for initiator, 0 for receiver.
	 */
	public Communicator(Socket socket, Peer p,int direction) {
		this.socket = socket;
		this.direction = direction;
		running = true;
		this.diameterPeer = p.diameterPeer;
		this.peer = p;
		this.start();		
	}
	
	
	
	/**
	 * Send a Diameter message.
	 * 
	 * @param msg The Diameter message which is sent.
	 * @return true if successful, false otherwise.
	 */
	public boolean sendMessage(DiameterMessage msg)
	{
		if (this.peer!=null){
			// to optimize the call and avoid critical zone 
			//StateMachine.process(peer,StateMachine.Send_Message,msg,this);
			StateMachine.Snd_Message(peer,msg);
		}
			
		return sendDirect(msg);
	}
	
	
	
	/**
	 * Send a Diameter message.
	 * 
	 * @param msg Diameter request which is sent.
	 * @return true if successful, false otherwise.
	 */
	public synchronized boolean sendDirect(DiameterMessage msg)
	{
		if (!socket.isConnected()) {
			System.err.println("Communicator: Tried to send message to unconnected socket.");
			return false;
		}
		byte[] buffer;
		int sent=0;
		//LOGGER.debug("Communicator:sendDirect():"+msg.toString());
		try {
			OutputStream out=socket.getOutputStream();
			buffer = Codec.encodeDiameterMessage(msg);
			out.write(buffer,sent,buffer.length-sent);
			msg.networkTime = System.currentTimeMillis();
		} catch (Exception e){
			LOGGER.debug("Communicator: Error on message send\n");
			e.printStackTrace();
			return false;
		}
		return true;
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Runnable#run()
	 */
	public void run() {
		InputStream in;
		byte[] buffer = new byte[MAX_MESSAGE_LENGTH];
		DiameterMessage msg;
		
		int cnt,len,x;
		
		try {
			socket.setTcpNoDelay(true);
			in = socket.getInputStream();
		} catch (IOException e) {
			System.err.println("Communicator: Error getting InputStream from socket");
			e.printStackTrace();
			return;
		}

		try {
				while(running){
					/* first we read the version */ 
					cnt=0;
					while(cnt<1){
						x=in.read(buffer,cnt,1);
						if (x<0) throw(new Exception("Read failed"));
						cnt+=x;
					}
					if (buffer[0]!=1){
						System.err.println("Communicator: Expecting diameter version 1. Received version "+buffer[0]);
						continue;
					}
					/* then we read the length of the message */
					while(cnt<4){
						x = in.read(buffer,cnt,4-cnt);
						if (x<0) throw(new Exception("Read failed"));
						cnt+=x;
					}
					len = ((int)buffer[1]&0xFF)<<16 |
						  ((int)buffer[2]&0xFF)<< 8 |
						  ((int)buffer[3]&0xFF);
					if (len>MAX_MESSAGE_LENGTH){
						System.err.println("Communicator: Message too long ("+len+">"+MAX_MESSAGE_LENGTH+".");
					}
					/* and then we read all the rest of the message */
					while(cnt<len){
						x = in.read(buffer,cnt,len-cnt);
						if (x<0) throw(new Exception("Read failed"));
						cnt+=x;
					}
					//LOGGER.debug("received "+cnt+" bytes");
					/* now we can decode the message */
					try {
						msg = Codec.decodeDiameterMessage(buffer,0);
					} catch (DiameterMessageDecodeException e3) {
						System.err.println("Communicator: Error decoding diameter message");
						e3.printStackTrace();
						continue;
					}
					//LOGGER.debug("Communicator:run(): "+msg.toString());
					msg.networkTime = System.currentTimeMillis();
					if (this.peer!=null)
						this.peer.refreshTimer();
					processMessage(msg);
					msg = null;
															
				}
		} catch (Exception e1) {
			if (running){
				if (this.peer!=null){
					if (this.peer.I_comm==this) StateMachine.process(this.peer,StateMachine.I_Peer_Disc);
					if (this.peer.R_comm==this) StateMachine.process(this.peer,StateMachine.R_Peer_Disc);
				}
				LOGGER.debug("Communicator: Error reading from InputStream. Closing socket.");
				e1.printStackTrace();
			}/* else it was a shutdown request, it's normal */
		}

		running=false;

		try {
			socket.close();
		} catch (IOException e2) {
			System.err.println("Communicator: Error closing socket.");
			e2.printStackTrace();
		}
	}
	
	private void processMessage(DiameterMessage msg)
	{
		int event;
		
//		LOGGER.debug("Communicator: Received message \n"+
//				msg.toString()+"\n---");
		
		/* pre-processing for special states */
		if (this.peer!=null){
			switch (this.peer.state){
				case StateMachine.Wait_I_CEA:
					if (msg.commandCode!=DiameterMessage.Code_CE){
						StateMachine.process(this.peer,StateMachine.I_Rcv_Non_CEA,msg,this);
						return;
					}
					break;		
				case StateMachine.R_Open:
					switch (msg.commandCode){
						case DiameterMessage.Code_CE:
							if (msg.flagRequest)
								StateMachine.process(this.peer,StateMachine.R_Rcv_CER,msg,this);
							else
								StateMachine.process(this.peer,StateMachine.R_Rcv_CEA,msg,this);
							return;
						case DiameterMessage.Code_DW:
							if (msg.flagRequest)
								StateMachine.process(this.peer,StateMachine.R_Rcv_DWR,msg,this);
							else
								StateMachine.process(this.peer,StateMachine.R_Rcv_DWA,msg,this);
							return;
						case DiameterMessage.Code_DP:
							if (msg.flagRequest)
								StateMachine.process(this.peer,StateMachine.R_Rcv_DPR,msg,this);
							else
								StateMachine.process(this.peer,StateMachine.R_Rcv_DPA,msg,this);
							return;
						default:
							/* faster processing -> no state machine for regular messages */
							//StateMachine.process(this.peer,StateMachine.R_Rcv_Message,msg,this);
							StateMachine.Rcv_Process(this.peer,msg);
							return;
					}
				case StateMachine.I_Open:
					switch (msg.commandCode){
					case DiameterMessage.Code_CE:
						if (msg.flagRequest)
							StateMachine.process(this.peer,StateMachine.I_Rcv_CER,msg,this);
						else
							StateMachine.process(this.peer,StateMachine.I_Rcv_CEA,msg,this);
						return;
					case DiameterMessage.Code_DW:
						if (msg.flagRequest)
							StateMachine.process(this.peer,StateMachine.I_Rcv_DWR,msg,this);
						else
							StateMachine.process(this.peer,StateMachine.I_Rcv_DWA,msg,this);
						return;
					case DiameterMessage.Code_DP:
						if (msg.flagRequest)
							StateMachine.process(this.peer,StateMachine.I_Rcv_DPR,msg,this);
						else
							StateMachine.process(this.peer,StateMachine.I_Rcv_DPA,msg,this);
						return;
					default:
						/* faster processing -> no state machine for regular messages */
						//StateMachine.process(this.peer,StateMachine.I_Rcv_Message,msg,this);
						StateMachine.Rcv_Process(this.peer,msg);
						return;
				}
			}
		}
		
		/* main processing */
		switch (msg.commandCode){
			case DiameterMessage.Code_CE:
				if (msg.flagRequest) {
					/* CER  - Special processing to find the peer */
					/* find peer */
					AVP fqdn,realm;
					Peer p;
					fqdn = msg.findAVP(AVP.Origin_Host,true,0);
					if (fqdn==null) {
						System.err.println("Communicator: CER Received without Origin-Host");
						return;
					}
					realm = msg.findAVP(AVP.Origin_Realm,true,0);
					if (realm==null) {
						System.err.println("Communicator: CER Received without Origin-Realm");
						return;
					}
					p = diameterPeer.peerManager.getPeerByFQDN(new String(fqdn.data));
					if (p==null) {
						p = diameterPeer.peerManager.addDynamicPeer(new String(fqdn.data),
									new String(realm.data));
					}
					if (p==null){
						//Give up
						System.err.println("Communicator: Not Allowed to create new Peer");
						return;
					}
					this.peer = p;
					/* call state machine */	
					StateMachine.process(p,StateMachine.R_Conn_CER,msg,this);
	
				}else{
					/* CEA */
					if (this.peer==null) {
						System.err.println("Receiver: received CEA for an unknown peer");
						System.err.println(msg.toString());
					}else{
						if (this.direction == Initiator) event = StateMachine.I_Rcv_CEA;
						else event = StateMachine.R_Rcv_CEA;
						StateMachine.process(this.peer,event,msg,this);
					}
				}
				break;
				
			case DiameterMessage.Code_DW:
				if (msg.flagRequest){
					if (this.direction == Initiator) event = StateMachine.I_Rcv_DWR;
					else event = StateMachine.R_Rcv_DWR;
					StateMachine.process(peer,event,msg,this);
				}
				else { 
					if (this.direction == Initiator) event = StateMachine.I_Rcv_DWA;
					else event = StateMachine.R_Rcv_DWA;
					StateMachine.process(peer,event,msg,this);
				}
				break;
			
			case DiameterMessage.Code_DP:
				if (msg.flagRequest){
					if (this.direction == Initiator) event = StateMachine.I_Rcv_DPR;
					else event = StateMachine.R_Rcv_DPR;
					StateMachine.process(peer,event,msg,this);
				}
				else { 
					if (this.direction == Initiator) event = StateMachine.I_Rcv_DPA;
					else event = StateMachine.R_Rcv_DPA;
					StateMachine.process(peer,event,msg,this);
				}
				break;
			
			default:
				/*
				if (this.direction == Initiator) event = StateMachine.I_Rcv_Message;
				else event = StateMachine.R_Rcv_Message;
				StateMachine.process(peer,event,msg,this);
				*/
				/* faster processing -> no state machine for regular messages */
				StateMachine.Rcv_Process(this.peer,msg);
		}
	}
	
	/**
	 * Shutdown the socket.
	 */
	public void shutdown()
	{
		this.running = false;
		try {
			this.socket.close();
		} catch (IOException e) {
			System.err.println("Communicator: Shutdown - error closing socket");
			e.printStackTrace();
		}

	}
}
/**

\package de.fhg.fokus.diameter.DiameterPeer.transport
Contains acceptors and communicators for creating and maintaining connections 
between Diameter peers.

<p>
A DiameterPeer uses an Acceptor to listen to requests from other DiameterPeers. 
If a request is coming in from an undetected DiameterPeer, a Communicator is 
created to maintain this connection. This Communicator will also be associated 
with a Peer in the PeerManager. 
*/

