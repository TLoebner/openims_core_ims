package de.fhg.fokus.hss.db.op;
import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMSU;


/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class IMSU_DAO {

	public static IMSU update(Session session, int id, String scscf_name, String diameter_name){
		
		IMSU imsu = (IMSU) session.load(IMSU.class, id);
		imsu.setScscf_name(scscf_name);
		imsu.setDiameter_name(diameter_name);
		session.saveOrUpdate(imsu);
		
		return imsu;
	}

	public static IMSU getByID(Session session, String id){
		return (IMSU) session.load(IMSU.class, Integer.parseInt(id));
	}
	public static IMSU getByID(Session session, int id){
		return (IMSU) session.load(IMSU.class, id);
	}
	
	public static IMSU getByName(Session session, String name){
		Query query;
		query = session.createQuery("from IMSU where name like ?");
		query.setString(0, name);
		return (IMSU) query.uniqueResult();
	}
	
	public static Object[] getByWildcardedName(Session session, String name, int firstResult, int maxResults){
		Query query;
		query = session.createQuery("from IMSU where name like ?");
		query.setString(0, "%" + name + "%");

		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static Object[] getAll(Session session, int firstResult, int maxResults){
		Query query;
		query = session.createQuery("from IMSU");
		
		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static List getAll(Session session){
		return session.createCriteria(IMSU.class).list();		
	}
	
	public static int deleteByID(Session session, int id){
		Query query = session.createQuery("delete from IMSU where id=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}
	
}
