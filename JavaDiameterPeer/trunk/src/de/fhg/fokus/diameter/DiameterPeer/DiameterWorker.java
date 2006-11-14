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


import java.util.concurrent.ArrayBlockingQueue;
import java.util.Vector;

import org.apache.log4j.Logger;

import de.fhg.fokus.diameter.DiameterPeer.data.DiameterTask;

/**
 * DiameterWorker gets a DiameterTask out of the task queue and delivers them
 * to the listeners for further processes, if they are defined.
 * 
 * @author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
public class DiameterWorker extends Thread {
	
	/** The logger */
	private static final Logger LOGGER = Logger.getLogger(DiameterWorker.class);

	int id;
	private boolean running;
	private ArrayBlockingQueue<DiameterTask> queueTasks;
	
	
	/**
	 * Constructs a DiameterWorker
	 * 
	 * @param id 		An id number of the DiameterWorker.
	 * @param tasks		Task queue for saving all DiameterTasks.
	 */
	public DiameterWorker(int id, ArrayBlockingQueue<DiameterTask> tasks) {
		this.id = id;
		this.running = true;
		this.queueTasks = tasks;
		this.start();
	}



	/**
	 * Delivers a DiameterTask to a listener.
	 * 
	 * @see java.lang.Thread#run()
	 */
	public void run() {
		DiameterTask task;
		int i, size;
		EventListener e;
		Vector listeners;
		
		while(running){
			task=null;
			try {
				task = (DiameterTask) queueTasks.take();
			} catch (InterruptedException e1) {	}
		
			if (task==null||task.peer==null||task.msg==null) {
				//LOGGER.debug("["+id+"]ety "+queueTasks.size());
				continue;
			}
			//LOGGER.debug("["+id+"]got "+queueTasks.size());
			//LOGGER.debug("processing");
			listeners = task.peer.diameterPeer.eventListeners;
//			if (listeners.size()>0) ((EventListener) listeners.get(0)).recvMessage(task.peer.FQDN,task.msg);
			size = listeners.size();
			for(i=0;i<size;i++){
				e = (EventListener) listeners.get(i); 
				e.recvMessage(task.peer.FQDN,task.msg);
			}
		}
		//LOGGER.debug("["+id+"]end "+queueTasks.size());
	}

	
	/**
	 * Shuts downs the Diameter worker. 
	 */
	public void shutdown()
	{
		this.running = false;
		
		// poison the queue (now it just wakes the worker)
		try {
			this.queueTasks.put(new DiameterTask(null,null));
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
}
