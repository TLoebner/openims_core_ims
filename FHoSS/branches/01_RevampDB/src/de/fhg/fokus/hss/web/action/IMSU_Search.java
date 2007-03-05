package de.fhg.fokus.hss.web.action;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.util.HibernateUtil;
import de.fhg.fokus.hss.web.form.IMPI_SearchForm;
import de.fhg.fokus.hss.web.form.IMSU_SearchForm;
import de.fhg.fokus.hss.web.util.WebConstants;

public class IMSU_Search extends Action{
	
	public ActionForward execute(ActionMapping mapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		IMSU_SearchForm form = (IMSU_SearchForm) actionForm;
		HibernateUtil.getCurrentSession().beginTransaction();
		Session session = HibernateUtil.getCurrentSession();

		Query query;
		if (form.getImsu_id() != null && !form.getImsu_id().equals("")){
			query = session.createQuery("from de.fhg.fokus.hss.db.model.IMSU as imsu where imsu.id=?");
			query.setInteger(0, Integer.parseInt(form.getImsu_id()));
		}
		else if (form.getName() != null && !form.getName().equals("")){
			query = session.createQuery("from de.fhg.fokus.hss.db.model.IMSU as imsu where imsu.identity like ?");
			query.setString(0, form.getName());
		}
		else{
			query = session.createQuery("from de.fhg.fokus.hss.db.model.IMSU");
		}

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

		ActionForward forward = mapping.findForward(WebConstants.FORWARD_SUCCESS);
		forward = new ActionForward(forward.getPath());
		return forward;
	}
}