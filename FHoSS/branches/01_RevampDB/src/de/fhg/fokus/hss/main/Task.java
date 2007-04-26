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

package de.fhg.fokus.hss.main;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.hss.diam.DiameterConstants;
import de.fhg.fokus.hss.sh.op.PNR;
import de.fhg.fokus.hss.sh.op.PUR;
import de.fhg.fokus.hss.sh.op.SNR;
import de.fhg.fokus.hss.sh.op.UDR;

import de.fhg.fokus.hss.cx.op.*;
/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class Task {
	public int interface_type;
	public int command_code;
	public boolean isRequest;
	public String FQDN;
	public DiameterPeer peer;
	public DiameterMessage message;
	
	public Task (int interface_type, int command_code, boolean isRequest, String FQDN, DiameterPeer peer, DiameterMessage message){
		this.interface_type = interface_type;
		this.command_code = command_code;
		this.isRequest = isRequest;
		this.FQDN = FQDN;
		this.peer = peer;
		this.message = message;
	}
	
	public DiameterMessage execute (){
		DiameterMessage response = null;
		
		if (interface_type == DiameterConstants.Application.Cx){
			
			// Cx commands
			switch (command_code){
				case DiameterConstants.Command.LIR:
					System.out.println("Processing LIR!");
					response = LIR.processRequest(peer, message);
					peer.sendMessage(FQDN, response);
					break;
				
				case DiameterConstants.Command.MAR:
					System.out.println("Processing MAR!");
					response = MAR.processRequest(peer, message);
					peer.sendMessage(FQDN, response);
					break;
				
				case DiameterConstants.Command.PPR:
					if (isRequest){
						PPR.processAnswer(peer, message);
					}
					else{
						PPR.sendRequest(peer, message);
					}
					break;
				
				case DiameterConstants.Command.RTR:
					//RTR.sendRequest(peer, )
					break;
				case DiameterConstants.Command.SAR:
					System.out.println("Processing SAR!");
					response = SAR.processRequest(peer, message);
					peer.sendMessage(FQDN, response);
					break;
				case DiameterConstants.Command.UAR:
					System.out.println("Processing UAR!");
					response = UAR.processRequest(peer, message);
					peer.sendMessage(FQDN, response);
					break;
			}
		}
		
		else if (interface_type == DiameterConstants.Application.Sh){
			// Sh Commands
			switch(message.commandCode){
			
				case DiameterConstants.Command.UDR:
					
					response = UDR.processRequest(peer, message);
					break;
			
				case DiameterConstants.Command.PUR:
					response = PUR.processRequest(peer, message);
					break;
			
				case DiameterConstants.Command.SNR:
					response = SNR.processRequest(peer, message);
					break;
		
				case DiameterConstants.Command.PNA:
					if (isRequest){
						PNR.processAnswer(message);
					}
					else{
						// send request
					}
					break;
			}
		}
		else if (interface_type == DiameterConstants.Application.Zh){
			// Zh commands
		}
		
		return response;
	}
}
