package de.fhg.fokus.hss.web.action;

import java.util.Iterator;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.hibernate.Session;


import de.fhg.fokus.hss.db.model.ChargingInfo;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.SP;
import de.fhg.fokus.hss.util.HibernateUtil;
import de.fhg.fokus.hss.web.form.IMPU_Form;
import de.fhg.fokus.hss.web.util.WebConstants;

public class IMPU_Submit extends Action{
	
	public ActionForward execute(ActionMapping actionMapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		IMPU_Form form = (IMPU_Form) actionForm;
		String nextAction = form.getNextAction();
		int id = form.getId();
		
		HibernateUtil.beginTransaction();
		Session session = HibernateUtil.getCurrentSession();
		
		// refresh the SP and ChargingInfo list
		List list;
		list = session.createCriteria(SP.class).list();
		form.setSelect_sp(list);
		list = session.createCriteria(ChargingInfo.class).list();
		form.setSelect_charging_info(list);
		
		if (nextAction.equals("save")){
			if (id == -1){
				// create
				IMPU impu = new IMPU();
				impu.setIdentity(form.getIdentity());
				impu.setType((short)1);
				if (form.isBarring()){
					impu.setBarring((short)1);
				}
				else{
					impu.setBarring((short)0);
				}
								
				impu.setId_impu_implicitset(0);
				impu.setWildcard_psi("");
				impu.setDisplay_name("");
				
				Iterator it = form.getSelect_sp().iterator();
				while (it.hasNext()){
					SP sp = (SP) it.next();
					if (sp.getId()== form.getId_sp()){
						impu.setSp(sp);
						break;
					}
				}
				it = form.getSelect_charging_info().iterator();
				while (it.hasNext()){
					ChargingInfo chargingInfo = (ChargingInfo) it.next();
					if (chargingInfo.getId()== form.getId_charging_info()){
						impu.setChargingInfo(chargingInfo);
						break;
					}
				}

				session.save(impu);
				impu.setId_impu_implicitset(impu.getId());
				session.saveOrUpdate(impu);
			
				form.setId(impu.getId());
				form.setId_impu_implicitset(impu.getId_impu_implicitset());
			}
			else{
				// update
				
				System.out.println("\n\nIDDDD: " + form.getId());
				IMPU impu = (IMPU)session.load(IMPU.class, form.getId());
				impu.setIdentity(form.getIdentity());
				impu.setType((short)1);
				if (form.isBarring()){
					impu.setBarring((short)1);
				}
				else{
					impu.setBarring((short)0);
				}
				Iterator it = form.getSelect_sp().iterator();
				while (it.hasNext()){
					SP sp = (SP) it.next();
					if (sp.getId()== form.getId_sp()){
						impu.setSp(sp);
						break;
					}
				}
				it = form.getSelect_charging_info().iterator();
				while (it.hasNext()){
					ChargingInfo chargingInfo = (ChargingInfo) it.next();
					if (chargingInfo.getId()== form.getId_charging_info()){
						impu.setChargingInfo(chargingInfo);
						break;
					}
				}
				impu.setId_impu_implicitset(form.getId_impu_implicitset());
				impu.setWildcard_psi("");
				impu.setDisplay_name("");
				session.saveOrUpdate(impu);
			}
			
			System.out.println("We have the save here!");
		}
		else if (nextAction.equals("refresh")){
			
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
			
			System.out.println("We have the refresh here!");
		}
		else if (nextAction.equals("ppr")){
			System.out.println("We have the ppr here!");
		}
		else if (nextAction.equals("rtr")){
			System.out.println("We have the rtr here!");
		}		

		HibernateUtil.commitTransaction();
		HibernateUtil.closeSession();
		
		ActionForward forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
		forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
		return forward;
	}
}
