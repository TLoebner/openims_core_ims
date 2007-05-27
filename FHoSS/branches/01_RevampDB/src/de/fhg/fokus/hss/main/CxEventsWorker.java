/**
 * 
 */
package de.fhg.fokus.hss.main;

import java.util.ArrayList;
import java.util.List;

import org.hibernate.HibernateException;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.hibernate.HibernateUtil;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.RTR_PPR;
import de.fhg.fokus.hss.db.op.IMPI_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
import de.fhg.fokus.hss.db.op.RTR_PPR_DAO;
import de.fhg.fokus.hss.diam.DiameterConstants;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class CxEventsWorker extends Thread{

	// timeout in seconds
	private int timeout;
	
	public CxEventsWorker(int timeout){
		this.timeout = timeout; 
	}
	
	public void run(){
		try{
			while (true){
				Thread.sleep(timeout * 1000);
				
				boolean dbException = false;
				try{
					Session session = HibernateUtil.getCurrentSession();
					HibernateUtil.beginTransaction();
					
					RTR_PPR rtr_ppr = RTR_PPR_DAO.get_next_available(session);
					while (rtr_ppr != null){
						
						// mark row(s) that it was already taken into consideration
						RTR_PPR_DAO.mark_all_from_grp(session, rtr_ppr.getGrp());
						
						if (rtr_ppr.getType() == 1){
							//	we have RTR
							List all_rows_from_grp = RTR_PPR_DAO.get_all_from_grp(session, rtr_ppr.getGrp());
							List<IMPI> impiList = null;
							List<IMPU> impuList = null;
							if (all_rows_from_grp != null && all_rows_from_grp.size() > 0){
								RTR_PPR row = null;
								IMPI impi = null;
								IMPU impu = null;
								for (int i = 0; i < all_rows_from_grp.size(); i++){
									row = (RTR_PPR)all_rows_from_grp.get(i);
									if (i == 0){
										impi = IMPI_DAO.get_by_ID(session, row.getId_impi());
										if (impiList == null){
											impiList = new ArrayList<IMPI>();
										}
										impiList.add(impi);
									}
									
									if (i != 0 && row.getId_impu() == -1){
										impi = IMPI_DAO.get_by_ID(session, row.getId_impi());
										impiList.add(impi);
									}
									if (row.getId_impu() != -1){
										impu = IMPU_DAO.get_by_ID(session, row.getId_impu());
										if (impuList == null){
											impuList = new ArrayList<IMPU>();
										}
										impuList.add(impu);
									}
								}
								// prepare the RTR request
								Task task = new Task (1, DiameterConstants.Command.RTR, DiameterConstants.Application.Cx);
								task.grp = row.getGrp();
								task.impiList = impiList;
								task.impuList = impuList;
								task.reasonCode = row.getSubtype();
								task.reasonInfo = row.getReason_info();
								task.diameter_name = row.getDiameter_name();
								HSSContainer.getInstance().tasksQueue.add(task);	
							}
						}
						else if (rtr_ppr.getType() == 2){
							// prepare the PPR request
							Task task = new Task (1, DiameterConstants.Command.PPR, DiameterConstants.Application.Cx);
							task.type = rtr_ppr.getSubtype();
							task.grp = rtr_ppr.getGrp();
							task.id_impi = rtr_ppr.getId_impi();
							task.id_implicit_set = rtr_ppr.getId_implicit_set();
							HSSContainer.getInstance().tasksQueue.add(task);
						}
						rtr_ppr = RTR_PPR_DAO.get_next_available(session);
					}
				}
				catch (HibernateException e){
					//logger.error("Hibernate Exception occured!\nReason:" + e.getMessage());
					e.printStackTrace();
					dbException = true;
				}
				finally{
					if (!dbException){
						HibernateUtil.commitTransaction();
					}
					HibernateUtil.closeSession();
				}		        	
			}
		}
		catch (InterruptedException e){
			e.printStackTrace();
		}
	}
}
