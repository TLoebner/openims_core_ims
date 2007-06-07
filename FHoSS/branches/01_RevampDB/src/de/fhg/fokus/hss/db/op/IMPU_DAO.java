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

import java.util.Iterator;
import java.util.List;

import org.apache.log4j.Logger;
import org.hibernate.Hibernate;
import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.SP;
import de.fhg.fokus.hss.db.model.VisitedNetwork;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class IMPU_DAO {
	private static Logger logger = Logger.getLogger(IMPU_DAO.class);
	public static void insert(Session session, IMPU impu){
		session.save(impu);
	}
	
	public static void update(Session session, IMPU impu){
		session.saveOrUpdate(impu);
	}	
	
	public static IMPU update(Session session, int id, short user_state){
		IMPU impu = (IMPU) session.load(IMPU.class, id);
		if (impu != null){
			impu.setUser_state(user_state);
		}
		return impu;
	}

	public static void update_others_from_implicit_set_ID(Session session, int id_impu, int old_implicit_set_id){
		List l = IMPU_DAO.get_others_from_set(session, id_impu, old_implicit_set_id);
		if (l != null && l.size() > 0){
			Iterator it = l.iterator();
			IMPU crt_impu;
			int id_set = -1;
			while (it.hasNext()){
				crt_impu = (IMPU) it.next();
				if (id_set == -1){
					id_set = crt_impu.getId();
				}
				if (crt_impu.getId() != id_impu){
					crt_impu.setId_implicit_set(id_set);
					IMPU_DAO.update(session, crt_impu);
				}
			}
		}
		/*query = session.createSQLQuery("update impu set id_implicit_set=? where id_implicit_set=? and id !=?");
		query.setInteger(0, new_implicit_set_id);
		query.setInteger(1, old_implicit_set_id);
		query.setInteger(2, id_impu);
		query.executeUpdate();*/
	}
	
	public static List get_all_VisitedNetworks_by_IMPU_ID(Session session, int id_impu){
		Query query;
		query = session.createSQLQuery("select * from visited_network" +
				"	inner join impu_visited_network on impu_visited_network.id_visited_network=visited_network.id" +
				" where impu_visited_network.id_impu=?")
				.addEntity("visited_network", VisitedNetwork.class);
		query.setInteger(0, id_impu);
		return query.list();
	}
	
	public static List get_all_IMPU_for_VN_ID(Session session, int id_vn){
		Query query;
		query = session.createSQLQuery("select * from impu" +
				"	inner join impu_visited_network on impu_visited_network.id_impu=impu.id" +
				" where impu_visited_network.id_visited_network=?")
				.addEntity("impu", IMPU.class);
		query.setInteger(0, id_vn);
		return query.list();
	}

	public static List get_all_IMPI_for_IMPU_ID(Session session, int id_impu){
		Query query;
		query = session.createSQLQuery("select * from impi" +
				"	inner join impi_impu on impi.id=impi_impu.id_impi" +
				" where impi_impu.id_impu=?")
				.addEntity("impi", IMPI.class);
		query.setInteger(0, id_impu);
		return query.list();
	}

	public static int get_a_registered_IMPI_ID(Session session, int id_impu){
		Query query;
		query = session.createSQLQuery("select id_impi from impi_impu" +
				"	inner join impu on impu.id=impi_impu.id_impu" +
				" where (impi_impu.user_state=1 or impi_impu.user_state=2) and impi_impu.id_impu=? limit 1")
				.addScalar("id_impi", Hibernate.INTEGER);
		query.setInteger(0, id_impu);
		Integer result = (Integer) query.uniqueResult();
		if (result == null)
			return -1;
		return result;
	}
	
	public static int delete_VisitedNetwork_for_IMPU(Session session, int id_impu, int id_vn){
		Query query = session.createSQLQuery("delete from impu_visited_network where id_impu=? and id_visited_network=?");
		query.setInteger(0, id_impu);
		query.setInteger(1, id_vn);
		return query.executeUpdate();
	}
	
	public static IMPU get_by_ID(Session session, int id){
		Query query;
		query = session.createSQLQuery("select * from impu where id=?")
			.addEntity(IMPU.class);
		query.setInteger(0, id);

		return (IMPU) query.uniqueResult();
	}

	public static IMPU get_one_from_set(Session session, int id_implicit_set){
		Query query;
		query = session.createSQLQuery("select * from impu where id_implicit_set=? limit 1")
			.addEntity(IMPU.class);
		query.setInteger(0, id_implicit_set);

		return (IMPU) query.uniqueResult();
	}
	
	public static List get_aliases_IMPUs(Session session, int id_implicit_set, int id_sp){
		Query query;
		query = session.createSQLQuery("select * from impu where id_implicit_set=? and id_sp=?")
			.addEntity(IMPU.class);
		query.setInteger(0, id_implicit_set);
		query.setInteger(1, id_sp);

		return query.list();
	}
	
	public static IMPU get_by_Identity(Session session, String identity){
		Query query;
		query = session.createSQLQuery("select * from impu where identity=?")
			.addEntity(IMPU.class);
		query.setString(0, identity);
		
		IMPU result = null;
		try{
			result = (IMPU) query.uniqueResult();
		}
		catch(org.hibernate.NonUniqueResultException e){
			logger.error("Query did not returned an unique result! You have a duplicate in the database!");
			e.printStackTrace();
		}

		return result;
	}
	
	public static Object[] get_by_Wildcarded_Identity(Session session, String identity, int firstResult, int maxResults){
		Query query;
		query = session.createSQLQuery("select * from impu where identity like ?")
			.addEntity(IMPU.class);
		query.setString(0, "%" + identity + "%");

		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static Object[] get_all(Session session, int firstResult, int maxResults){
		Query query;
		query = session.createSQLQuery("select * from impu")
			.addEntity(IMPU.class);
		
		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static int delete_by_ID(Session session, int id){
		Query query = session.createSQLQuery("delete from impu where id=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}
	
	public static List get_all_from_set(Session session, int id_set){
		Query query;
		query = session.createSQLQuery("select * from impu where id_implicit_set=?")
			.addEntity(IMPU.class);
		query.setInteger(0, id_set);
		return query.list();
	}
	
	public static Object[] get_all_from_set(Session session, int id_set, int firstResult, int maxResults){
		Query query;
		query = session.createSQLQuery("select * from impu where id_implicit_set=?")
		.addEntity(IMPU.class);
		query.setInteger(0, id_set);

		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static List get_others_from_set(Session session, int id, int id_set){
		Query query;
		query = session.createSQLQuery("select * from impu where id != ? and id_implicit_set=?")
			.addEntity(IMPU.class);
		query.setInteger(0, id);
		query.setInteger(1, id_set);
		return query.list();
	}
	
	public static List get_all_sp_for_set(Session session, int id_implicit_set){
		Query query;
		query = session.createSQLQuery(
				"select {SP.*}, {IMPU.*} from sp SP" +
				"	inner join impu IMPU on IMPU.id_sp=SP.id" +
				"		where IMPU.id_implicit_set=? order by IMPU.id_sp")
					.addEntity(SP.class)
					.addEntity(IMPU.class);
		query.setInteger(0, id_implicit_set);
		return query.list();
	}
	
	
	public static List get_by_Charging_Info_ID(Session session, int id_charging_info){
		Query query;
		query = session.createSQLQuery("select * from impu where id_charging_info = ?")
			.addEntity(IMPU.class);
		query.setInteger(0, id_charging_info);
		return query.list();
	}

	public static List get_all_IMPU_ID_for_IMSU(Session session, int id_imsu){
		Query query;
		query = session.createSQLQuery("select distinct impu.id from impu" +
				"	inner join impi_impu on impu.id=impi_impu.id_impu" +
				"	inner join impi on impi.id=impi_impu.id_impi" +
				" where impi.id_imsu=?")
				.addScalar("id", Hibernate.INTEGER);
		query.setInteger(0, id_imsu);
		return query.list();
	}
	
	
}
