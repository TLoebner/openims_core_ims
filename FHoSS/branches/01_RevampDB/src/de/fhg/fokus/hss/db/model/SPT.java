/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class SPT {
	private int id;
	private int id_tp;
	private int condition_negated;
	private int grp;
	private int type;
	private String requesturi;
	private String method;
	private String header;
	private String header_content;
	private int session_case;
	private String sdp_line;
	private String sdp_line_content;
	private int registration_type;
	
	//associations
	private TP tp;
	
	public SPT(){}

	public int getCondition_negated() {
		return condition_negated;
	}

	public void setCondition_negated(int condition_negated) {
		this.condition_negated = condition_negated;
	}

	public int getGrp() {
		return grp;
	}

	public void setGrp(int grp) {
		this.grp = grp;
	}

	public String getHeader() {
		return header;
	}

	public void setHeader(String header) {
		this.header = header;
	}

	public String getHeader_content() {
		return header_content;
	}

	public void setHeader_content(String header_content) {
		this.header_content = header_content;
	}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getId_tp() {
		return id_tp;
	}

	public void setId_tp(int id_tp) {
		this.id_tp = id_tp;
	}

	public String getMethod() {
		return method;
	}

	public void setMethod(String method) {
		this.method = method;
	}

	public int getRegistration_type() {
		return registration_type;
	}

	public void setRegistration_type(int registration_type) {
		this.registration_type = registration_type;
	}

	public String getRequesturi() {
		return requesturi;
	}

	public void setRequesturi(String requesturi) {
		this.requesturi = requesturi;
	}

	public String getSdp_line() {
		return sdp_line;
	}

	public void setSdp_line(String sdp_line) {
		this.sdp_line = sdp_line;
	}

	public String getSdp_line_content() {
		return sdp_line_content;
	}

	public void setSdp_line_content(String sdp_line_content) {
		this.sdp_line_content = sdp_line_content;
	}

	public int getSession_case() {
		return session_case;
	}

	public void setSession_case(int session_case) {
		this.session_case = session_case;
	}

	public int getType() {
		return type;
	}

	public void setType(int type) {
		this.type = type;
	}

	public TP getTp() {
		return tp;
	}

	public void setTp(TP tp) {
		this.tp = tp;
	}
	
}
