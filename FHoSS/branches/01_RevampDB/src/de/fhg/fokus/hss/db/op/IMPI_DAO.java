/**
 * 
 */
package de.fhg.fokus.hss.db.op;

import org.hibernate.Query;
import org.hibernate.Session;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMSU;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class IMPI_DAO {

	public static IMPI insert(Session session, String identity, String key, int auth_scheme, String ip, 
			byte[] amf, byte[] op, String sqn, IMSU imsu){
		
		IMPI impi = new IMPI();
		impi.setIdentity(identity);
		impi.setK(key);
		impi.setAuth_scheme(auth_scheme);
		impi.setIp(ip);
		impi.setAmf(amf);
		impi.setOp(op);
		impi.setSqn(sqn);
		impi.setImsu(imsu);
		session.save(impi);
		
		return impi;
	}
	
	public static IMPI update(Session session, int id, String identity, String key, int auth_scheme, String ip,   
			byte[] amf, byte[] op, String sqn, IMSU imsu){
		
		IMPI impi = (IMPI) session.load(IMPI.class, id);
		impi.setIdentity(identity);
		impi.setK(key);
		impi.setAuth_scheme(auth_scheme);
		impi.setIp(ip);
		impi.setAmf(amf);
		impi.setOp(op);
		impi.setSqn(sqn);
		impi.setImsu(imsu);
		session.saveOrUpdate(impi);
		
		return impi;
	}

	public static IMPI update(Session session, int id, String sqn){
		
		IMPI impi = (IMPI) session.load(IMPI.class, id);
		impi.setSqn(sqn);
		session.saveOrUpdate(impi);
		return impi;
	}
	
	
	public static IMPI getByID(Session session, String id){
		return (IMPI) session.load(IMPI.class, Integer.parseInt(id));
	}
	
	public static IMPI getByID(Session session, int id){
		return (IMPI) session.load(IMPI.class, id);
	}
	
	public static IMPI getByIdentity(Session session, String identity){
		Query query;
		query = session.createQuery("from IMPI where identity like ?");
		query.setString(0, identity);
		return (IMPI) query.uniqueResult();
	}
	
	public static Object[] getByWildcardedIdentity(Session session, String identity, int firstResult, int maxResults){
		Query query;
		query = session.createQuery("from IMPI where identity like ?");
		query.setString(0, "%" + identity + "%");

		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static Object[] getAll(Session session, int firstResult, int maxResults){
		Query query;
		query = session.createQuery("from IMPI");
		
		Object[] result = new Object[2];
		result[0] = new Integer(query.list().size());
		query.setFirstResult(firstResult);
		query.setMaxResults(maxResults);
		result[1] = query.list();
		return result;
	}
	
	public static int deleteByID(Session session, int id){
		Query query = session.createQuery("delete from IMPI where id=?");
		query.setInteger(0, id);
		return query.executeUpdate();
	}
}
