package de.fhg.fokus.hss.db.op;

import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.ChargingInfo;
import de.fhg.fokus.hss.db.model.IMPU;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class IMPU_DAO {
	
	public static IMPU update(Session session, int id, short user_state){
		IMPU impu = (IMPU) session.load(IMPU.class, id);
		if (impu != null){
			impu.setUser_state(user_state);
		}
		return impu;
	}
	
	public static IMPU getByID(Session session, String id){
		return (IMPU) session.load(IMPU.class, Integer.parseInt(id));
	}
	
	public static IMPU getByIdentity(Session session, String identity){
		Query query;
		query = session.createQuery("from IMPU where identity like ?");
		query.setString(0, identity);
		return (IMPU) query.uniqueResult();
	}
	
	public static Object[] getByWildcardedIdentity(Session session, String identity, int firstResult, int maxResults){
		Query query;
		query = session.createQuery("from IMPU where identity like ?");
		query.setString(0, "%" + identity + "%");

		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static Object[] getAll(Session session, int firstResult, int maxResults){
		Query query;
		query = session.createQuery("from IMPU");
		
		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static int deleteByID(Session session, int id){
		Query query = session.createQuery("delete from IMPU where id=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}
	
	public static List getAllFromSet(Session session, int id_set){
		Query query;
		query = session.createQuery("from IMPU where id_set=?");
		query.setInteger(0, id_set);
		return query.list();
	}
	
	public static List getOthersFromSet(Session session, int id, int id_set){
		Query query;
		query = session.createQuery("from IMPU where id!=? and id_set=?");
		query.setInteger(0, id);
		query.setInteger(1, id_set);
		return query.list();
	}
	
	
}
