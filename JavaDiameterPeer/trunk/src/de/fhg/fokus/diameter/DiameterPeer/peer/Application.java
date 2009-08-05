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

/**
 * This class defines applications supported by a peer.
 * 
 * @author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */
public class Application {

	/** Application id */
	public int id;
	
	/** Vendor id */
	public int vendor;
	
	/** Type */
	public int type;
	
	public static final int Unknown = -1;

	/** Auth-application is identified with 0 */
	public static final int Auth = 0;
	
	/** Acct-application is identified with 1 */
	public static final int Acct = 1;
	
	
	/**
	 * Create a supported application.
	 * 
	 * @param id    	Application id
	 * @param vendor    Vendor id
	 * @param type      0 Auth-application, 1 Acct-application.
	 */
	public Application(int id, int vendor, int type) {
		super();
		this.id = id;
		this.vendor = vendor;
		this.type = type;
	}
	
	/** Id Oxffffffff is allocated for relay agent. */
	public static final int Relay=0xffffffff;

	public boolean equals(Application x)
	{
		return this.id == x.id &&
			this.vendor == x.vendor &&
			(this.type == x.type || this.type == Application.Unknown || x.type == Application.Unknown);
	}
}
