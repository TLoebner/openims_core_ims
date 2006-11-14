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
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.net.SocketTimeoutException;

import org.apache.log4j.Logger;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;


/**
 * This class defines the Diameter Connection Acceptor.
 * 
 * <p>
 * A DiameterPeer may use several acceptors to get connections with other 
 * DiameterPeers. If a connection is created, it will be maintained by a 
 * communicator.
 * 
 * @author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */
public class Acceptor extends Thread {

	/** The logger */
	private static final Logger LOGGER = Logger.getLogger(Acceptor.class);
	
	/** DiameterPeer API reference */
	public DiameterPeer diameterPeer;

	/** The port it listens to */
	int port;
	
	/** The socket we are accepting connections on */
	ServerSocket acceptSocket;
	
	/** The server address we are accepting connections on */
	InetAddress acceptAddr;

	/** If it is accepting connections */
	private boolean accepting=false;
	
	/** Used to create the listening socket */ 
	private static int backlog=50;

	/** SO_TIMEOUT for the socket */ 
	private static int so_timeout=0;
	
	/**
	 * Creates a new acceptor. 
	 * @param port Port number, at which the DiameterPeer is ready to accept
	 * 			   a connection.	
	 */
	public Acceptor(int port) {
		this.port = port;
		this.acceptAddr = null;
		openSocket();
	}

	/**
	 * Create a new acceptor.
	 * 
	 * @param port     Port number, at which the DiameterPeer is ready to accept
	 * 			       a connection.	
	 * @param bindAddr IP address, at which the DiameterPeer accepts connnection. 
	 * @param dp       DiameterPeer, for which the acceptor is created.
	 */
	public Acceptor(int port,InetAddress bindAddr,DiameterPeer dp) {
		this.port = port;
		this.acceptAddr = bindAddr;
		this.diameterPeer = dp;
		openSocket();
	}
	
	private void openSocket()
	{
		try {
			acceptSocket = new ServerSocket(port,backlog,acceptAddr);
		} catch (IOException e) {
			LOGGER.error("Acceptor: Error opening socket on port "+port);
			e.printStackTrace();
		}
		try {
			acceptSocket.setSoTimeout(so_timeout);
		} catch (SocketException e1) {
			LOGGER.error("Acceptor: Error setting SO_TIMEOUT socket of "+so_timeout);
			e1.printStackTrace();
		}
	}
	
	private void closeSocket()
	{
		try {
			acceptSocket.close();
		} catch (IOException e) {
			LOGGER.error("Acceptor: Error closing socket on port "+port);
			e.printStackTrace();
		}
	}	
	
	
	/**
	 * Start a acceptor. A DiameterPeer is ready to accept a connection.
	 *
	 */
	public void startAccepting()
	{
		accepting = true;
		this.start();
	}

	/** 
	 * Stop a acceptor. A DiameterPeer is unable to accept a connection.
	 *
	 */
	public void stopAccepting()
	{
		accepting = false;
		if (so_timeout==0)
			closeSocket();
	}
	
	/** 
	 * Accept incoming request, create a communicator which will
	 * be associated with a propor Peer in the PeerManager.
	 * 	 
	 * @see java.lang.Runnable#run()
	 */
	public void run() {
		Socket clientSocket;
		while(accepting){
			try {
				clientSocket = acceptSocket.accept();
			} catch (SocketTimeoutException e) {
				LOGGER.error("Acceptor: SO_TIMEOUT -> No Connection");
				continue;
			} catch (IOException e) {
				if (accepting){
					LOGGER.error("Acceptor: I/O Error on accept");
					e.printStackTrace();
				}
				break;
			} 
			LOGGER.debug("Acceptor: Connection Accepted");
			Communicator r = new Communicator(clientSocket,diameterPeer,Communicator.Receiver);
		}
		closeSocket();
	}
	
	
	
	
}
