/**
 * 
 */
package de.fhg.fokus.hss.db.op;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.TP;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class TP_DAO {
	
	public static TP getByID(Session session, String id){
		return (TP) session.load(TP.class, Integer.parseInt(id));
	}
	
	public static TP getByName(Session session, String name){
		Query query;
		query = session.createQuery("from TP where name like ?");
		query.setString(0, name);
		return (TP) query.uniqueResult();
	}
}
