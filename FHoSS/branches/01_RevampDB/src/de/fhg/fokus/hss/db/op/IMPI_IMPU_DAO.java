/**
 * 
 */
package de.fhg.fokus.hss.db.op;

import java.util.ArrayList;
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
	
	public static List getJoinByIMPI(Session session, int id_impi){
		Query query;
		query = session.createQuery(
				"from IMPI_IMPU as impi_impu" +
				"	inner join impi_impu.impi" +
				"	inner join impi_impu.impu" +
				"		where impi_impu.impi.id=?");
		query.setInteger(0, id_impi);
		return query.list();
	}

	public static List getJoinByIMPU(Session session, int id_impu){
		Query query;
		query = session.createQuery(
				"from IMPI_IMPU as impi_impu" +
				"	inner join impi_impu.impi" +
				"	inner join impi_impu.impu" +
				"		where impi_impu.impu.id=?");
		query.setInteger(0, id_impu);
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

	public static List<IMPI> get_all_IMPI_by_IMPU(Session session, int id_impu){
		Query query;
		query = session.createQuery(
				"from IMPI_IMPU as impi_impu" +
				"	inner join impi_impu.impi" +
				"	inner join impi_impu.impu" +
				"		where impi_impu.impu.id=?");
		query.setInteger(0, id_impu);
		List resultList = query.list();
		
		if (resultList == null || resultList.size() == 0){
			return null;
		}
		
		List<IMPI> impiList = new ArrayList<IMPI>();
		Iterator it = resultList.iterator();
		while (it.hasNext()){
			Object[] resultRow = (Object []) it.next();
			IMPI impi = (IMPI) resultRow[1];
			impiList.add(impi);
		}
		
		return impiList;
	}	

	public static List<IMPU> get_all_IMPU_by_IMPI(Session session, int id_impi){
		Query query;
		query = session.createQuery(
				"from IMPI_IMPU as impi_impu" +
				"	inner join impi_impu.impi" +
				"	inner join impi_impu.impu" +
				"		where impi_impu.impi.id=?");
		query.setInteger(0, id_impi);
		List resultList = query.list();
		if (resultList == null || resultList.size() == 0){
			return null;
		}

		List<IMPU> impuList = new ArrayList<IMPU>();
		Iterator it = resultList.iterator();
		while (it.hasNext()){
			Object[] resultRow = (Object []) it.next();
			IMPU impu = (IMPU) resultRow[2];
			impuList.add(impu);
		}
		
		return impuList;
	}
	
	public static List<IMPU> get_all_Default_IMPU_of_Set_by_IMPI(Session session, int id_impi){
		Query query;
		query = session.createQuery(
				"from IMPI_IMPU as impi_impu" +
				"	inner join impi_impu.impi" +
				"	inner join impi_impu.impu" +
				"		where impi_impu.impi.id=? order by impi_impu.impu.id_impu_implicitset");
		query.setInteger(0, id_impi);
		List resultList = query.list();
		if (resultList == null || resultList.size() == 0){
			return null;
		}

		List<IMPU> impuList = new ArrayList<IMPU>();
		Iterator it = resultList.iterator();
		int previousSet = -1;
		int currentSet = -1;
		
		while (it.hasNext()){
			Object[] resultRow = (Object []) it.next();
			IMPU impu = (IMPU) resultRow[2];
			currentSet = impu.getId_impu_implicitset();
			if (currentSet != previousSet){
				impuList.add(impu);
				previousSet = currentSet;
			}
		}
		
		return impuList;
	} 
	
	public static int get_IMPU_Registered_cnt(Session session, int id_impu){
		Query query;
		query = session.createQuery(
				"select count(*) from IMPI_IMPU as impi_impu" +
				"	inner join impi_impu.impu" +
				"		where impi_impu.user_state=1 and impi_impu.impu.id=?");
		query.setInteger(0, id_impu);
		Integer result = (Integer)query.uniqueResult();
		return result.intValue();
	}
	
}
