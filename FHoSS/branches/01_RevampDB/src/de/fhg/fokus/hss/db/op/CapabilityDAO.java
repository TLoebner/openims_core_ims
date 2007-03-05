/**
 * 
 */
package de.fhg.fokus.hss.db.op;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.Capability;
import de.fhg.fokus.hss.util.HibernateUtil;

public class CapabilityDAO {
	
	public static Capability getByID(Session session, int id){
		return (Capability) session.load(Capability.class, id);
	}

	public static Capability getByName(Session session, String name){
		Query query = session.createQuery("from Capability where name like ?");
		query.setString(0, name);
		return (Capability) query.uniqueResult();
	}

	public static List getAll(Session session){
		Query query = session.createQuery("from Capability");
		return query.list();
	}
	
	public static int deleteByID(Session session, int id){
		Query query = session.createQuery("delete from Capability where id=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}
}
