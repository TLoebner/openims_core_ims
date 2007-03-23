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


import java.io.IOException;
import java.util.concurrent.LinkedBlockingQueue;

import org.apache.log4j.Logger;
import org.hibernate.Session;

import de.fhg.fokus.hss.diam.*;
import de.fhg.fokus.hss.db.hibernate.*;
/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class HSSContainer {
	private static Logger logger = Logger.getLogger(HSSContainer.class);
	private static HSSContainer appContainer = null;
	public LinkedBlockingQueue tasksQueue;
	public Worker [] workers;
	
	// diameter stack
	public DiameterStack diamStack;
	public TomcatServer tomcatServer;
	
	//....
	public static HSSContainer getInstance(){
		if (appContainer == null){
			appContainer = new HSSContainer();
		}
		
		return appContainer;
	}
	
	private HSSContainer(){
		diamStack = new DiameterStack();
		
		try{
			tomcatServer = new TomcatServer();
			tomcatServer.setPath("./");
			tomcatServer.startTomcat();
        }
		catch(Exception e){
			System.out.println("Exception occured during starting Tomcat!\nExiting from HSS...");
			e.printStackTrace();
			System.exit(-1);
		}
		
        Session session = HibernateUtil.getCurrentSession();
        //IMPI_DAO.getAll(session, 10, 10);
        HibernateUtil.commitTransaction();
        HibernateUtil.closeSession();
		
		tasksQueue = new LinkedBlockingQueue();
		workers = new Worker[10];
		for (int i = 0; i < workers.length; i++){
			workers[i] = new Worker(this);
			workers[i].start();
		}
	}
	
	public static void main(String args[]){
		HSSContainer hssContainer = HSSContainer.getInstance();
		waitForExit();
		try{
			hssContainer.tomcatServer.stopTomcat();
        }
		catch(Exception e){
			System.out.println("Exception occured during stoping Tomcat!\nExiting from HSS...");
			e.printStackTrace();
			System.exit(-1);
		}
		
        hssContainer.diamStack.diameterPeer.shutdown();
        System.exit(1);
	}
	
	/**
	 * This method waits until exit is typed in the console
	 * If wait is typed, then it returns.
	 */
	private static void waitForExit() {
		byte[] buffer = new byte[80]; 
		int read;
		String input="";
		while (true){
		    try{
		        logger.info("Type \"exit\" if you want to exit.");			       
		        read = System.in.read(buffer, 0, 80);			        
		        input = new String(buffer, 0, read);
		        input  = input.trim();
		    }
		    catch (IOException e){
		        e.printStackTrace();
		    }
			   
		    if(input.equalsIgnoreCase("exit")){
		    	return;
		    }
		}
	}
}
