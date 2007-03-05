package de.fhg.fokus.hss.web.action;

import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.util.HibernateUtil;
import de.fhg.fokus.hss.web.form.IMPU_SearchForm;
import de.fhg.fokus.hss.web.util.WebConstants;

public class IMPU_Search extends Action{
	
	public ActionForward execute(ActionMapping mapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse)
			throws Exception{
		
		IMPU_SearchForm form = (IMPU_SearchForm) actionForm;
		
		HibernateUtil.getCurrentSession().beginTransaction();
		Session session = HibernateUtil.getCurrentSession();

		Query query;
		if (form.getImpu_id() != null && !form.getImpu_id().equals("")){
			System.out.println("ID: ");
			query = session.createQuery("from de.fhg.fokus.hss.db.model.IMPU as impu where impu.id=?");
			query.setInteger(0, Integer.parseInt(form.getImpu_id()));
		}
		else if (form.getIdentity() != null && !form.getIdentity().equals("")){
			System.out.println("IDentity: ");
			query = session.createQuery("from de.fhg.fokus.hss.db.model.IMPU as impu where impu.identity like ?");
			query.setString(0, form.getIdentity());
		}
		else if (form.getId_impu_implicitset() != null && !form.getId_impu_implicitset().equals("")){
			System.out.println("IMPLICIT SRY: " + form.getId_impu_implicitset());
			query = session.createQuery("from de.fhg.fokus.hss.db.model.IMPU as impu where impu.id_impu_implicitset=?");
			query.setInteger(0, Integer.parseInt(form.getId_impu_implicitset()));
		}
		else{
			System.out.println("FULL: ");			
			query = session.createQuery("from de.fhg.fokus.hss.db.model.IMPU");
		}

		//List list = session.createCriteria(IMPU.class).list();
		int rowPerPage = Integer.parseInt(form.getRowsPerPage());
		int currentPage = Integer.parseInt(form.getCrtPage()) - 1;
		int maxPages = (int) ((query.list().size() - 1) / rowPerPage) + 1;

		if (currentPage > maxPages){
			currentPage = 0;
		}

		int firstResult = currentPage * rowPerPage;

		// Edit page browsing parameters
		query.setMaxResults(rowPerPage);
		query.setFirstResult(firstResult);
		request.setAttribute("resultList", query.list());
		
		
		HibernateUtil.commitTransaction();
		HibernateUtil.closeSession();

		
		request.setAttribute("maxPages", String.valueOf(maxPages));
		request.setAttribute("currentPage", String.valueOf(currentPage));
		request.setAttribute("rowPerPage", String.valueOf(rowPerPage));

		//form.setImpu_id(null);
		//form.setId_impu_implicitset(null);
		//form.setIdentity(null);
		ActionForward forward = mapping.findForward(WebConstants.FORWARD_SUCCESS);
		forward = new ActionForward(forward.getPath());
		return forward;
	}
}