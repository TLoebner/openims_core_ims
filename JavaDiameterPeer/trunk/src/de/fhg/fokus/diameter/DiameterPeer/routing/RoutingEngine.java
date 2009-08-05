/*
 * $Id: RoutingEngine.java 2 2006-11-14 22:37:20Z vingarzan $
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

package de.fhg.fokus.diameter.DiameterPeer.routing;

import java.util.TreeMap;
import java.util.Vector;

import de.fhg.fokus.diameter.DiameterPeer.data.AVP;
import de.fhg.fokus.diameter.DiameterPeer.data.AVPDecodeException;
import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;
import de.fhg.fokus.diameter.DiameterPeer.peer.Application;
import de.fhg.fokus.diameter.DiameterPeer.peer.Peer;
import de.fhg.fokus.diameter.DiameterPeer.peer.PeerManager;
import de.fhg.fokus.diameter.DiameterPeer.peer.StateMachine;

public class RoutingEngine {
	 
	Vector<RoutingEntry> defaultRoutes;
	TreeMap<String,RoutingRealm> realms;

	public RoutingEngine()
	{
		defaultRoutes = new Vector<RoutingEntry>();
		realms = new TreeMap<String, RoutingRealm>();
	}
	
	public void addDefaultRoute(String FQDN,int metric){		
		RoutingEntry re = new RoutingEntry(FQDN,metric);
		for (RoutingEntry i : defaultRoutes) 
			if (i.metric<=metric){
				defaultRoutes.add(defaultRoutes.indexOf(i), re);
				break;
			}
	}
	
	public void addRealmRoute(String realm, String FQDN, int metric)
	{
		RoutingRealm rr;
		rr = realms.get(realm);
		if (rr==null){
			rr = new RoutingRealm(realm);
			this.realms.put(realm, rr);
		}
		rr.addRoute(FQDN,metric);
	}

	public Peer getRoute(DiameterMessage msg, PeerManager peerManager) {
		String destinationHost=null,destinationRealm=null;
		AVP avp;
		Peer p;
		Application application=null;
		
		avp = msg.findAVP(AVP.Destination_Host);
		if (avp!=null) destinationHost = new String(avp.getData());
		if (destinationHost!=null && destinationHost.length()>0){
			p = peerManager.getPeerByFQDN(destinationHost);
			if (p!=null && (p.state==StateMachine.I_Open || p.state==StateMachine.R_Open))
				return p;
		}
		avp = msg.findAVP(AVP.Destination_Realm);
		if (avp!=null) destinationRealm = new String(avp.getData());
		application = getApplication(msg);
		if (destinationRealm!=null && destinationRealm.length()>0){
			RoutingRealm rr = realms.get(destinationRealm);
			if (rr!=null){
				for (RoutingEntry i : rr.routes) {
					p = peerManager.getPeerByFQDN(i.FQDN);
					if (p!=null && 
						(p.state==StateMachine.I_Open || p.state==StateMachine.R_Open) &&
						(application==null || p.handlesApplication(application))
						)
						return p;					
				}
			}
		}
		for (RoutingEntry i : defaultRoutes) {
			p = peerManager.getPeerByFQDN(i.FQDN);
			if (p!=null && (p.state==StateMachine.I_Open || p.state==StateMachine.R_Open) &&
				(application==null || p.handlesApplication(application))
				)
				return p;					
		}
		return null;
	}

	private Application getApplication(DiameterMessage msg) {
		AVP avp,avp_vendor,avp2;
		int vendorId=0,appId;
	
		appId = msg.applicationID;	
		
		avp_vendor = msg.findAVP(AVP.Vendor_Id);				
		avp = msg.findAVP(AVP.Auth_Application_Id);
		if (avp!=null){
			if (avp_vendor!=null) vendorId = avp_vendor.getIntData();
			else vendorId = 0;
			appId = avp.getIntData();
			return new Application(appId,vendorId,Application.Auth);
		}

		avp = msg.findAVP(AVP.Acct_Application_Id);
		if (avp!=null){
			if (avp_vendor!=null) vendorId = avp_vendor.getIntData();
			else vendorId = 0;
			appId = avp.getIntData();
			return new Application(appId,vendorId,Application.Acct);
		}
				
		avp = msg.findAVP(AVP.Vendor_Specific_Application_Id);
		if (avp!=null){
			try {
				avp.ungroup();
				avp_vendor = avp.findChildAVP(AVP.Vendor_Id);
				if (avp_vendor!=null) vendorId = avp_vendor.getIntData();
				else vendorId = 0;
				avp2 = avp.findChildAVP(AVP.Auth_Application_Id);				
				if (avp_vendor!=null&&avp2!=null){
					vendorId = avp_vendor.getIntData();				
					appId = avp2.getIntData();
					return new Application(appId,vendorId,Application.Auth);
				}
				avp2 = avp.findChildAVP(AVP.Acct_Application_Id);				
				if (avp_vendor!=null&&avp2!=null){
					vendorId = avp_vendor.getIntData();				
					appId = avp2.getIntData();
					return new Application(appId,vendorId,Application.Acct);
				}
			} catch (AVPDecodeException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		return new Application(appId,vendorId,Application.Unknown);

	}
	
	
	
	
}

class RoutingEntry {
	String FQDN;
	int metric;
	public RoutingEntry(String FQDN, int metric) {
		this.FQDN = FQDN;
		this.metric = metric;
	}
}

class RoutingRealm {
	String realm;
	Vector<RoutingEntry> routes;
	public RoutingRealm(String realm) {
		this.realm = realm;
		this.routes = new Vector<RoutingEntry>();
	}
	public void addRoute(String FQDN,int metric){		
		RoutingEntry re = new RoutingEntry(FQDN,metric);
		for (RoutingEntry i : routes) 
			if (i.metric<=metric){
				routes.add(routes.indexOf(i), re);
				break;
			}
	}
}