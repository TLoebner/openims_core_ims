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

package de.fhg.fokus.hss.db.op;

import java.util.List;

import org.apache.log4j.Logger;
import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.ShNotification;
import de.fhg.fokus.hss.db.model.ShSubscription;
import de.fhg.fokus.hss.sh.ShConstants;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class ShNotification_DAO {
	private static Logger logger = Logger.getLogger(ShNotification_DAO.class);
	
	public static void insert(Session session, ShNotification sh_notification){
		session.save(sh_notification);
	}

	public static void update(Session session, ShNotification sh_notification){
		session.save(sh_notification);
	}
	
	public static ShNotification get_by_ID(Session session, int id){
		Query query;
		query = session.createSQLQuery("select * from sh_notification where id=?")
			.addEntity(ShNotification.class);
		query.setInteger(0, id);

		return (ShNotification) query.uniqueResult();
	}	

	public static ShNotification get_next_available(Session session){
		Query query;
		query = session.createSQLQuery("select * from sh_notification where hopbyhop=0 limit 1")
				.addEntity(ShNotification.class);
		return (ShNotification) query.uniqueResult();
	}

	public static void mark_all_from_grp(Session session, int grp){
		Query query;
		query = session.createSQLQuery("update sh_notification set hopbyhop=1 where grp=?");
		query.setInteger(0, grp);
		query.executeUpdate();
	}
	

	public static int get_max_grp(Session session){
		Query query = session.createSQLQuery("select max(grp) from sh_notification");
		Integer result = (Integer) query.uniqueResult();
		if (result == null)
			return 0;
		return result.intValue();
	}
	
	public static ShNotification get_one_from_grp (Session session, int grp){
		Query query;
		query = session.createSQLQuery("select * from sh_notification where grp=? limit 1")
			.addEntity(ShNotification.class);
		query.setLong(0, grp);
		return (ShNotification) query.uniqueResult();
	}

	public static List get_all_from_grp(Session session, int grp){
		Query query;
		query = session.createSQLQuery("select * from sh_notification where grp=?")
				.addEntity(ShNotification.class);
		query.setInteger(0, grp);
		return query.list();
	}
	
	
	public static void update_by_grp(Session session, int grp, long hopByHopID, long endToEndID){
		Query query;
		query = session.createSQLQuery("update sh_notification set hopbyhop=?, endtoend=? where grp=?")
				.setLong(0, hopByHopID)
				.setLong(1, endToEndID)
				.setInteger(2, grp);
		query.executeUpdate();
	}
	
	public static int delete_by_ID(Session session, int id){
		Query query = session.createSQLQuery("delete from sh_notification where id=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}

	public static void delete(Session session, long hopbyhop, long endtoend){
		Query query = session.createSQLQuery("delete from sh_notification where hopbyhop=? and endtoend=?");
		query.setLong(0, hopbyhop);
		query.setLong(1, endtoend);
		query.executeUpdate();
	}
	
	
	public static void insert_notif_for_IMS_User_State(Session session, int id_set, int imsUserState){
		// send Sh notification to all subscribers
		
		List impuList = IMPU_DAO.get_all_from_set(session, id_set);
		
		if (impuList != null){
			for (int i = 0; i < impuList.size(); i++){
				IMPU crtIMPU = (IMPU) impuList.get(i);
				List shSubscriptionList = ShSubscription_DAO.get_all_by_IMPU_and_DataRef(session, crtIMPU.getId(), 
						ShConstants.Data_Ref_IMS_User_State);
				if (shSubscriptionList != null){
					for (int j = 0; j < shSubscriptionList.size(); j++){
						ShSubscription shSubscription = (ShSubscription) shSubscriptionList.get(j);
						ShNotification shNotification = new ShNotification();
						shNotification.setData_ref(ShConstants.Data_Ref_IMS_User_State);
						shNotification.setGrp(ShNotification_DAO.get_max_grp(session) + 1);
						shNotification.setId_application_server(shSubscription.getId_application_server());
						shNotification.setId_impu(crtIMPU.getId());
						shNotification.setReg_state(imsUserState);
						ShNotification_DAO.insert(session, shNotification);
					}
				}
				
			}
		}
	}
	
	public static void insert_notif_for_SCSCFName(Session session, int id_imsu, String scscfName){
		
		List impuIDList = IMPU_DAO.get_all_IMPU_ID_for_IMSU(session, id_imsu);
		
		if (impuIDList != null){
			for (int i = 0; i < impuIDList.size(); i++){
				Integer id_impu = (Integer) impuIDList.get(i);
		
				// send Sh notification to all subscribers
				List shSubscriptionList = ShSubscription_DAO.get_all_by_IMPU_and_DataRef(session, id_impu, 
						ShConstants.Data_Ref_SCSCF_Name);
				if (shSubscriptionList != null){
					for (int j = 0; j < shSubscriptionList.size(); j++){
						ShSubscription shSubscription = (ShSubscription) shSubscriptionList.get(j);
						ShNotification shNotification = new ShNotification();
						shNotification.setData_ref(ShConstants.Data_Ref_SCSCF_Name);
						shNotification.setGrp(ShNotification_DAO.get_max_grp(session) + 1);
						shNotification.setId_application_server(shSubscription.getId_application_server());
						shNotification.setId_impu(id_impu);
						shNotification.setScscf_name(scscfName);
						ShNotification_DAO.insert(session, shNotification);
					}
				}
			}
		}	
	}
}
