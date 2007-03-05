/**
 * 
 */
package de.fhg.fokus.hss.db.op;

import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class CapabilitiesSetDAO {
	
	public static List getAllFromSet(Session session, int id_set){
		Query query = session.createQuery("from CapabilitiesSet as cap_set where cap_set.id_set=?");
		query.setInteger(0, id_set);
		return query.list();
	}

	public static List getAllSets(Session session){
		Query query = session.createQuery("select distinct id_set, name from CapabilitiesSet");
		return query.list();
	}
	
	public static int deleteSetByID(Session session, int ID){
		Query query = session.createQuery("delete from CapabilitiesSet where id_set=?");
		query.setInteger(0, ID);
		return query.executeUpdate();
	}
	
	public static int deleteCapabilityFromSet(Session session, int id_set, int id_capability){
		Query query = session.createQuery("delete from CapabilitiesSet where id_set=? and id_capability=?");
		query.setInteger(0, id_set);
		query.setInteger(1, id_capability);
		return query.executeUpdate();
	}

}
