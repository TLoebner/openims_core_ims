package de.fhg.fokus.hss.web.action;

import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.struts.Globals;
import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import org.apache.struts.action.ActionMessage;
import org.apache.struts.action.ActionMessages;
import org.hibernate.Session;


import de.fhg.fokus.hss.auth.HexCodec;
import de.fhg.fokus.hss.db.model.IMPI;
import de.fhg.fokus.hss.db.model.IMPU;
import de.fhg.fokus.hss.db.model.IMSU;
import de.fhg.fokus.hss.db.op.IMPI_DAO;
import de.fhg.fokus.hss.db.op.IMPI_IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMPU_DAO;
import de.fhg.fokus.hss.db.op.IMSU_DAO;
import de.fhg.fokus.hss.util.HibernateUtil;
import de.fhg.fokus.hss.web.form.IMPI_Form;
import de.fhg.fokus.hss.web.util.WebConstants;

public class IMPI_Submit extends Action{
	
	public ActionForward execute(ActionMapping actionMapping, ActionForm actionForm,
			HttpServletRequest request, HttpServletResponse reponse) {
		
		String action = request.getParameter("action");
		IMPI_Form form = (IMPI_Form) actionForm;
		String nextAction = form.getNextAction();
		
		int id = form.getId();

		HibernateUtil.beginTransaction();
		Session session = HibernateUtil.getCurrentSession();
					
		if (nextAction.equals("save")){
			int auth_scheme;
			if (id == -1){
				// create
				auth_scheme = IMPI.generateAuthScheme(form.isAka1(), form.isAka2(), form.isMd5(), 
						form.isEarly(), form.isAll());	
				
				IMSU imsu = IMSU_DAO.getByID(session, form.getId_imsu());
				IMPI impi = IMPI_DAO.insert(session, form.getIdentity(), form.getSecretKey(), auth_scheme, form.getIp(), 
						HexCodec.decode(form.getAmf()), HexCodec.decode(form.getOp()), new String (HexCodec.decode(form.getSqn())),
						imsu);
				form.setId(impi.getId());
			}
			else{
				// update
				 auth_scheme = IMPI.generateAuthScheme(form.isAka1(), form.isAka2(), form.isMd5(), 
						form.isEarly(), form.isAll());				
				
				IMSU imsu = IMSU_DAO.getByID(session, form.getId_imsu());
				IMPI_DAO.update(session, form.getId(), form.getIdentity(), form.getSecretKey(), auth_scheme, form.getIp(),
						HexCodec.decode(form.getAmf()), HexCodec.decode(form.getOp()), new String (HexCodec.decode(form.getSqn())),
						imsu);
			}	
			
			if ((auth_scheme & 15) == 15){
				form.setAll(true);
				form.setAka1(false);
				form.setAka2(false);
				form.setMd5(false);
				form.setEarly(false);
			}
		}
		else if (nextAction.equals("refresh")){
			IMPI impi = (IMPI) IMPI_DAO.getByID(session, id);
			if (impi != null){
				form.setIdentity(impi.getIdentity());
				form.setSecretKey(impi.getK());
				int auth_scheme = impi.getAuth_scheme();
				if ((auth_scheme & 15) == 15){
					form.setAll(true);
					form.setAka1(false);
					form.setAka2(false);
					form.setMd5(false);
					form.setEarly(false);
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
				form.setIp(impi.getIp());
				
				form.setAmf(HexCodec.encode(impi.getAmf()));
				form.setOp(HexCodec.encode(impi.getOp()));
				form.setSqn(HexCodec.encode(impi.getSqn()));
				
				form.setId_imsu(impi.getImsu().getId());
			}
		}
		else if (nextAction.equals("add_impu")){
			IMPU impu = IMPU_DAO.getByIdentity(session, form.getImpu_identity());	
			if (impu != null){
				IMPI impi = IMPI_DAO.getByID(session, form.getId());
				IMPI_IMPU_DAO.insert(session, impi, impu, (short)0);
			}
			else{
				ActionMessages actionMessages = new ActionMessages();
				actionMessages.add(Globals.MESSAGE_KEY, new ActionMessage("impi.error.associated_impu_not_found"));
				saveMessages(request, actionMessages);
			}
		}
		else if (nextAction.equals("ppr")){
			System.out.println("We have the ppr here!");
		}
		else if (nextAction.equals("rtr")){
			System.out.println("We have the rtr here!");
		}		
		
		//reload the imsuList
		List imsuList= IMSU_DAO.getAll(session);		
		form.setSelect_imsu(imsuList);
		
		// reload the associated IMPUs
		IMPI impi = IMPI_DAO.getByID(session, id);
		List list = IMPI_IMPU_DAO.getIMPU_By_IMPI(session, id);
		form.setAssociated_impu_set(list);

		HibernateUtil.commitTransaction();
		HibernateUtil.closeSession();
		
		ActionForward forward = actionMapping.findForward(WebConstants.FORWARD_SUCCESS);
		forward = new ActionForward(forward.getPath() +"?id=" + form.getId());
		return forward;
	}
}
