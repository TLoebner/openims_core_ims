/**
 * 
 */
package de.fhg.fokus.hss.db.op;

import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.ChargingInfo;
import de.fhg.fokus.hss.util.HibernateUtil;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class ChargingInfoDAO {
	public static ChargingInfo getByID(Session session, int id){
		return (ChargingInfo) session.load(ChargingInfo.class, id);
	}

	public static ChargingInfo getByName(Session session, String name){
		Query query = session.createQuery("from ChargingInfo where name like ?");
		query.setString(0, name);
		return (ChargingInfo) query.uniqueResult();
	}

	public static List getAll(Session session){
		Query query = session.createQuery("from ChargingInfo");
		return query.list();
	}
	
	public static int deleteByID(Session session, int id){
		Query query = session.createQuery("delete from ChargingInfo where id=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}
}
