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
public class SP_Shared_IFC_Set {
	
	public static SP_Shared_IFC_Set getBySP_Shared_IFC_Set(Session session, int id_sp, int id_shared_ifc_set){
		Query query;
		query = session.createQuery("from SP_Shared_IFC_set where id_sp=? and id_shared_ifc_set=?");
		query.setInteger(0, id_sp);
		query.setInteger(1, id_shared_ifc_set);
		return (SP_Shared_IFC_Set) query.uniqueResult();
	}
	
	public static List getJoinResult(Session session, int id_sp){
		Query query;
		query = session.createQuery(
				"from SP_Shared_IFC_Set as sp_shared_ifc_set" +
				"	inner join sp_shared_ifc_set.sp" +
				"	inner join sp_shared_ifc_set.shared_ifc_set" +
				"		where sp_shared_ifc_set.sp.id=?");
		query.setInteger(0, id_sp);
		return query.list();
	}
}
