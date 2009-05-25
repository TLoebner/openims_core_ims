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

	/** Diameter protocol header: 
		byte 0:   protocol version
		byte 1-3: length of message, including the header
	*/
	private static int HEADER_LENGTH = 4;

	/**
	   The size of the buffer that run() allocates for flushing the
	   InputStream of a message that is too big to fit in memory.
	 */
	private static final int TRASH_BUFFER_LENGTH = 524388; // 512KB

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
		byte[] buffer;
		long bytesRead = 0L;

		// Every message begins with a header.  We know how big that
		// is.  We don't need to allocate the header every time.
		byte[] header = new byte[HEADER_LENGTH];

		// When a message is legitimate but too big to fit into the
		// JVM, we'll attempt to read all of it in chunks to remove if
		// from the InputStream.  We put those chunks in trashBuffer.
		byte[] trashBuffer = new byte[TRASH_BUFFER_LENGTH];
		DiameterMessage msg;
		
		int cnt = 0,len = 0,x = 0;
		
		try {
			socket.setTcpNoDelay(true);
			in = socket.getInputStream();
		} catch (IOException e) {
			System.err.println("Communicator: Error getting InputStream from socket");
			e.printStackTrace();
			return;
		}

		try {
			// TALMAGE: New plan: After reading the four-byte header,
			// allocate a buffer for the whole message.  Read into
			// the buffer in chunks, blocking if necessary.
			//
			// What I really want to do is to use a StringBuilder to
			// accumulate bytes.  Alas, there is no byte holding
			// version of StringBuilder.
			//
			// I also considered using Java NIO.
			while(running){
				/* first we read the version */ 
				cnt=0;
				while(cnt<1){
					x=in.read(header,cnt,1);
					// It's not that the read failed.  There isn't
					// any more data in the input stream.
					if (x<0) throw(new Exception("Read failed"));
					cnt+=x;
				}
				if (header[0]!=1){
					System.err.println("Communicator: Expecting diameter version 1. Received version "+header[0]);
					continue;
				}
				/* then we read the length of the message */
				while(cnt<4){
					x = in.read(header,cnt,4-cnt);
					if (x<0) throw(new Exception("Read failed.  # bytes before failure: "));
					cnt+=x;
					bytesRead += x;
				}
				len = ((int)header[1]&0xFF)<<16 |
					((int)header[2]&0xFF)<< 8 |
					((int)header[3]&0xFF);

				LOGGER.debug("Message length " + len);
				try {
					// The biggest message permitted by the protocol
					// is 2^24 (= 16777216) bytes.  This could throw
					// OutOfMemoryError.
					buffer = new byte[len];

					// This could throw three different RuntimeExceptions.
					// The conditions that cause them will never occur
					// here, so there is no need to catch them.
					System.arraycopy(header, 0, buffer, 0, header.length);

					/* and then we read all the rest of the message */
					while(cnt<len){
						x = in.read(buffer,cnt,len-cnt);
						// It's not that the read failed.  There isn't
						// any more data in the input stream.
						if (x<0) throw(new Exception("Premature end of input stream"));
						cnt+=x;
						bytesRead += x;
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
				} catch (OutOfMemoryError e1) {
					// Attempt to read all of the bytes in the message and
					// throw them away.  Is it an error to continue?
					//
					// TALMAGE: I want to wrap the arg of
					// LOGGER.warn() in one of my Strings objects to
					// prevent concatenation unless the logging level
					// is Level.WARN or higher.  However,
					// OutOfMemoryError shouldn't occur very often and
					// I don't have permission to distribute the
					// Strings class.
					LOGGER.warn("Out of memory allocating buffer for message of length " + len);
					LOGGER.debug(e1);
					try {
						while (cnt < len) {
							x = in.read(trashBuffer, 0, trashBuffer.length);
							cnt += x;
							bytesRead += x;
						}
					} catch (IOException ex) {
						LOGGER.warn("Read " + bytesRead + 
									" before Exception.");
						disconnect(ex);
						running=false;
					}
				}
			}
		} catch (Exception e1) {
			LOGGER.warn("Read " + bytesRead + " before Exception.");
			disconnect(e1);
		}

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
	   On some kind of Exception, forcefully disconnect from the Peer
	   that caused it.
	 */
    private void disconnect(Exception ex) {
		if (running){
			if (this.peer!=null){
				if (this.peer.I_comm==this) 
					StateMachine.process(this.peer,StateMachine.I_Peer_Disc);
				if (this.peer.R_comm==this) 
					StateMachine.process(this.peer,StateMachine.R_Peer_Disc);
			}
			LOGGER.debug("Communicator: Error reading from InputStream. Closing socket.");
			if (null != ex) {
				LOGGER.debug(ex);
			}
		}/* else it was a shutdown request, it's normal */
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

