package de.fhg.fokus.hss.web.action;

import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.op.IMPI_IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
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

public class IMPU_Delete extends Action {
	
	public ActionForward execute(ActionMapping mapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		try{
			DeleteForm form = (DeleteForm) actionForm;
			HibernateUtil.beginTransaction();
			Session session = HibernateUtil.getCurrentSession();
			int id = form.getId();

			// delete from IMPI_IMPU & IMPU
			IMPI_IMPU_DAO.deleteByID_IMPU(session, id);
			IMPU_DAO.deleteByID(session, id);
		} 
		finally{
			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
		}
		return mapping.findForward(WebConstants.FORWARD_SUCCESS);
	}
}
