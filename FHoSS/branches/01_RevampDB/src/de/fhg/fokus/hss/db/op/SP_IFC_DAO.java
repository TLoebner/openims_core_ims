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

import de.fhg.fokus.hss.cx.CxConstants;
import de.fhg.fokus.hss.db.model.IFC;
import de.fhg.fokus.hss.db.model.SP_IFC;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class SP_IFC_DAO {
	
	public static SP_IFC getBySP_IFC(Session session, int id_sp, int id_ifc){
		Query query;
		query = session.createQuery("from SP_IFC where id_sp=? and id_ifc=?");
		query.setInteger(0, id_sp);
		query.setInteger(1, id_ifc);
		return (SP_IFC) query.uniqueResult();
	}
	
	public static List get_IFC_by_SP(Session session, int id_sp){
		Query query;
		query = session.createQuery(
				"from SP_IFC as sp_ifc" +
				"	inner join sp_ifc.sp" +
				"	inner join sp_ifc.ifc" +
				"		where sp_ifc.sp.id=?");
		query.setInteger(0, id_sp);
		return query.list();
	}

	public static int getUnregisteredServicesCount(Session session, int id_sp){
		// to be fixed!
		
//		Query query;
//		query = session.createQuery(
//				"select count(*) from SP_IFC as sp_ifc" +
//				"	inner join sp_ifc.sp" +
//				"	inner join sp_ifc.ifc");// +
			//	"		where sp_ifc.sp.id=? and sp_ifc.ifc.profile_part_ind=?");
		//query.setInteger(0, id_sp);
		//query.setInteger(1, CxConstants.Profile_Part_Indicator_UnRegistered);
//		Integer result = (Integer)query.uniqueResult();
		//return result.intValue();
		return 0;
	}
	
	public static List get_all_IFC_by_SP_ID(Session session, int id_sp){
		Query query;
		query = session.createSQLQuery(
					"select {SP_IFC.*}, {IFC.*} from sp_ifc SP_IFC" +
					"	inner join ifc IFC on SP_IFC.id_ifc = IFC.id" +
					"		where SP_IFC.id_sp=?")
						.addEntity(SP_IFC.class)
						.addEntity(IFC.class);
		query.setInteger(0, id_sp);
		return query.list();
	}	
}
