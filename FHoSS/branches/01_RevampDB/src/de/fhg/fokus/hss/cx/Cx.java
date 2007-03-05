/**
 * 
 */
package de.fhg.fokus.hss.cx;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.hss.cx.op.*;
import de.fhg.fokus.hss.diam.DiameterConstants;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class Cx {

	public static void processCxMessage(DiameterPeer diameterPeer, String FQDN, DiameterMessage message){
		
		DiameterMessage response = null;
		
		if (message.flagRequest){
			// we have a request
			switch(message.commandCode){
		
				case DiameterConstants.Command.UAR:
					response = UAR.processRequest(diameterPeer, message);
					break;
				
				case DiameterConstants.Command.SAR:
					response = SAR.processRequest(diameterPeer, message);
					break;
				
				case DiameterConstants.Command.MAR:
					response = MAR.processRequest(diameterPeer, message);
					break;
				
				case DiameterConstants.Command.LIR:
					response = LIR.processRequest(diameterPeer, message);
					break;
			}
			// send the response
			diameterPeer.sendMessage(FQDN, response);
		}
		else{
			// we have an answer
			switch(message.commandCode){
			
				case DiameterConstants.Command.PPA:
					PPR.processAnswer(diameterPeer, message);
					break;
			
				case DiameterConstants.Command.RTA:
					RTR.processAnswer(diameterPeer, message);
					break;
			}
		}
	}
}
