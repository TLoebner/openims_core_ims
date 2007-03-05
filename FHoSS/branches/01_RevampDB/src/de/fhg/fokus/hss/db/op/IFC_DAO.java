/**
 * 
 */
package de.fhg.fokus.hss.db.op;

import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.IFC;
import de.fhg.fokus.hss.util.HibernateUtil;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class IFC_DAO {
	public static IFC getByID(Session session, int id){
		return (IFC) session.load(IFC.class, id);
	}

	public static IFC getByName(Session session, String name){
		Query query = session.createQuery("from IFC where name like ?");
		query.setString(0, name);
		return (IFC) query.uniqueResult();
	}

	public static List getAll(Session session){
		Query query = session.createQuery("from IFC");
		return query.list();
	}
	
	public static int deleteByID(Session session, int id){
		Query query = session.createQuery("delete from IFC where id=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}
}
