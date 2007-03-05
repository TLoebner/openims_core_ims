package de.fhg.fokus.hss.web.action;

import java.util.List;
import java.util.Set;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.hibernate.Session;


import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMSU;
import de.fhg.fokus.hss.db.op.IMPI_DAO;
import de.fhg.fokus.hss.db.op.IMPI_IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMSU_DAO;
import de.fhg.fokus.hss.util.HibernateUtil;
import de.fhg.fokus.hss.web.form.IMPI_Form;
import de.fhg.fokus.hss.web.util.WebConstants;
import de.fhg.fokus.hss.auth.HexCodec;

public class IMPI_Load extends Action {
	
	public ActionForward execute(ActionMapping actionMapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		

		IMPI_Form form = (IMPI_Form) actionForm;
		int id = form.getId();

		HibernateUtil.beginTransaction();
		Session session = HibernateUtil.getCurrentSession();
		
		try{
			List imsuList= IMSU_DAO.getAll(session);		
			form.setSelect_imsu(imsuList);
			
			if (id != -1){
				// load
				IMPI impi = IMPI_DAO.getByID(session, id); 

				if (impi != null){
					form.setIdentity(impi.getIdentity());
					form.setId_imsu(impi.getImsu().getId());
					form.setSecretKey(impi.getK());
					form.setAmf(HexCodec.encode(impi.getAmf()));
					form.setOp(HexCodec.encode(impi.getOp()));
					form.setSqn(HexCodec.encode(impi.getSqn()));
					form.setIp(impi.getIp());
				
					int auth_scheme = impi.getAuth_scheme();
					if ((auth_scheme & 15) == 15){
						form.setAll(true);
					}
					else{
						if ((auth_scheme & 1) == 1){
							form.setAka1(true);
						}
					
						if ((auth_scheme & 2) == 1){
							form.setAka2(true);
						}
						if ((auth_scheme & 4) == 1){
							form.setMd5(true);
						}
						if ((auth_scheme & 8) == 1){
							form.setEarly(true);
						}
					}		
					// associated IMPUs
					List list = IMPI_IMPU_DAO.getIMPU_By_IMPI(session, id);
					form.setAssociated_impu_set(list);
				}
			}
		}
		finally{
			HibernateUtil.commitTransaction();
			HibernateUtil.closeSession();
		}
		
		ActionForward forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
		forward = new ActionForward(forward.getPath() + "?id=" + id);
		return forward;
	}
}
