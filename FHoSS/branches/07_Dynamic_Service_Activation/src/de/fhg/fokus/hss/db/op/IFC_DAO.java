/*
  *  Copyright (C) 2004-2007 FhG Fokus
  *
  *  Parts by Instrumentacion y Componentes S.A. (Inycom). Contact at: ims at inycom dot es
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

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.log4j.Logger;
import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.ApplicationServer;
import de.fhg.fokus.hss.db.model.ChargingInfo;
import de.fhg.fokus.hss.db.model.IFC;

/**
 * This class has been modified by Instrumentacion y Componentes S.A (ims at inycom dot es)
 * to support the DSAI concept according to Release 7 (methods get_all_by_Same_AS_ID,
 * get_all_by_AS_ID_and_IMPU_ID_and_DSAI_ID and get_all_by_IMPU_ID_and_DSAI_Value_Active).
 *
 * @author adp dot fokus dot fraunhofer dot de
 * Adrian Popescu / FOKUS Fraunhofer Institute
 * @author Instrumentacion y Componentes S.A (Inycom)
 * for modifications (ims at inycom dot es)
 */

public class IFC_DAO {
	private static Logger logger = Logger.getLogger(IFC_DAO.class);

	public static void insert(Session session, IFC ifc){
		session.save(ifc);
	}

	public static void update(Session session, IFC ifc){
		if (ifc.isDirtyFlag()){
			ShNotification_DAO.insert_notif_for_iFC(session, ifc);
			ifc.setDirtyFlag(false);
		}
		session.saveOrUpdate(ifc);
	}

	public static IFC get_by_ID(Session session, int id){
		Query query;
		query = session.createSQLQuery("select * from ifc where id=?")
			.addEntity(IFC.class);
		query.setInteger(0, id);

		return (IFC) query.uniqueResult();
	}

	public static IFC get_by_Name(Session session, String name){
		Query query = session.createSQLQuery("select * from ifc where name like ?")
			.addEntity(IFC.class);
		query.setString(0, name);

		IFC result = null;
		try{
			result = (IFC) query.uniqueResult();
		}
		catch(org.hibernate.NonUniqueResultException e){
			logger.error("Query did not returned an unique result! You have a duplicate in the database!");
			e.printStackTrace();
		}
		return result;
	}

	public static List get_all(Session session){
		Query query = session.createSQLQuery("select * from ifc")
			.addEntity(IFC.class);
		return query.list();
	}

	public static List get_all_by_AS_ID(Session session, int id_as){
		Query query;
		query = session.createSQLQuery("select * from ifc where id_application_server=?")
			.addEntity(IFC.class);
		query.setInteger(0, id_as);
		return query.list();
	}

	public static List get_all_by_TP_ID(Session session, int id_tp){
		Query query;
		query = session.createSQLQuery("select * from ifc where id_tp=?")
			.addEntity(IFC.class);
		query.setInteger(0, id_tp);
		return query.list();
	}

	public static int delete_by_ID(Session session, int id){
		Query query = session.createSQLQuery("delete from ifc where id=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}


	public static Object[] get_by_Wildcarded_Name(Session session, String name,
			int firstResult, int maxResults){

		Query query;
		query = session.createSQLQuery("select * from ifc where name like ?")
			.addEntity(IFC.class);
		query.setString(0, "%" + name + "%");

		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}

	public static Object[] get_by_Wildcarded_AS_Name(Session session, String as_name,
			int firstResult, int maxResults){

		Query query;
		query = session.createSQLQuery("select * from ifc, application_server where application_server.name like ? " +
				"and application_server.id=ifc.id_application_server")
			.addEntity(IFC.class);

		query.setString(0, "%" + as_name + "%");

		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}

	public static Object[] get_by_Wildcarded_TP_Name(Session session, String tp_name,
			int firstResult, int maxResults){

		Query query;
		query = session.createSQLQuery("select * from ifc, tp where tp.name like ? " +
				"and tp.id=ifc.id_tp")
			.addEntity(IFC.class);

		query.setString(0, "%" + tp_name + "%");

		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}

	public static Object[] get_all(Session session, int firstResult, int maxResults){
		Query query;
		query = session.createSQLQuery("select * from ifc")
			.addEntity(IFC.class);

		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}


	/**
     * This method returns all iFCs associated to the same AS as the iFCs given
     * <p>
     * Method developed by Instrumentacion y Componentes S.A (Inycom) (ims at inycom dot es) to support the
     * DSAI Information Element
     *
     * @param session	Hiberntate session
     * @param list_ifc	lisf of IFCs
     * @return List of IFCs
     */

	public static List get_all_by_Same_AS_ID(Session session, List list_ifc){

		List list_as= new ArrayList();

		Iterator it=list_ifc.iterator();
		IFC ifc=null;
		while (it.hasNext()){
				ifc= (IFC) it.next();
				Integer as= (Integer) ifc.getId_application_server();
				if (list_as.contains(as)){
				}
				else {
						list_as.add(as);
				}
		}
		if(list_as.isEmpty()){
			Query query2=null;
			query2=	session.createSQLQuery("select distinct (ifc.id_application_server) from ifc");
			list_as= query2.list();
		}
		Query query = null;
		query = session.createSQLQuery("select * from ifc where id_application_server in (:list_as)").addEntity(IFC.class);
		query.setParameterList("list_as", list_as);

		return query.list();
	}

	/**
     * This method returns all iFCs associated to AS, IMPU and DSAI given.
     * <p>
     * Method developed by Instrumentacion y Componentes S.A (Inycom) (ims at inycom dot es)
     * to support the DSAI Information Element
     *
     * @param session	Hibernate session
     * @param id_as	Application Server identifier
     * @param id_impu  IMPU identifier
     * @param id_dsai	DSAI identifier
     * @return List of IFCs
     */

	public static List get_all_by_AS_ID_and_IMPU_ID_and_DSAI_ID(Session session, int id_as, int id_impu, int id_dsai){
		Query query;
		query = session.createSQLQuery("select ifc.* from ifc ifc, impu impu, sp_ifc sp_ifc, dsai_ifc dsai_ifc where ifc.id_application_server=? and impu.id=? and dsai_ifc.id_dsai=? and sp_ifc.id_sp=impu.id_sp and ifc.id=sp_ifc.id_ifc;")
		.addEntity(IFC.class);
		query.setInteger(0, id_as);
		query.setInteger(1, id_impu);
		query.setInteger(2, id_dsai);
		return query.list();
	}

	/**
     * This method returns all iFCs associated to the same DSAI as the IMPU given whose DSAI-value is
     * set to active.
     * <p>
     * Method developed by Instrumentacion y Componentes S.A (Inycom) (ims at inycom dot es)
     * to support the DSAI Information Element
     *
     * @param session Hibernate session
     * @param id_impu impu identifier
     * @return List of IFC
     */

	public static List get_all_by_IMPU_ID_and_DSAI_Value_Active(Session session, int id_impu){
		Query query;
		query = session.createSQLQuery("select ifc.* from"+
				" ifc ifc, dsai_ifc dsai_ifc, dsai_impu dsai_impu where"+
				" dsai_impu.id_impu=? and dsai_impu.dsai_value=1 and "+
				" dsai_impu.id_dsai=dsai_ifc.id_dsai and dsai_ifc.id_ifc=ifc.id;")
		.addEntity(IFC.class);
		query.setInteger(0, id_impu);
		return query.list();
	}
}
