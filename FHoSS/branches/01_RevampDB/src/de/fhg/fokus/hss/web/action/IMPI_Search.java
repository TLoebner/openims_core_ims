package de.fhg.fokus.hss.web.action;

import java.util.LinkedList;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.hibernate.Query;
import org.hibernate.Session;

import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.op.IMPI_DAO;
import de.fhg.fokus.hss.util.HibernateUtil;
import de.fhg.fokus.hss.web.form.IMPI_SearchForm;
import de.fhg.fokus.hss.web.util.WebConstants;

public class IMPI_Search extends Action{
	
	public ActionForward execute(ActionMapping mapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		IMPI_SearchForm form = (IMPI_SearchForm) actionForm;
		Object [] queryResult = null;
		IMPI uniqueResult = null;
		
		int rowsPerPage = Integer.parseInt(form.getRowsPerPage());
		int currentPage = Integer.parseInt(form.getCrtPage()) - 1;
		int firstResult = currentPage * rowsPerPage;		
		
		HibernateUtil.beginTransaction();
		Session session = HibernateUtil.getCurrentSession();
		
		if (form.getImpi_id() != null && !form.getImpi_id().equals("")){
			uniqueResult = IMPI_DAO.getByID(session, form.getImpi_id());
		}
		else if (form.getIdentity() != null && !form.getIdentity().equals("")){
			queryResult = IMPI_DAO.getByWildcardedIdentity(session, form.getIdentity(), firstResult, rowsPerPage);
		}
		else{
			queryResult = IMPI_DAO.getAll(session, firstResult, rowsPerPage);
		}
		
		int maxPages = 1;
		if (queryResult != null){
			maxPages = ((((Integer)queryResult[0]).intValue() - 1) / rowsPerPage) + 1;
			request.setAttribute("resultList", (List)queryResult[1]);
		}
		else{
			List list = new LinkedList();
			list.add(uniqueResult);
			System.out.println("ID is:" + uniqueResult.getId());
			System.out.println("Identity is:" + uniqueResult.getIdentity());
			request.setAttribute("resultList", list);
		}

		if (currentPage > maxPages){
			currentPage = 0;
		}
		
		HibernateUtil.commitTransaction();
		HibernateUtil.closeSession();

		request.setAttribute("maxPages", String.valueOf(maxPages));
		request.setAttribute("currentPage", String.valueOf(currentPage));
		request.setAttribute("rowPerPage", String.valueOf(rowsPerPage));

		ActionForward forward = mapping.findForward(WebConstants.FORWARD_SUCCESS);
		forward = new ActionForward(forward.getPath());
		return forward;
	}
}