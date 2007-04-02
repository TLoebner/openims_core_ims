/**
 * 
 */
package de.fhg.fokus.hss.main;


import java.util.List;
import java.util.Set;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.auth.HexCodec;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPI_IMPU;
import de.fhg.fokus.hss.db.model.IMSU;
import de.fhg.fokus.hss.db.op.CapabilitiesSet_DAO;
import de.fhg.fokus.hss.db.op.DB_Op;
import de.fhg.fokus.hss.db.op.IMPI_DAO;
import de.fhg.fokus.hss.db.op.IMPI_IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMSU_DAO;
import de.fhg.fokus.hss.db.hibernate.*;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class Tester extends Thread{
	public void run() {
		try {
			Thread.sleep(1000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		System.out.println("Tester class!");
		
		long time1 = System.currentTimeMillis();
		Session session = HibernateUtil.getCurrentSession();
		HibernateUtil.getCurrentSession().beginTransaction();
		
		long t01= System.currentTimeMillis();
		//IMPI_IMPU_DAO.get_IMPI(session, 2);
		long t02= System.currentTimeMillis();
		System.out.println("Delta select t0: " + (t02 - t01));
		
		IMPI impi = new IMPI();
		impi.setIdentity("sasas");
		impi.setK("saasa");
		impi.setAuth_scheme(1);
		impi.setIp("");
		impi.setAmf(HexCodec.decode("0000"));
		impi.setOp(HexCodec.decode("0000000000"));
		impi.setSqn(new String(HexCodec.decode("1111")));
		impi.setId_imsu(-1);
		//impi.setDefault_auth_scheme()
		impi.setLine_identifier("erere");
		
		IMPI_DAO.insert(session, impi);
		impi.setIdentity("id2");
		IMPI_DAO.update(session, impi);
		
		IMPI impi2 = IMPI_DAO.get_by_ID(session, impi.getId());
		System.out.println("\n\nIMPI Identitu:" + impi2.getIdentity());
		
		int xmed = 0;
		int cnt = 0;
		for (int i = 0; i < 1000; i++){		
			long t1;
			t1= System.currentTimeMillis();
			//IMPI impi = IMPI_DAO.getByID(session, 2);
			
			// List list = IMPI_IMPU_DAO.get_all_IMPU_by_IMPI(session, 2);
			//DB_Op.setUserState(session, 2, 1, (short)0, true);
			//IMPI_IMPU_DAO.get_all_Default_IMPU(session, 2);
			//IMPI_IMPU_DAO.get_all_Default_IMPU_of_Set_by_IMPI(session, 2);
			long t2;
			t2 = System.currentTimeMillis();
			System.out.println("Delta select one: " + (t2 - t1));
			xmed += t2 - t1;
		}
		HibernateUtil.commitTransaction();
		HibernateUtil.closeSession();
		
		System.out.println("XMED:" + xmed / 1000);
		
		long time3 = System.currentTimeMillis();
		System.out.println("Delta last after first selection process: " + (time3 - time1));
		System.out.println("Sleep 15 secs");
		try {
			Thread.sleep(15000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		System.out.println("Last measurement:");
		long t4, t5;
		t4 = System.currentTimeMillis();
		session = HibernateUtil.getCurrentSession();
		HibernateUtil.beginTransaction();
		//List list = IMPI_IMPU_DAO.get_all_IMPU_by_IMPI(session, 2);
		HibernateUtil.commitTransaction();
		session.close();
		t5 = System.currentTimeMillis();
		System.out.println("Time difference is:" + (t5 - t4));
		
		xmed = 0;
		session = HibernateUtil.getCurrentSession();
		HibernateUtil.beginTransaction();		
		for (int i = 0; i < 1000; i++){		
			long t1;
			t1= System.currentTimeMillis();
			//IMPI impi = IMPI_DAO.getByID(session, 2);
			//list = IMPI_IMPU_DAO.get_all_IMPU_by_IMPI(session, 2);
			//IMSU imsu = IMSU_DAO.getByID(session, 1);
			//DB_Op.setUserState(session, 2, 1, (short)0, true);
			//IMPI_IMPU_DAO.get_all_Default_IMPU(session, 2);
			//IMPI_IMPU_DAO.get_all_Default_IMPU_of_Set_by_IMPI(session, 2);
			long t2;
			t2 = System.currentTimeMillis();
			System.out.println("Delta select two: " + (t2 - t1));
			xmed += t2 - t1;
		}
		System.out.println("XMED:" + xmed / 1000);
		HibernateUtil.commitTransaction();
		HibernateUtil.closeSession();
		
/*		
		ApplicationServer as = new ApplicationServer();
		as.setName("adrian");
		as.setDiameter_address("diameter");
		as.setDefault_handling(1);
		as.setServer_name("server-name");
		as.setService_info("info");
		as.setRep_data_size_limit(100);
		HibernateUtil.getCurrentSession().save(as);
		HibernateUtil.commitTransaction();
		long time2 = System.currentTimeMillis();
		long delta = time2 - time1;
		System.out.println("Time difference is:" + delta);
		time2 = System.currentTimeMillis();
		HibernateUtil.getCurrentSession().beginTransaction();
		HibernateUtil.getCurrentSession().delete(as);
		HibernateUtil.commitTransaction();
		long time3 = System.currentTimeMillis();
		System.out.println("Time difference2 is:" + (time3 - time2));
		time3 = System.currentTimeMillis();
		
		HibernateUtil.getCurrentSession().beginTransaction();
		as = new ApplicationServer();
		as.setName("adrian2");
		as.setDiameter_address("diameter");
		as.setDefault_handling(1);
		as.setServer_name("server-name");
		as.setService_info("info");
		as.setRep_data_size_limit(100);
		HibernateUtil.getCurrentSession().save(as);
		HibernateUtil.commitTransaction();
		long time4 = System.currentTimeMillis();
		System.out.println("Time difference is:" + (time4 - time3));
		
		SP sp = new SP();
		sp.setName("default_sp");
		
		ChargingInfo ci = new ChargingInfo();
		ci.setName("default_chinfo");
		ci.setPri_ccf("a");
		ci.setSec_ccf("b");
		ci.setPri_ecf("c");
		ci.setSec_ecf("d");*/
		
/*		HibernateUtil.beginTransaction();
		//HibernateUtil.getCurrentSession().saveOrUpdate(sp);
		//HibernateUtil.getCurrentSession().saveOrUpdate(ci);
/*		IMSU imsu = new IMSU();
		imsu.setDiameter_name("diameter_name");
		imsu.setName("name");
		HibernateUtil.getCurrentSession().save(imsu);
		HibernateUtil.commitTransaction();
		HibernateUtil.closeSession();
		ApplicationServerDAO.joinTest();*/

/*		HibernateUtil.beginTransaction();
		List result = IMPI_IMPU_DAO.getJoinResult(HibernateUtil.getCurrentSession(), 1);
		System.out.println("Result size is:" + result.size());
		
		Object[] all = (Object[])result.get(0);
		IMPI_IMPU row = (IMPI_IMPU)all[0];
		System.out.println("IMPI:" + row.getImpi().getIdentity() + " IMPU:" + row.getImpu().getIdentity() + " user state:" + row.getUser_state());
		HibernateUtil.commitTransaction();
		HibernateUtil.closeSession();*/
	}
}
