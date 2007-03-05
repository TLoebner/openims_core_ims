package de.fhg.fokus.hss.web.action;

import java.util.LinkedList;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.log4j.Logger;
import org.apache.struts.Globals;
import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.apache.struts.action.ActionMessage;
import org.apache.struts.action.ActionMessages;
import org.hibernate.LockMode;
import org.hibernate.Session;


import de.fhg.fokus.hss.db.model.ChargingInfo;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.SP;
import de.fhg.fokus.hss.util.HibernateUtil;
import de.fhg.fokus.hss.web.form.IMPU_Form;
import de.fhg.fokus.hss.web.util.WebConstants;

public class IMPU_Load extends Action {
	
	public ActionForward execute(ActionMapping actionMapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse)
			throws Exception{
		

		IMPU_Form form = (IMPU_Form) actionForm;
		int id = form.getId();
		
		if (id == -1 || form.getSelect_charging_info() == null || form.getSelect_sp() == null){
			// create
			System.out.println("We have the create here!");
			form.setUser_state(0);
			List l1;
			HibernateUtil.beginTransaction();
			l1 = HibernateUtil.getCurrentSession().createCriteria(SP.class).list();
			form.setSelect_sp(l1);

			l1 = HibernateUtil.getCurrentSession().createCriteria(ChargingInfo.class).list();
			HibernateUtil.commitTransaction();
			form.setSelect_charging_info(l1);
		}
		
		if (id != -1){
			// load
			HibernateUtil.beginTransaction();
			Session session = HibernateUtil.getCurrentSession();
			IMPU impu = (IMPU) session.load(IMPU.class, form.getId());

			if (impu != null){
				form.set_Id_sp(impu.getSp().getId());
				
				if (impu.getBarring() == 1){
					form.setBarring(true);
				}
				else{
					form.setBarring(false);
				}
				
				form.setIdentity(impu.getIdentity());
				form.setId_charging_info(impu.getChargingInfo().getId());
				form.setId_impu_implicitset(impu.getId_impu_implicitset());
			}
			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
			
		}
		
		ActionForward forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
		forward = new ActionForward(forward.getPath() + "?id=" + id);
		return forward;
	}
}
