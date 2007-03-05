package de.fhg.fokus.hss.db.op;

import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.VisitedNetwork;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class VisitedNetworkDAO {
	public static VisitedNetwork getByID(Session session, String id){
		return (VisitedNetwork) session.load(VisitedNetwork.class, Integer.parseInt(id));
	}
	
	public static VisitedNetwork getByIdentity(Session session, String identity){
		Query query;
		query = session.createQuery("from VisitedNetwork where identity like ?");
		query.setString(0, identity);
		return (VisitedNetwork) query.uniqueResult();
	}
}
