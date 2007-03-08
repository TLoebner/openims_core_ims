/**
 * 
 */
package de.fhg.fokus.hss.db.op;

import java.util.Iterator;
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

public class DB_Op {
	public static void setUserState(Session session, int id_impi, int id_impu_implicitset, short user_state,
			boolean apply_on_IMPU){

		Query query = session.createQuery(
				"from IMPI_IMPU as impi_impu " +
				"	inner join impi_impu.impu" +
				"		where impi_impu.impu.id_impu_implicitset=? and impi_impu.impi.id=?"
				);
		query.setInteger(0, id_impu_implicitset);
		query.setInteger(1, id_impi);
		
		// update the user state on impi_impu and on impu table
		List resultList = query.list();
		Iterator it = resultList.iterator();
		IMPU impu = null; 
		while (it.hasNext()){
			Object[] resultRow = (Object[])it.next();
			IMPI_IMPU impi_impu = (IMPI_IMPU) resultRow[0];
			
			impi_impu.setUser_state(user_state);
			impu = (IMPU) resultRow[1];
			if (apply_on_IMPU){
				impu.setUser_state(user_state);
			}
			session.saveOrUpdate(impi_impu);
			session.saveOrUpdate(impu);
		}
	}
}
