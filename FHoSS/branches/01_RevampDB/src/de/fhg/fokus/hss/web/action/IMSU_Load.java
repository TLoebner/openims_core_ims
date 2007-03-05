package de.fhg.fokus.hss.web.action;

import java.util.LinkedList;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.hibernate.Session;


import de.fhg.fokus.hss.db.model.CapabilitiesSet;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMSU;
import de.fhg.fokus.hss.db.model.PreferredSet;
import de.fhg.fokus.hss.util.HibernateUtil;
import de.fhg.fokus.hss.web.form.IMSU_Form;
import de.fhg.fokus.hss.web.util.WebConstants;

public class IMSU_Load extends Action {
	
	public ActionForward execute(ActionMapping actionMapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		IMSU_Form form = (IMSU_Form) actionForm;
		int id = form.getId();
		
		if (id == -1 || form.getSelect_capabilities_set() == null || form.getSelect_preferred_scscf() == null){
			// create
			List l1;
			HibernateUtil.beginTransaction();
			l1 = HibernateUtil.getCurrentSession().createCriteria(CapabilitiesSet.class).list();
			form.setSelect_capabilities_set(new LinkedList());
			l1 = HibernateUtil.getCurrentSession().createCriteria(PreferredSet.class).list();
			form.setSelect_preferred_scscf(new LinkedList());
			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
		}
		
		if (id != -1) {
			// load
			HibernateUtil.beginTransaction();
			Session session = HibernateUtil.getCurrentSession();
			IMSU imsu = (IMSU) session.load(IMSU.class, form.getId());

			if (imsu != null){
				form.setName(imsu.getName());
				form.setDiameter_name(imsu.getDiameter_name());
				form.setScscf_name(imsu.getScscf_name());
				form.setId_capabilities_set(imsu.getId_capabilities_set());
				form.setId_preferred_scscf(imsu.getId_preferred_scscf());
			}

			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
			
		}
		
		ActionForward forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
		forward = new ActionForward(forward.getPath() + "?id=" + id);
		return forward;
	}
}
