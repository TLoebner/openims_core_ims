/**
 * 
 */
package de.fhg.fokus.hss.db.op;

import java.util.Iterator;
import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.ApplicationServer;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.util.HibernateUtil;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class ApplicationServerDAO {
	
	public static ApplicationServer getByID(Session session, int id){
		return (ApplicationServer) session.load(ApplicationServer.class, id);
	}

	public static List getAll(Session session){
		Query query = session.createQuery("from ApplicationServer");
		return query.list();
	}
	
	public static int deleteByID(Session session, int ID){
		Query query = session.createQuery("delete from ApplicationServer as app_server where app_server.id=");
		query.setInteger(0, ID);
		return query.executeUpdate();
	}
	
	public static List joinTest(){
		HibernateUtil.beginTransaction();
		Session session = HibernateUtil.getCurrentSession();
		Query query = session.createQuery("from IMPU, IMPI");
		List l = query.list();
		Iterator pairsIt = l.iterator();
		System.out.println("\n\nBegin the test!!!");
		
		while (pairsIt.hasNext()){
			Object [] result = (Object[])pairsIt.next();
			IMPU impu = (IMPU) result[0];
			IMPI impi = (IMPI) result[1];
			System.out.println("IMPU is: " + impu.getIdentity() + "IMPI is: " + impi.getIdentity());
		}
		HibernateUtil.commitTransaction();		
		session.close();
		System.out.println("Test ended!!!\n\n");
		
		return null;
	} 
}
