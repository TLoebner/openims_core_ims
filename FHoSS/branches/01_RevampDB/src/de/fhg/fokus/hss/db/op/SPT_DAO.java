/**
 * 
 */
package de.fhg.fokus.hss.db.op;

import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.SPT;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class SPT_DAO {
	public static SPT getByID(Session session, int id){
		return (SPT) session.load(SPT.class, id);
	}

	public static List getAllByTP(Session session, int id_tp){
		Query query = session.createQuery("from SPT where id_tp=?");
		query.setInteger(0, id_tp);
		return query.list();
	}
	
	public static int deleteByID(Session session, int id){
		Query query = session.createQuery("delete from SPT where id=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}
}
