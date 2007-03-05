package de.fhg.fokus.hss.db.op;

import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class Shared_IFC_Set_DAO {
	public static List getAllFromSet(Session session, int id_set){
		Query query = session.createQuery("from Shared_IFC_Set where id_set=?");
		query.setInteger(0, id_set);
		return query.list();
	}

	public static List getAllSets(Session session){
		Query query = session.createQuery("select distinct id_set, name from Shared_IFC_Set");
		return query.list();
	}
	
	public static int deleteSetByID(Session session, int id){
		Query query = session.createQuery("delete from Shared_IFC_Set where id_set=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}
	
	public static int delete_IFC_FromSet(Session session, int id_set, int id_ifc){
		Query query = session.createQuery("delete from Shared_IFC_Set where id_set=? and id_ifc=?");
		query.setInteger(0, id_set);
		query.setInteger(1, id_ifc);
		return query.executeUpdate();
	}
}
