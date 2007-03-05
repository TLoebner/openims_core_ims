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
public class PreferredSetDAO {
	
	public static List getAllFromSet(Session session, int id_set){
		Query query = session.createQuery("from PreferredSet where id_set=?");
		query.setInteger(0, id_set);
		return query.list();
	}

	public static List getAllSets(Session session){
		Query query = session.createQuery("select distinct id_set, name from PreferredSet");
		return query.list();
	}
	
	public static int deleteSetByID(Session session, int id){
		Query query = session.createQuery("delete from PreferredSet where id_set=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}
	
	public static int delete_SCSCF_Name_FromSet(Session session, String scscf_name){
		Query query = session.createQuery("delete from PreferredSet where scscf_name like ?");
		query.setString(0, scscf_name);
		return query.executeUpdate();
	}
}
