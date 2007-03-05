package de.fhg.fokus.hss.web.action;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.hibernate.Session;


import de.fhg.fokus.hss.db.model.IMSU;
import de.fhg.fokus.hss.util.HibernateUtil;
import de.fhg.fokus.hss.web.form.IMSU_Form;
import de.fhg.fokus.hss.web.util.WebConstants;

public class IMSU_Submit extends Action{
	
	public ActionForward execute(ActionMapping actionMapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		String action = request.getParameter("action");
		IMSU_Form form = (IMSU_Form) actionForm;
		String nextAction = form.getNextAction();
		
		int id = form.getId();
		if (nextAction.equals("save")){
			HibernateUtil.beginTransaction();
			Session session = HibernateUtil.getCurrentSession();

			if (id == -1){
				// create
				IMSU imsu = new IMSU();
				imsu.setName(form.getName());
				imsu.setDiameter_name(form.getDiameter_name());
				imsu.setScscf_name(form.getScscf_name());
				imsu.setId_capabilities_set(form.getId_capabilities_set());
				imsu.setId_preferred_scscf(form.getId_preferred_scscf());
				session.save(imsu);
				form.setId(imsu.getId());
			}
			else{
				// update
				IMSU imsu = (IMSU) session.load(IMSU.class, form.getId());
				imsu.setName(form.getName());
				imsu.setDiameter_name(form.getDiameter_name());
				imsu.setScscf_name(form.getScscf_name());
				imsu.setId_capabilities_set(form.getId_capabilities_set());
				imsu.setId_preferred_scscf(form.getId_preferred_scscf());

				session.saveOrUpdate(imsu);
			}
			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
		}
		else if (nextAction.equals("refresh")){
			HibernateUtil.beginTransaction();
			Session session = HibernateUtil.getCurrentSession();
			IMSU imsu = (IMSU) session.load(IMSU.class, form.getId());

			if (imsu != null){
				form.setDiameter_name(imsu.getDiameter_name());
				form.setScscf_name(imsu.getScscf_name());
				form.setName(imsu.getName());
				form.setId_capabilities_set(imsu.getId_capabilities_set());
				form.setId_preferred_scscf(imsu.getId_preferred_scscf());
				
			}
			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
			System.out.println("We have the refresh here!");
			
		}
		else if (nextAction.equals("ppr")){
			System.out.println("We have the ppr here!");
		}
		else if (nextAction.equals("rtr")){
			System.out.println("We have the rtr here!");
		}		
		System.out.println("\n\nIMSU_SUBMIT FORWARD!");
		
		ActionForward forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
		forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
		return forward;
	}
}
