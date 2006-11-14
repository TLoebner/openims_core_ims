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

package de.fhg.fokus.diameter.DiameterPeer;

import org.apache.log4j.Logger;

import de.fhg.fokus.diameter.DiameterPeer.data.AVP;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.diameter.DiameterPeer.transaction.TransactionListener;



/**
 * This class tests the DiameterPeer for correct behaviour
 * 
 * @author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */
public class DiameterPeerTest {
	
	/** The logger */
	private static final Logger LOGGER = Logger.getLogger(DiameterPeerTest.class);

	public static DiameterMessage UAR(DiameterPeer dp)
	{
		AVP a,b;
		DiameterMessage uar = dp.newRequest(300,16777216);
		/*session-id*/
		a = new AVP(263,true,0);
		a.setData("test.open-ims.test;11271298949"+System.currentTimeMillis());
		uar.addAVP(a);
		/*destination-host*/
		a = new AVP(293,true,0);
		a.setData("hss.open-ims.test");
		/*destination-realm*/
		a = new AVP(283,true,0);
		a.setData("open-ims.test");
		/*vendor-specific app id */
		a = new AVP(260,true,0);
			b = new AVP(266,true,0);
			b.setData(10415);
			a.addChildAVP(b);
			b = new AVP(258,true,0);
			b.setData(16777216);
			a.addChildAVP(b);
		uar.addAVP(a);
		/*auth-session-state*/
		a = new AVP(277,true,0);
		a.setData(1);
		uar.addAVP(a);
		/*user-name*/
		a = new AVP(1,true,0);
		a.setData("alice@open-ims.test");		
		uar.addAVP(a);
		/*public-id*/
		a = new AVP(601,true,10415);
		a.setData("sip:alice@open-ims.test");
		uar.addAVP(a);
		/*visited-network-id*/
		a = new AVP(600,true,10415);
		a.setData("open-ims.test");
		uar.addAVP(a);

		return uar;
	}
	
	
	
	public static void test(String[] args) throws InterruptedException {
		
		if (args.length != 2) {
			LOGGER.error("Provide two XML config files as input");
			System.exit(0);
		} else {
			LOGGER.info("DiameterPeer Starting...");
			
			// Create a Diameter client.
			String filename = args[0];
			LOGGER.debug("Opening Config file: "+filename);
			DiameterPeer dp1 = new DiameterPeer(filename);
			dp1.enableTransactions(10,1);
			
			// Create a Diameter server.
			String filename2 = args[1];
			LOGGER.debug("Opening Config file: "+filename2);
			DiameterPeer dp2 = new DiameterPeer(filename2);
			TestEventListener eventlistener = new TestEventListener();
			eventlistener.diameterPeer = dp2;
			dp2.addEventListener(eventlistener);
			
			
			Thread.sleep(2000);
			DiameterMessage uar = UAR(dp1);
			
			DiameterMessage uaa = dp1.sendRequestBlocking("cartman",uar);
			if (uaa==null) LOGGER.debug("SendBlocking timed-out");
			else LOGGER.debug("SendBlocking answer: "+uaa.toString());
	
			Thread.sleep(600*1000);
		
			dp1.shutdown();
			Thread.sleep(10000);
			LOGGER.debug("... DiameterPeer Done");
		}
	}	
	
	
	
	public static void main(String[] args) throws InterruptedException {
		test(args);
	}
}

class TestEventListener implements EventListener{

	DiameterPeer diameterPeer;
	
	public void recvMessage(String FQDN, DiameterMessage msg) {
		int i;
		DiameterMessage response;
		response = diameterPeer.newResponse(msg);
		AVP a = new AVP(602,true,0);
		a.setData("HelloWorld!");
		
		for(i=0;i<10;i++){
			response.addAVP(a);
		}
		diameterPeer.sendMessage(FQDN,response);
	}
}

class TestTransactionListener implements TransactionListener{

	/** The logger */
	private final Logger LOG = Logger.getLogger(DiameterPeerTest.class);
	
	DiameterPeer diameterPeer;

	public void receiveAnswer(String FQDN, DiameterMessage request, DiameterMessage answer) {
		LOG.debug("Transaction received an answer: "+answer.toString());		
	}

	public void timeout(DiameterMessage request) {
		LOG.debug("Transaction received an timeout");
	}

}