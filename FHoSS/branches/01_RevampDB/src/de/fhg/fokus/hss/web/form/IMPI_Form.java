package de.fhg.fokus.hss.web.form;

import org.apache.struts.action.ActionErrors;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionMapping;
import org.apache.struts.action.ActionMessage;

import java.io.Serializable;
import java.util.List;
import java.util.Set;

import javax.servlet.http.HttpServletRequest;

public class IMPI_Form extends ActionForm implements Serializable{
	public static final String DEFAULT_OP = "00000000000000000000000000000000";  
	public static final String DEFAULT_AMF = "0000";
	public static final String DEFAULT_SQN = "000000000000";
	private int id;
	private int id_imsu;
	private String identity;
	private String secretKey;
	private boolean aka1;
	private boolean aka2;
	private boolean md5;
	private boolean early;
	private boolean all;
	private String amf;
	private String op;
	private String sqn;
	private String ip;
	
	private List select_imsu;
	
	private String nextAction;
	private String impu_identity;
	private List associated_impu_set;
	
    public void reset(ActionMapping actionMapping, HttpServletRequest request){
    	this.id = -1;
    	this.id_imsu = -1;
    	this.identity = null;
    	this.secretKey = null;
    	this.ip = null;
    	this.op = IMPI_Form.DEFAULT_OP;
    	this.amf = IMPI_Form.DEFAULT_AMF;
    	this.sqn = IMPI_Form.DEFAULT_SQN;
    	this.aka1 = false;
    	this.aka2 = false;
    	this.md5 = false;
    	this.early = false;
    	this.all = false;
    	
    	this.select_imsu = null;
    	this.associated_impu_set = null;
    }
	
    public ActionErrors validate(ActionMapping actionMapping, HttpServletRequest request){
        ActionErrors actionErrors = new ActionErrors();

        if (identity == null || identity.equals("")){
        	actionErrors.add("identity", new ActionMessage("impi_form.error.identity"));
        }
        if (this.id_imsu == -1){
        	actionErrors.add("id_imsu", new ActionMessage("impi_form.error.id_imsu"));
        }
        if (!(this.aka1 || this.aka2 || this.md5 || this.early || this.all)){
        	actionErrors.add("auth_scheme", new ActionMessage("impi_form.error.auth_scheme"));
        }
        if (secretKey == null || secretKey.equals("")){
        	actionErrors.add("secret_key", new ActionMessage("impi_form.error.secret_key"));
        }
        if (amf == null || amf.equals("")){
        	actionErrors.add("secret_key", new ActionMessage("impi_form.error.amf"));
        }
        if (op == null || op.equals("")){
        	actionErrors.add("secret_key", new ActionMessage("impi_form.error.op"));
        }
        return actionErrors;
    }

	public boolean isAka1() {
		return aka1;
	}


	public void setAka1(boolean aka1) {
		this.aka1 = aka1;
	}


	public boolean isAka2() {
		return aka2;
	}


	public void setAka2(boolean aka2) {
		this.aka2 = aka2;
	}


	public boolean isAll() {
		return all;
	}


	public void setAll(boolean all) {
		this.all = all;
	}


	public String getAmf() {
		return amf;
	}


	public void setAmf(String amf) {
		this.amf = amf;
	}


	public boolean isEarly() {
		return early;
	}


	public void setEarly(boolean early) {
		this.early = early;
	}


	public int getId() {
		return id;
	}


	public void setId(int id) {
		this.id = id;
	}


	public String getIdentity() {
		return identity;
	}


	public void setIdentity(String identity) {
		this.identity = identity;
	}


	public String getIp() {
		return ip;
	}


	public void setIp(String ip) {
		this.ip = ip;
	}


	public boolean isMd5() {
		return md5;
	}


	public void setMd5(boolean md5) {
		this.md5 = md5;
	}


	public String getNextAction() {
		return nextAction;
	}


	public void setNextAction(String nextAction) {
		this.nextAction = nextAction;
	}


	public String getOp() {
		return op;
	}


	public void setOp(String op) {
		this.op = op;
	}


	public String getSecretKey() {
		return secretKey;
	}


	public void setSecretKey(String secretKey) {
		this.secretKey = secretKey;
	}


	public List getSelect_imsu() {
		return select_imsu;
	}


	public void setSelect_imsu(List select_imsu) {
		this.select_imsu = select_imsu;
	}

	public String getSqn() {
		return sqn;
	}

	public void setSqn(String sqn) {
		this.sqn = sqn;
	}

	public int getId_imsu() {
		return id_imsu;
	}

	public void setId_imsu(int id_imsu) {
		this.id_imsu = id_imsu;
	}

	public String getImpu_identity() {
		return impu_identity;
	}

	public void setImpu_identity(String impu_identity) {
		this.impu_identity = impu_identity;
	}

	public List getAssociated_impu_set() {
		return associated_impu_set;
	}

	public void setAssociated_impu_set(List associated_impu_set) {
		this.associated_impu_set = associated_impu_set;
	}

	
}
