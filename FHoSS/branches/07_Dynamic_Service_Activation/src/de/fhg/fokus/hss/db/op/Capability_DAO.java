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

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
import java.math.BigInteger;
import java.util.List;

import org.apache.log4j.Logger;
import org.hibernate.Query;
import org.hibernate.Session;
import de.fhg.fokus.hss.db.model.Capability;

public class Capability_DAO {
	private static Logger logger = Logger.getLogger(Capability_DAO.class);
	
	public static void insert(Session session, Capability cap){
		session.save(cap);
	}
	
	public static void update(Session session, Capability cap){
		session.saveOrUpdate(cap);
	}

	public static int get_cnt(Session session){
		Query query;
		query = session.createSQLQuery("select count(*) from capability");
		BigInteger result = (BigInteger) query.uniqueResult();
		return result.intValue();
	}

	public static Capability get_by_ID(Session session, int id){
		Query query;
		query = session.createSQLQuery("select * from capability where id=?")
			.addEntity(Capability.class);
		query.setInteger(0, id);

		return (Capability) query.uniqueResult();
	}	
	
	public static Capability get_by_Name(Session session, String name){
		Query query = session.createSQLQuery("select * from capability where name like ?")
			.addEntity(Capability.class);
		query.setString(0, name);
		
		
		Capability result = null;
		
		try{
			result = (Capability) query.uniqueResult();
		}
		catch(org.hibernate.NonUniqueResultException e){
			logger.error("Query did not returned an unique result! You have a duplicate in the database!");
			e.printStackTrace();
		}
		
		return result;
		
	}

	public static Object[] get_by_Wildcarded_Name(Session session, String name, 
			int firstResult, int maxResults){
		
		Query query;
		query = session.createSQLQuery("select * from capability where name like ?")
			.addEntity(Capability.class);
		query.setString(0, "%" + name + "%");

		Object[] result = new Object[2];
		result[0] = get_cnt(session);
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static List get_all(Session session){
		Query query = session.createSQLQuery("select * from capability")
			.addEntity(Capability.class);
		return query.list();
	}
	
	public static Object[] get_all(Session session, int firstResult, int maxResults){
		Query query;
		query = session.createSQLQuery("select * from capability")
			.addEntity(Capability.class);
		
		Object[] result = new Object[2];
		result[0] = new Integer(get_cnt(session));
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static int delete_by_ID(Session session, int id){
		Query query = session.createSQLQuery("delete from capability where id=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}
	
}
