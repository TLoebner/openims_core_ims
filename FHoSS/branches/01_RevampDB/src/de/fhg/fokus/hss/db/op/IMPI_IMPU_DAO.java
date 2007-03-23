/*
  *  Copyright (C) 2004-2007 FhG Fokus
  *
  * This file is part of Open IMS Core - an open source IMS CSCFs & HSS
  * implementation
  *
  * Open IMS Core is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License, or
  * (at your option) any later version.
  *
  * For a license to use the Open IMS Core software under conditions
  * other than those described here, or to purchase support for this
  * software, please contact Fraunhofer FOKUS by e-mail at the following
  * addresses:
  *     info@open-ims.org
  *
  * Open IMS Core is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * It has to be noted that this Open Source IMS Core System is not
  * intended to become or act as a product in a commercial context! Its
  * sole purpose is to provide an IMS core reference implementation for
  * IMS technology testing and IMS application prototyping for research
  * purposes, typically performed in IMS test-beds.
  *
  * Users of the Open Source IMS Core System have to be aware that IMS
  * technology may be subject of patents and licence terms, as being
  * specified within the various IMS-related IETF, ITU-T, ETSI, and 3GPP
  * standards. Thus all Open IMS Core users have to take notice of this
  * fact and have to agree to check out carefully before installing,
  * using and extending the Open Source IMS Core System, if related
  * patents and licenses may become applicable to the intended usage
  * context. 
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  
  * 
  */

package de.fhg.fokus.hss.db.op;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import org.hibernate.Hibernate;
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

	public static void insert(Session session, int id_impi, int id_impu, short user_state){
		IMPI_IMPU impi_impu = new IMPI_IMPU();
		impi_impu.setId_impi(id_impi);
		impi_impu.setId_impu(id_impu);
		impi_impu.setUser_state(user_state);
		session.save(impi_impu);
	}
	
	public static IMPI_IMPU get_by_IMPI_and_IMPU_ID(Session session, int id_impi, int id_impu){
		Query query;
		query = session.createSQLQuery("select * from impi_impu where id_impi=? and id_impu=?")
			.addEntity(IMPI_IMPU.class);
		query.setInteger(0, id_impi);
		query.setInteger(1, id_impu);
		return (IMPI_IMPU) query.uniqueResult();
	}

	public static List get_all_IMPU_by_IMPI_ID(Session session, int id_impi){
		Query query;

		query = session.createSQLQuery("select * from impi_impu" +
				"	inner join impu on impi_impu.id_impu=impu.id" +
				" where impi_impu.id_impi=?")
					.addEntity(IMPU.class);
		query.setInteger(0, id_impi);
		return query.list();
		
/*		Iterator it = queryResult.iterator();
		List<IMPU> result = new LinkedList<IMPU>(); 
		while (it.hasNext()){
			IMPI_IMPU row = (IMPI_IMPU)it.next();
			result.add(row.getImpu());
		}
		
		return result;
*/		
	}
	
/*	public static List getJoinByIMPI(Session session, int id_impi){
		Query query;
		query = session.createQuery(
				"from IMPI_IMPU as impi_impu" +
				"	inner join impi_impu.impi" +
				"	inner join impi_impu.impu" +
				"		where impi_impu.impi.id=?");
		query.setInteger(0, id_impi);
		return query.list();
	}


	*/

	public static List get_join_by_IMPU_ID(Session session, int id_impu){
		Query query;
		
		query = session.createSQLQuery(
					"select * from impi_impu " + 
					"	inner join impi on impi_impu.id_impi = impi.id  where impi_impu.id_impu=? ")
						.addEntity(IMPI_IMPU.class)
						.addEntity(IMPI.class);
		query.setInteger(0, id_impu);
		return query.list();
	} 
	
	public static int delete_by_IMPI_ID(Session session, int id_impi){
		Query query;
		query = session.createSQLQuery("delete from impi_impu where id_impi=?");
		query.setInteger(0, id_impi);
		return query.executeUpdate();
	}

	public static int delete_by_IMPI_and_IMPU_ID(Session session, int id_impi, int id_impu){
		Query query;
		query = session.createSQLQuery("delete from impi_impu where id_impi=? and id_impu=?");
		query.setInteger(0, id_impi);
		query.setInteger(1, id_impu);
		return query.executeUpdate();
	}
	

	public static int delete_by_IMPU_ID(Session session, int id_impu){
		Query query;
		query = session.createSQLQuery("delete from impi_impu where id_impu=?");
		query.setInteger(0, id_impu);
		return query.executeUpdate();
	}
	
	public static List get_all_IMPU_of_IMSU_with_User_State(Session session, int id_imsu, short user_state){
		Query query;
		query = session.createSQLQuery(
				"select * from impi_impu" +
				"	inner join impi on impi_impu.id_impi=impi.id" +
				"	inner join impu on impi_impu.id_impu=impu.id" +
				"		where impi.id_imsu=? and impu.user_state=?")
					.addEntity(IMPU.class);
		query.setInteger(0, id_imsu);
		query.setShort(1, user_state);
		return query.list();
	}	

	public static List get_all_IMPI_by_IMPU(Session session, int id_impu){
		Query query;
		query = session.createSQLQuery(
				"select * from impi_impu" +
				"	inner join impi on impi_impu.id_impi=impi.id" +
				"		where impi_impu.id_impu=?")
					.addEntity(IMPI.class);
		query.setInteger(0, id_impu);
		return query.list();
		
		/*
		if (resultList == null || resultList.size() == 0){
			return null;
		}
		
		List<IMPI> impiList = new ArrayList<IMPI>();
		Iterator it = resultList.iterator();
		while (it.hasNext()){
			IMPI impi = (IMPI) it.next();
			impiList.add(impi);
		}
		
		return impiList;*/
	}	

/*	public static List<IMPU> get_all_IMPU_by_IMPI(Session session, int id_impi){
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
	}*/
	
	public static List<IMPU> get_all_Default_IMPU_of_Set_by_IMPI(Session session, int id_impi){
		Query query;
		query = session.createSQLQuery(
				"select * from impi_impu" +
				"	inner join impi on impi_impu.id_impi=impi.id" +
				"	inner join impu on impi_impu.id_impu=impu.id" +
				"		where impi_impu.id_impi=? order by impu.id_impu_implicitset")
					.addEntity(IMPU.class);
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
			
			IMPU impu = (IMPU) it.next();
			currentSet = impu.getId_impu_implicitset();
			if (currentSet != previousSet){
				impuList.add(impu);
				previousSet = currentSet;
			}
		}
		
		return impuList;
	} 
	
/*	public static List get_all_Default_IMPU(Session session, int id_impi){
		Query query;
		query = session.createSQLQuery("select * from impi_impu inner join impi on impi.id=impi_impu.id_impi" +
				" inner join impu on impu.id=impi_impu.id where impi_impu.id_impi=?")
				.addScalar("id_impi", Hibernate.INTEGER)
				.addScalar("id_impu", Hibernate.INTEGER)
				.addScalar("impi.identity", Hibernate.STRING)
				.addScalar("impu.identity", Hibernate.STRING)
				.setInteger(0, id_impi);
		return query.list();
	}*/

	public static int get_Registered_IMPU_Count(Session session, int id_impu){
		Query query;
		query = session.createSQLQuery(
				"select count(*) from impi_impu" +
				"	inner join impu on impu.id=impi_impu.id_impu" +
				"		where impi_impu.user_state=1 and impi_impu.id_impu=?");
		query.setInteger(0, id_impu);
		BigInteger result = (BigInteger)query.uniqueResult();
		return result.intValue();
	}
	
}
