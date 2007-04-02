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


import java.util.List;

import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.Preferred_SCSCF_Set;
/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class Preferred_SCSCF_Set_DAO {
	
	public static List get_all_from_set(Session session, int id_set){
		Query query = session.createSQLQuery("select * from preferred_set where id_set=?")
			.addEntity(Preferred_SCSCF_Set.class);
		query.setInteger(0, id_set);
		return query.list();
	}

	public static List get_all_sets(Session session){
		Query query = session.createSQLQuery("select distinct id_set, name from preferred_set")
			.addEntity(Preferred_SCSCF_Set.class);
		return query.list();
	}
	
	public static int delete_set_by_set_ID(Session session, int id_set){
		Query query = session.createSQLQuery("delete from preferred_set where id_set=?");
		query.setInteger(0, id_set);
		return query.executeUpdate();
	}
	
/*	public static int delete_SCSCF_Name_from_set(Session session, String scscf_name){
		Query query = session.createQuery("delete from PreferredSet where scscf_name like ?");
		query.setString(0, scscf_name);
		return query.executeUpdate();
	}*/
}
