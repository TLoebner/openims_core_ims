/**
 * 
 */
package de.fhg.fokus.hss.db.op;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPI_IMPU;
import de.fhg.fokus.hss.db.model.IMPU;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class IMPI_IMPU_DAO {

	public static void insert(Session session, IMPI impi, IMPU impu, short user_state){
		IMPI_IMPU impi_impu = new IMPI_IMPU();
		impi_impu.setImpi(impi);
		impi_impu.setImpu(impu);
		impi_impu.setUser_state(user_state);
		session.save(impi_impu);
	}
	
	public static IMPI_IMPU getByIMPI_IMPU(Session session, int id_impi, int id_impu){
		Query query;
		query = session.createQuery("from IMPI_IMPU where id_impi=? and id_impu=?");
		query.setInteger(0, id_impi);
		query.setInteger(1, id_impu);
		return (IMPI_IMPU) query.uniqueResult();
	}

	public static List<IMPU> getIMPU_By_IMPI(Session session, int id_impi){
		Query query;
		query = session.createQuery("from IMPI_IMPU where id_impi=?");
		query.setInteger(0, id_impi);
		List queryResult = query.list();
		Iterator it = queryResult.iterator();
		
		List<IMPU> result = new LinkedList<IMPU>(); 
		while (it.hasNext()){
			IMPI_IMPU row = (IMPI_IMPU)it.next();
			result.add(row.getImpu());
		}
		
		return result;
	}
	
	public static List getJoinResult(Session session, int id_impi){
		Query query;
		query = session.createQuery(
				"from IMPI_IMPU as impi_impu" +
				"	inner join impi_impu.impi" +
				"	inner join impi_impu.impu" +
				"		where impi_impu.impi.id=?");
		query.setInteger(0, id_impi);
		return query.list();
	}
	
	public static int deleteByID_IMPI(Session session, int id_impi){
		Query query;
		query = session.createQuery("delete from IMPI_IMPU where id_impi=?");
		query.setInteger(0, id_impi);
		return query.executeUpdate();
	}

	public static int deleteByIMPI_IMPU(Session session, int id_impi, int id_impu){
		Query query;
		query = session.createQuery("delete from IMPI_IMPU where id_impi=? and id_impu=?");
		query.setInteger(0, id_impi);
		query.setInteger(1, id_impu);
		return query.executeUpdate();
	}
	

	public static int deleteByID_IMPU(Session session, int id_impu){
		Query query;
		query = session.createQuery("delete from IMPI_IMPU where id_impu=?");
		query.setInteger(0, id_impu);
		return query.executeUpdate();
	}
	
	public static List get_All_IMPU_of_IMSU_user_state(Session session, int id_imsu, short user_state){
		Query query;
		query = session.createQuery(
				"from IMPI_IMPU as impi_impu" +
				"	inner join impi_impu.impi" +
				"	inner join impi_impu.impu" +
				"		where impi_impu.impi.imsu.id=? and impi_impu.impu.user_state=?");
		query.setInteger(0, id_imsu);
		query.setShort(1, user_state);
		return query.list();
	}	
}
