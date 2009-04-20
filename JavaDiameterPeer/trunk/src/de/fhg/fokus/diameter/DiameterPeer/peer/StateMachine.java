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

import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Iterator;
import java.util.concurrent.TimeUnit;

import org.apache.log4j.Logger;

import de.fhg.fokus.diameter.DiameterPeer.data.AVP;
import de.fhg.fokus.diameter.DiameterPeer.data.AVPDecodeException;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterCEA;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterCER;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterDPA;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterDPR;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterDWA;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterDWR;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterTask;
import de.fhg.fokus.diameter.DiameterPeer.transport.Communicator;

/**
 * This class defines the Diameter Peer State Machine. 
 * 
 * @author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */
public class StateMachine {
	
	/** The logger */
	private static final Logger LOGGER = Logger.getLogger(StateMachine.class);

	/* State definitions */
	/** Not connected. */
	public static final int Closed 				= 0; 
	
	/** Connecting - waiting for Ack. */
	public static final int Wait_Conn_Ack		= 1;
	
	/** Connecting - waiting for Capabilities-Exchange-Answer. */
	public static final int Wait_I_CEA 			= 2;
	
	/** Connecting - Acknowledged and going for Election. */
	public static final int Wait_Conn_Ack_Elect	= 3;
	
	/** Connecting - done. */
	public static final int Wait_Returns  		= 4;
	
	/** Connected as receiver. */
	public static final int R_Open 				= 5;
	
	/** Connected as initiator. */
	public static final int I_Open 				= 6;
	
	/** Closing the connection. */
	public static final int Closing 			= 7;
	
	
	
	
	/* Event definitions */
	/** Start connection attempt. */
	public static final int Start			= 101;

	/** Stop. */
	public static final int Stop			= 102;
	
	/** Time-out */
	public static final int Timeout			= 103;
	
	/** Winning the election */
	public static final int Win_Election	= 104;
	
	/** Receiver - Received connection Capabilities-Exchange-Request. */
	public static final int R_Conn_CER		= 105;
	
	/** Initiator - Received connection Ack. */
	public static final int I_Rcv_Conn_Ack 	= 106;
	
	/** Initiator - Received connection NAck. */
	public static final int I_Rcv_Conn_NAck	= 107;
	
	/** Initiator - Received Capabilities-Exhange-Request. */
	public static final int I_Rcv_CER		= 108;

	/** Initiator - Recieved Capabilities-Exchange-Request. */
	public static final int I_Rcv_CEA		= 109;
	
	/** Receiver - Receiver Capabilities-Exchange-Request. */
	public static final int R_Rcv_CER		= 110;
	
	/** Receiver - Receiver Capabilities-Exchange-Answer. */
	public static final int R_Rcv_CEA		= 111;
	
	/** Initiator - Received non-Capabilities-Exchange-Answer. */
	public static final int I_Rcv_Non_CEA	= 112;
	
	/** Initiator - Received Disconnect-Peer-Request. */
	public static final int I_Rcv_DPR		= 113;
	
	/** Initiator - Received Disconnect-Peer-Answer. */
	public static final int I_Rcv_DPA		= 114;
	
	/** Receiver - Received Disconnect-Peer-Request. */
	public static final int R_Rcv_DPR		= 115;
	
	/** Receiver - Received Disconnect-Peer-Answer. */
	public static final int R_Rcv_DPA		= 116;
	
	/** Initiator - Received Diameter-Watchdog-Request. */
	public static final int I_Rcv_DWR		= 117;
	
	/** Initiator - Received Diameter-Watchdog-Answer. */
	public static final int I_Rcv_DWA		= 118;
	
	/** Receiver - Received Diameter-Watchdog-Request. */
	public static final int R_Rcv_DWR		= 119;
	
	/** Receiver - Received Diameter-Watchdog-Answer. */
	public static final int R_Rcv_DWA		= 120;
	
	/** Send a message. */
	public static final int Send_Message	= 121;
	
	/** Initiator - Received a message. */
	public static final int I_Rcv_Message	= 122;
	
	/** Receiver - Received a message. */
	public static final int R_Rcv_Message	= 123;
	
	/** Initiator - Peer disconnected. */
	public static final int I_Peer_Disc		= 124;
	
	/** Receiver - Peer disconnected. */
	public static final int R_Peer_Disc		= 125;
	
	/**
	 * Process a transition in the state machine.
	 * 
	 * @param p     The peer for which the event happened.
	 * @param event The event happened.
	 * @return  1 one success, 0 on error. Also the peer states are updated.
	 */
	public static int process(Peer p, int event)
	{
		return process(p,event,null,null);
	}
	
	
	
	/**
	 * Process a transition in the state machine.
	 * 
	 * @param p		The peer for which the event happend.
	 * @param event The event happened.
	 * @param msg   Received message.
	 * @return 1 one success, 0 one error. Also the peer states are upated.
	 */
	public static int process(Peer p, int event, DiameterMessage msg)
	{
		return process(p,event,msg,null);
	}
	
	
	/**
	 * Process a transition in the state machine.
	 * 
	 * @param p     The peer for which the event happend.
	 * @param event The event happened.
	 * @param msg   Received message.
	 * @param comm  Communicator used to send a DiameterMessage
	 * @return 1 one success, 0 one error. Also the peer states are upated.
	 */
	public static int process(Peer p, int event, DiameterMessage msg,Communicator comm)
	{
		int next_event,result_code;
		boolean msg_received=false;
		//LOGGER.debug("Peer Old State: "+p.state+" FQDN:"+p.FQDN);

	synchronized(p){
		switch (p.state){
			case Closed:
				switch (event){
					case Start:
						p.state = Wait_Conn_Ack;
						next_event = I_Snd_Conn_Req(p);
						StateMachine.process(p,next_event,null,p.I_comm);
						break;
					case R_Conn_CER:
						R_Accept(p,comm);
						result_code = Process_CER(p,msg);
						Snd_CEA(p,msg,result_code,p.R_comm);
						if (result_code>=2000 && result_code<3000)
							p.state = R_Open;
						else {
							R_Disc(p);
							p.state = Closed;
						}
						break;
					case Stop:
						/* just ignore this state */
						p.state = Closed;
						break;
					default:
						LOGGER.error("StateMachine: Invalid event "+event+" for state "+p.state);
						return 0;
				}
				break;
			case Wait_Conn_Ack:
				switch(event){
					case I_Rcv_Conn_Ack:
						I_Snd_CER(p);
						p.state = Wait_I_CEA;
						break;	
					case I_Rcv_Conn_NAck:
						Cleanup(p,comm);
						p.state = Closed;
						break;
/* Commented as not reachable*/						
//					case R_Conn_CER:
//						R_Accept(p,comm);
//						result_code = Process_CER(p,msg);
//						if (result_code>=2000 && result_code<3000)
//							p.state = Wait_Conn_Ack_Elect;
//						else {
//							p.state = Wait_Conn_Ack;
//							comm.shutdown();
//						}
//						break;
					case Timeout:
						Error(p,p.I_comm);
						p.state = Closed;
					default:
						LOGGER.error("StateMachine: Invalid event "+event+" for state "+p.state);
						return 0;
				}
				break;
			case Wait_I_CEA:
				switch(event){
					case I_Rcv_CEA:
						result_code = Process_CEA(p,msg);
						//if (result_code>=2000 && result_code<3000)
							p.state = I_Open; 
						//else {
						//	Cleanup(p,p.I_comm);
						//	p.state = Closed;
						//}
						break;
					case R_Conn_CER:
						R_Accept(p,comm);
						result_code = Process_CER(p,msg);
						p.state = Wait_Returns;
						if (Elect(p,msg))
							StateMachine.process(p,Win_Election,msg,comm);
						break;
					case I_Peer_Disc:
						I_Disc(p);
						p.state = Closed;
						break;
					case I_Rcv_Non_CEA:
						Error(p,p.I_comm);
						p.state = Closed;
						break;
					case Timeout:
						Error(p,p.I_comm);
						p.state = Closed;
						break;
					default:
						LOGGER.error("StateMachine: Invalid event "+event+" for state "+p.state);
						return 0;
				}
				break;
/* commented as not reachable */
//			case Wait_Conn_Ack_Elect:
//				switch(event){
//					default:
//						LOGGER.error("StateMachine: Invalid event "+event+" for state "+p.state);
//						return 0;
//				}
//				break;
			case Wait_Returns:
				switch(event){
					case Win_Election:
						I_Disc(p);
						result_code = Process_CER(p,msg);
						Snd_CEA(p,msg,result_code,p.R_comm);
						if (result_code>=2000 && result_code<3000){
							p.state = R_Open;
						}else{
							R_Disc(p);
							p.state = Closed;
						}
						break;
					case I_Peer_Disc:
						I_Disc(p);
						result_code = Process_CER(p,msg);
						Snd_CEA(p,msg,result_code,p.R_comm);
						if (result_code>=2000 && result_code<3000){
							p.state = R_Open;
						}else{
							R_Disc(p);
							p.state = Closed;
						}
						break;
					case I_Rcv_CEA:
						R_Disc(p);
						//result_code = Process_CEA(p,msg);
						//if (result_code>=2000 && result_code<3000)
							p.state = I_Open; 
						//else {
						//	Cleanup(p,p.I_comm);
						//	p.state = Closed;
						//}
						break;
					case R_Peer_Disc:
						R_Disc(p);
						p.state = Wait_I_CEA;
						break;
					case R_Conn_CER:
						R_Reject(p,comm);
						p.state = Wait_Returns;
						break;
					case Timeout:
						Error(p,comm);
						p.state = Closed;
					default:
						LOGGER.error("StateMachine: Invalid event "+event+" for state "+p.state);
						return 0;
				}
				break;
			case R_Open:
				switch (event){
					case Send_Message:
						Snd_Message(p,msg);
						p.state = R_Open;
						break;
					case R_Rcv_Message:
						// delayed processing until out of the critical zone
						//Rcv_Process(p,msg);
						msg_received = true;
						p.state = R_Open;
						break;
					case R_Rcv_DWR:
						result_code = Process_DWR(p,msg);
						Snd_DWA(p,msg,result_code,p.R_comm);
						p.state = R_Open;
						break;
					case R_Rcv_DWA:
						Process_DWA(p,msg);
						p.state = R_Open;
						break;
					case R_Conn_CER:
						R_Reject(p,comm);
						p.state = R_Open;
						break;
					case Stop:
						Snd_DPR(p);
						p.state = Closing;
						break;
					case R_Rcv_DPR:
						Snd_DPA(p,msg,DiameterMessage.DIAMETER_SUCCESS,p.R_comm);
						R_Disc(p);
						p.state = Closed;
						break;
					case R_Peer_Disc:
						R_Disc(p);
						p.state = Closed;
						break;
					case R_Rcv_CER:
						result_code = Process_CER(p,msg);
						Snd_CEA(p,msg,result_code,p.R_comm);
						if (result_code>=2000 && result_code<3000)
							p.state = R_Open;
						else {
							/*R_Disc(p);p.state = Closed;*/
							p.state = R_Open; /* Or maybe I should disconnect it?*/
						}
						break;
					case R_Rcv_CEA:
						result_code = Process_CEA(p,msg);
						if (result_code>=2000 && result_code<3000)
							p.state = R_Open;
						else {
							/*R_Disc(p);p.state = Closed;*/
							p.state = R_Open; /* Or maybe I should disconnect it?*/
						}
						break;
					default:
						LOGGER.error("StateMachine: Invalid event "+event+" for state "+p.state);
						return 0;
				}
				break;
			case I_Open:
				switch (event){
					case Send_Message:
						Snd_Message(p,msg);
						p.state = I_Open;
						break;
					case I_Rcv_Message:
						// delayed processing until out of the critical zone
						//Rcv_Process(p,msg);
						msg_received = true;
						p.state = I_Open;
						break;
					case I_Rcv_DWR:
						result_code = Process_DWR(p,msg);
						Snd_DWA(p,msg,result_code,p.I_comm);						
						p.state =I_Open;
						break;
					case I_Rcv_DWA:
						Process_DWA(p,msg);
						p.state =I_Open;
						break;
					case R_Conn_CER:
						R_Reject(p,comm);
						p.state = I_Open;
						break;
					case Stop:
						Snd_DPR(p);
						p.state = Closing;
						break;
					case I_Rcv_DPR:
						Snd_DPA(p,msg,2001,p.I_comm);
						R_Disc(p);
						p.state = Closed;
						break;
					case I_Peer_Disc:
						I_Disc(p);
						p.state = Closed;
						break;
					case I_Rcv_CER:
						result_code = Process_CER(p,msg);
						Snd_CEA(p,msg,result_code,p.I_comm);
						if (result_code>=2000 && result_code<3000)
							p.state = I_Open;
						else {
							/*I_Disc(p);p.state = Closed;*/
							p.state = I_Open; /* Or maybe I should disconnect it?*/
						}
						break;
					case I_Rcv_CEA:
						result_code = Process_CEA(p,msg);
						if (result_code>=2000 && result_code<3000)
							p.state = I_Open;
						else {
							/*I_Disc(p);p.state = Closed;*/
							p.state = I_Open; /* Or maybe I should disconnect it?*/
						}
						break;
					default:
						LOGGER.error("StateMachine: Invalid event "+event+" for state "+p.state);
						return 0;
				}
				break;				
			case Closing:
				switch(event){
					case I_Rcv_DPA:
						I_Disc(p);
						p.state = Closed;
						break;
					case R_Rcv_DPA:
						R_Disc(p);
						p.state = Closed;
						break;
					case Timeout:
						if (p.I_comm!=null) Error(p,p.I_comm);
						if (p.R_comm!=null) Error(p,p.R_comm);
						p.state = Closed;
						break;
					case I_Peer_Disc:
						I_Disc(p);
						p.state = Closed;
						break;
					case R_Peer_Disc:
						R_Disc(p);
						p.state = Closed;
						break;
					default:
						LOGGER.error("StateMachine: Invalid event "+event+" for state "+p.state);
						return 0;
				}
				break;
			default:
				LOGGER.error("StateMachine: Invalid state "+p.state);
				return 0;
				
		}
		//LOGGER.debug("Peer New State: "+p.state+" FQDN:"+p.FQDN);
	}
		if (msg_received){
			// delayed processing until out of the critical zone
			Rcv_Process(p,msg);
		}
		
		return 1;
	}
	
	private static int I_Snd_Conn_Req(Peer p)
	{
		Socket s;
		if (p.I_comm!=null) p.I_comm.shutdown();
		p.I_comm = null;
		try {
			s = new Socket(p.FQDN,p.port);
		} catch (UnknownHostException e1) {
			LOGGER.error("StateMachine: Peer "+p.FQDN+" can not be resolved.");
			return StateMachine.I_Rcv_Conn_NAck;
		} catch (IOException e1) {
			LOGGER.error("StateMachine: Peer "+p.FQDN+":"+p.port+" not responding to connection attempt ");
			return StateMachine.I_Rcv_Conn_NAck;
		}
		Communicator r = new Communicator(s,p,Communicator.Initiator);
		p.I_comm = r;
		return StateMachine.I_Rcv_Conn_Ack;
	}

	private static void R_Accept(Peer p,Communicator comm)
	{
		p.R_comm = comm;
		p.refreshTimer();
	}

	private static void R_Reject(Peer p,Communicator comm)
	{
		comm.shutdown();
	}
	
	private static void I_Snd_CER(Peer p)
	{
		DiameterCER cer = new DiameterCER();
		
		cer.hopByHopID = p.diameterPeer.getNextHopByHopId();
		cer.endToEndID = p.diameterPeer.getNextEndToEndId();
		cer.origin_host.setData(p.diameterPeer.FQDN);
		cer.origin_realm.setData(p.diameterPeer.Realm);
		byte addr[] ,laddr[] = p.I_comm.socket.getLocalAddress().getAddress();
		switch (laddr.length) {
			case 16:
				addr = new byte[18];			
				addr[0]=(byte)0;
				addr[1]=(byte)2;
				System.arraycopy(laddr,0,addr,2,laddr.length);
				break;			
			default:
			case 4:
				addr = new byte[6];			
				addr[0]=(byte)0;
				addr[1]=(byte)1;
				System.arraycopy(laddr,0,addr,2,laddr.length);
				break;			
		}
		cer.host_ip_address.setData(addr);
		cer.vendor_id.setData(p.diameterPeer.Vendor_Id);
		cer.product_name.setData(p.diameterPeer.Product_Name);
		
		Snd_CE_add_applications(cer,p);
		
		//LOGGER.debug(cer.toString());
		p.I_comm.sendDirect(cer);
	}
	
	
	private static void Cleanup(Peer p,Communicator comm)
	{
		if (comm==null) return;
		comm.shutdown();
		if (p.I_comm == comm) p.I_comm = null;
		if (p.R_comm == comm) p.R_comm = null;
	}

	private static void Error(Peer p, Communicator comm)
	{
		Cleanup(p,comm);
	}

	private static boolean Elect(Peer p,DiameterMessage cer)
	{
		/* returns if we win the election */
		AVP avp;
		byte[] remote,local;
		int x,i;
		local = p.diameterPeer.FQDN.getBytes();
		avp = cer.findAVP(AVP.Origin_Host,true,0);
		if (avp==null) {
			return true;
		}else{
			remote = avp.data;
			for(i=0;i<remote.length&&i<local.length;i++){
				x = ((int) local[i]&0xFF)-((int) remote[i]&0xFF);
				if (x>0) return true;
				if (x<0) return false;
			}
			if (local.length>remote.length) return true;
			return false;
		}
	}

	private static int Process_CER(Peer p,DiameterMessage cer)
	{
		int common_app=0;
		Iterator<AVP> i = cer.avps.iterator();
		Iterator<Application> i2;
		Application app;
		AVP avp,avp_vendor,avp2;
		p.AuthApp.clear();
		p.AcctApp.clear();
		while(i.hasNext()&& common_app==0){
			avp = (AVP) i.next();
			switch (avp.code){
				case AVP.Auth_Application_Id:
					p.AuthApp.add(new Application(avp.int_data,0,Application.Auth));
					i2 = p.diameterPeer.AuthApp.iterator();
					while(i2.hasNext()){
						app = i2.next();
						if (avp.int_data==Application.Relay ||
							(app.id==avp.int_data && app.vendor==0)){
							common_app++;
							break;
						}
					}
					break;
				case AVP.Acct_Application_Id:
					p.AcctApp.add(new Application(avp.int_data,0,Application.Acct));
					i2 = p.diameterPeer.AcctApp.iterator();
					while(i2.hasNext()){
						app = i2.next();
						if (avp.int_data==Application.Relay ||
							(app.id==avp.int_data && app.vendor==0)){
							common_app++;
							break;
						}
					}
					break;
				case AVP.Vendor_Specific_Application_Id:
					try {
						avp.ungroup();
					} catch (AVPDecodeException e) {
						e.printStackTrace();
					}
					avp_vendor = avp.findChildAVP(AVP.Vendor_Id,true,0);
					if (avp_vendor==null) break;
					avp2 = avp.findChildAVP(AVP.Auth_Application_Id,true,0);
					if (avp2!=null) {
						p.AuthApp.add(new Application(avp2.int_data,avp_vendor.int_data,Application.Auth));
						i2 = p.diameterPeer.AuthApp.iterator();
						while(i2.hasNext()){
							app = i2.next();
							if (avp2.int_data==Application.Relay ||
								(app.id==avp2.int_data && app.vendor==avp_vendor.int_data)){
								common_app++;
								break;
							}
						}
					}
					avp2 = avp.findChildAVP(AVP.Acct_Application_Id,true,0);
					if (avp2!=null) {
						p.AcctApp.add(new Application(avp2.int_data,avp_vendor.int_data,Application.Acct));
						i2 = p.diameterPeer.AcctApp.iterator();
						while(i2.hasNext()){
							app = (Application) i2.next();
							if (avp2.int_data==Application.Relay ||
								(app.id==avp2.int_data && app.vendor==avp_vendor.int_data)){
								common_app++;
								break;
							}
						}
					}
					break;
					
			}
		}
		
		if (common_app!=0)
			return DiameterMessage.DIAMETER_SUCCESS;
		else return DiameterMessage.DIAMETER_NO_COMMON_APPLICATION;
	}

	private static int Process_CEA(Peer p,DiameterMessage cea)
	{
		AVP avp;
		avp = cea.findAVP(AVP.Result_Code,true,0);
		if (avp==null) return DiameterMessage.DIAMETER_UNABLE_TO_COMPLY;
		else return avp.int_data;
	}
	
	private static void Snd_CEA(Peer p,DiameterMessage cer,int result_code,Communicator comm)
	{
		DiameterCEA cea;
		cea = new DiameterCEA(result_code);
		cea.hopByHopID = cer.hopByHopID;
		cea.endToEndID = cer.endToEndID;
		cea.origin_host.setData(p.diameterPeer.FQDN);
		cea.origin_realm.setData(p.diameterPeer.Realm);
		byte addr[] ,laddr[] = p.R_comm.socket.getLocalAddress().getAddress();
		switch (laddr.length) {
			case 16:
				addr = new byte[18];			
				addr[0]=(byte)0;
				addr[1]=(byte)2;
				System.arraycopy(laddr,0,addr,2,laddr.length);
				break;			
			default:
			case 4:
				addr = new byte[6];
				
				addr[0]=(byte)0;
				addr[1]=(byte)1;
				System.arraycopy(laddr,0,addr,2,laddr.length);
				break;			
		}
		cea.host_ip_address.setData(addr);
		cea.vendor_id.setData(p.diameterPeer.Vendor_Id);
		cea.product_name.setData(p.diameterPeer.Product_Name);
		Snd_CE_add_applications(cea,p);
		
		//LOGGER.debug(cea.toString());
		comm.sendDirect(cea);
	}
	
	private static void I_Disc(Peer p)
	{
		if (p.I_comm!=null)
		p.I_comm.shutdown();
		p.I_comm = null;
	}
	private static void R_Disc(Peer p)
	{
		if (p.R_comm!=null)
			p.R_comm.shutdown();
		p.R_comm = null;
	}
	
	private static void Snd_CE_add_applications(DiameterMessage msg,Peer p)
	{
		AVP avp,avp2,avp3;
		Iterator i;
		Application app;
		i = p.diameterPeer.AuthApp.iterator();
		while(i.hasNext()){
			app = (Application) i.next();
			if (app.vendor==0){
				avp = new AVP(AVP.Auth_Application_Id,true,0);
				avp.setData(app.id);
				msg.addAVP(avp);
			}else{
				avp = new AVP(AVP.Vendor_Specific_Application_Id,true,0);
				
				avp2 = new AVP(AVP.Vendor_Id,true,0);
				avp2.setData(app.vendor);
				avp.addChildAVP(avp2);
				
				avp3 = new AVP(AVP.Auth_Application_Id,true,0);
				avp3.setData(app.id);
				avp.addChildAVP(avp3);
				
				avp.group();
				msg.addAVP(avp);
			}
		}
		i = p.diameterPeer.AcctApp.iterator();
		while(i.hasNext()){
			app = (Application) i.next();
			if (app.vendor==0){
				avp = new AVP(AVP.Acct_Application_Id,true,0);
				avp.setData(app.id);
				msg.addAVP(avp);
			}else{
				avp = new AVP(AVP.Vendor_Specific_Application_Id,true,0);
				
				avp2 = new AVP(AVP.Vendor_Id,true,0);
				avp2.setData(app.vendor);
				avp.addChildAVP(avp2);
				
				avp3 = new AVP(AVP.Acct_Application_Id,true,0);
				avp3.setData(app.id);
				avp.addChildAVP(avp3);
				
				avp.group();
				msg.addAVP(avp);
			}
		}

	}

	private static int Process_DWR(Peer p,DiameterMessage dwr)
	{
		return DiameterMessage.DIAMETER_SUCCESS;
	}

	private static void Process_DWA(Peer p,DiameterMessage dwa)
	{
		p.waitingDWA = false;
	}

	/**
	 * Send Device-Watchdog-Request.
	 * 
	 * @param p Peer sending the request.
	 * @return true if sending successful, false otherwise. 
	 */
	public static boolean Snd_DWR(Peer p)
	{
		DiameterDWR dwr;
		boolean r=false;
		
		dwr = new DiameterDWR();
		dwr.hopByHopID = p.diameterPeer.getNextHopByHopId();
		dwr.endToEndID = p.diameterPeer.getNextEndToEndId();
		dwr.origin_host.setData(p.diameterPeer.FQDN);
		dwr.origin_realm.setData(p.diameterPeer.Realm);
		
		if (p.state==I_Open)
			r = p.I_comm.sendDirect(dwr);
		if (p.state==R_Open)
			r = p.R_comm.sendDirect(dwr);
		return r;
	}

	private static void Snd_DWA(Peer p,DiameterMessage dwr,int result_code,Communicator comm)
	{
		DiameterDWA dwa;
	
		dwa = new DiameterDWA(result_code);
		dwa.hopByHopID = dwr.hopByHopID;
		dwa.endToEndID = dwr.endToEndID;
		dwa.origin_host.setData(p.diameterPeer.FQDN);
		dwa.origin_realm.setData(p.diameterPeer.Realm);
		
		LOGGER.debug(dwa.toString());
		comm.sendDirect(dwa);
	}

	
	/**
	 * Send Disconnect-Peer-Request.
	 * 
	 * @param p Peer sending the message.
	 */
	public static void Snd_DPR(Peer p)
	{
		DiameterDPR dpr;
		
		dpr = new DiameterDPR();
		dpr.hopByHopID = p.diameterPeer.getNextHopByHopId();
		dpr.endToEndID = p.diameterPeer.getNextEndToEndId();
		dpr.origin_host.setData(p.diameterPeer.FQDN);
		dpr.origin_realm.setData(p.diameterPeer.Realm);
		dpr.disconnect_cause.setData(0/*Busy*/);
		
		LOGGER.debug(dpr.toString());
		if (p.state==I_Open)
			p.I_comm.sendDirect(dpr);
		if (p.state==R_Open)
			p.R_comm.sendDirect(dpr);
	}

	private static void Snd_DPA(Peer p,DiameterMessage dpr,int result_code,Communicator comm)
	{
		DiameterDPA dpa;
	
		dpa = new DiameterDPA(result_code);
		dpa.hopByHopID = dpr.hopByHopID;
		dpa.endToEndID = dpr.endToEndID;
		dpa.origin_host.setData(p.diameterPeer.FQDN);
		dpa.origin_realm.setData(p.diameterPeer.Realm);
		
		LOGGER.debug(dpa.toString());
		comm.sendDirect(dpa);
	}
	
	
	/**
	 * Send a Diameter message.
	 * 
	 * @param p   Peer sending the Diameter message.
	 * @param msg Diameter message being sent.
	 */
	public static void Snd_Message(Peer p, DiameterMessage msg)
	{
		p.refreshTimer();
	}

	
	/**
	 * Process the received Diameter message.
	 * 
	 * @param p		Peer receiving the Diameter message.
	 * @param msg   The received Diameter message.
	 */
	public static void Rcv_Process(Peer p, DiameterMessage msg)
	{
		boolean done=false;
		if (p.diameterPeer.eventListeners.size()!=0){
			while(!done)
			{
				try {
					done = p.diameterPeer.queueTasks.offer(new DiameterTask(p,msg),1000,TimeUnit.MILLISECONDS);
				} catch (InterruptedException e) {
					e.printStackTrace();
					break;
				}
//				if (!done) LOGGER.debug("ful "+p.diameterPeer.queueTasks.size());
//				else LOGGER.debug("put "+p.diameterPeer.queueTasks.size());
//				//LOGGER.debug("StateMachine: Processing queue is full. Overload...");
			}
		}
		//LOGGER.debug("StateMachine: Received message "+msg.toString()+"---");		
	}
}
