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

package de.fhg.fokus.diameter.DiameterPeer.transaction;

import de.fhg.fokus.diameter.DiameterPeer.data.DiameterMessage;

/**
 * \package de.fhg.fokus.diameter.DiameterPeer.transaction
 * Provides methods for a Diameter client to handle Diameter request and answer.
 * <p>
 * A Diameter request and its Diameter answer are bundled together as a 
 * transaction. 
 * A Diameter client uses a TransactionWorker to handle all transactions.
 */

/**
 * DiameterTransaction consists of a request and an answer.
 * 
 * Use this class at a Diameter client to sending a request and handle an 
 * answer. 
 * 
 * @author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */

public class DiameterTransaction {
	
	long expires=0;
	
	DiameterMessage request=null,answer=null;
	TransactionListener tl;
	boolean blocking=false;
	
	/**
	 * Construct a DiameterTransaction.
	 * 
	 * @param request	Diameter request.
	 * @param tl		Transaction listener which will handle the corresponding 
	 * 					Diameter answer.
	 * @param blocking  true, if the transaction should block the thread and wait 
	 * 					for the Diameter answer.
	 */
	public DiameterTransaction(DiameterMessage request,TransactionListener tl,boolean blocking) {
		super();
		// TODO Auto-generated constructor stub
		this.request = request;
		this.tl = tl;
		this.blocking = blocking;
	}
	
	
	/**
	 * Construct a DiameterTransaction.
	 * 
	 * @param request	Diameter request.
	 */
	public DiameterTransaction(DiameterMessage request) {
		super();
		// TODO Auto-generated constructor stub
		this.request = request;
		this.tl = null;
	}
	
}
