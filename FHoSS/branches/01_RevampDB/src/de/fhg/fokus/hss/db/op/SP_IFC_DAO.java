/**
 * 
 */
package de.fhg.fokus.hss.db.op;

import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.SP_IFC;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class SP_IFC_DAO {
	
	public static SP_IFC getBySP_IFC(Session session, int id_sp, int id_ifc){
		Query query;
		query = session.createQuery("from SP_IFC where id_sp=? and id_ifc=?");
		query.setInteger(0, id_sp);
		query.setInteger(1, id_ifc);
		return (SP_IFC) query.uniqueResult();
	}
	
	public static List getJoinResult(Session session, int id_sp){
		Query query;
		query = session.createQuery(
				"from SP_IFC as sp_ifc" +
				"	inner join sp_ifc.sp" +
				"	inner join sp_ifc.ifc" +
				"		where sp_ifc.sp.id=?");
		query.setInteger(0, id_sp);
		return query.list();
	}

}
