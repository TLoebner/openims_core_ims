package de.fhg.fokus.hss.web.action;

import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPI_IMPU;
import de.fhg.fokus.hss.db.op.IMPI_DAO;
import de.fhg.fokus.hss.db.op.IMPI_IMPU_DAO;
import de.fhg.fokus.hss.web.form.DeleteForm;
import de.fhg.fokus.hss.util.HibernateUtil;
import de.fhg.fokus.hss.web.util.WebConstants;

import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.hibernate.Session;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

public class IMPI_Delete extends Action {
	
	public ActionForward execute(ActionMapping mapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		ActionForward actionForward = null;
		
		try{
			HibernateUtil.beginTransaction();

			DeleteForm form = (DeleteForm) actionForm;
			Session session = HibernateUtil.getCurrentSession();
			String action = request.getParameter("action");
			
			if (action == null){
				// delete from IMPI_IMPU
				int id_impi = form.getId();
				IMPI_IMPU_DAO.deleteByID_IMPI(session, id_impi);
			
				// 	delete from IMPI
				IMPI_DAO.deleteByID(session, id_impi);
				
				actionForward = mapping.findForward(WebConstants.FORWARD_SUCCESS); 
			}
			else if (action.equals("delete_associated_impu")){
				int id_impu = Integer.parseInt(request.getParameter("id_impu"));
				IMPI_IMPU_DAO.deleteByIMPI_IMPU(session, form.getId(), id_impu);
				actionForward = mapping.findForward("success_impi_impu");
			}
		} 
		finally{
			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
		}

		return actionForward;
	}
}
