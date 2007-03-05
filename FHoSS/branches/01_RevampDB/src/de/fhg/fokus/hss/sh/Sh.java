/**
 * 
 */
package de.fhg.fokus.hss.sh;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.hss.diam.DiameterConstants;
import de.fhg.fokus.hss.sh.op.*;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class Sh {
	
	public static void processCxMessage(DiameterPeer diameterPeer, String FQDN, DiameterMessage message){
		
		DiameterMessage response = null;
		
		if (message.flagRequest){
			// we have a request
			switch(message.commandCode){
		
				case DiameterConstants.Command.UDR:
					response = UDR.processRequest(diameterPeer, message);
					break;
				
				case DiameterConstants.Command.PUR:
					response = PUR.processRequest(diameterPeer, message);
					break;
				
				case DiameterConstants.Command.SNR:
					response = SNR.processRequest(diameterPeer, message);
					break;
				
			}
			// send the response
			diameterPeer.sendMessage(FQDN, response);
		}
		else{
			// we have an answer
			switch(message.commandCode){
			
				case DiameterConstants.Command.PNA:
					PNR.processAnswer(message);
					break;
			}
		}
	}
}
