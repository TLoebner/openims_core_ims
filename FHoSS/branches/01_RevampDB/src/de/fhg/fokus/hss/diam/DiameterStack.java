/**
 * 
 */
package de.fhg.fokus.hss.diam;

import de.fhg.fokus.diameter.DiameterPeer.DiameterPeer;
import de.fhg.fokus.diameter.DiameterPeer.EventListener;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.diameter.DiameterPeer.transaction.TransactionListener;
import de.fhg.fokus.hss.cx.Cx;
import de.fhg.fokus.hss.sh.Sh;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class DiameterStack implements TransactionListener, EventListener {
	public DiameterPeer diameterPeer;
	
	public void configStack(){
		diameterPeer = new DiameterPeer("DiameterPeerHSS.xml");
		diameterPeer.enableTransactions(10, 1);
		diameterPeer.addEventListener(this);
	}
	
	public void receiveAnswer(String FQDN, DiameterMessage request, DiameterMessage answer) {
		System.out.println("\n\nTransactional Answer!");
	}

	public void timeout(DiameterMessage request) {
		System.out.println("Timeout occured!");
	}

	public void recvMessage(String FQDN, DiameterMessage request) {
		if (request.applicationID == DiameterConstants.Application.Cx){
			System.out.println("\n\nCx message received!");
			Cx.processCxMessage(diameterPeer, FQDN, request);
		}
		else if (request.applicationID == DiameterConstants.Application.Sh){
			System.out.println("\n\nSh message received!");
			Sh.processCxMessage(diameterPeer, FQDN, request);
		}
		else if (request.applicationID == DiameterConstants.Application.Zh){
			System.out.println("\n\nZh message received!");
		}
	}
}

