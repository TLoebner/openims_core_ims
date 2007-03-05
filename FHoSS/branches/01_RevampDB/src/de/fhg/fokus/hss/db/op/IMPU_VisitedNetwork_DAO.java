/**
 * 
 */
package de.fhg.fokus.hss.db.op;

import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.IMPI_IMPU;
import de.fhg.fokus.hss.db.model.IMPU_VisitedNetwork;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class IMPU_VisitedNetwork_DAO {
	
	public static IMPU_VisitedNetwork getByIMPU_VisitedNetwork(Session session, int id_impu, int id_visited_network){
		Query query;
		query = session.createQuery("from IMPU_VisitedNetwork where id_impu=? and id_visited_network=?");
		query.setInteger(0, id_impu);
		query.setInteger(1, id_visited_network);
		return (IMPU_VisitedNetwork) query.uniqueResult();
	}
	
	public static List getJoinResult(Session session, int id_impu){
		Query query;
		query = session.createQuery(
				"from IMPU_VisitedNetwork as impu_visited_network" +
				"	inner join impu_visited_network.impu" +
				"	inner join impu_visited_network.visited_network" +
				"		where impu_visited_network.impu.id=?");
		query.setInteger(0, id_impu);
		return query.list();
	}
}
