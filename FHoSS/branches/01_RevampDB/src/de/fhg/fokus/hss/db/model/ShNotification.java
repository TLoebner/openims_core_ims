/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class ShNotification {
	private int id;
	private int id_impu;
	private int id_application_server;
	private short type;
	private String rep_data;
	private int id_ifc;
	private String scscf_name;
	private int reg_state;

	public ShNotification(){}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getId_application_server() {
		return id_application_server;
	}

	public void setId_application_server(int id_application_server) {
		this.id_application_server = id_application_server;
	}

	public int getId_ifc() {
		return id_ifc;
	}

	public void setId_ifc(int id_ifc) {
		this.id_ifc = id_ifc;
	}

	public int getId_impu() {
		return id_impu;
	}

	public void setId_impu(int id_impu) {
		this.id_impu = id_impu;
	}

	public int getReg_state() {
		return reg_state;
	}

	public void setReg_state(int reg_state) {
		this.reg_state = reg_state;
	}

	public String getRep_data() {
		return rep_data;
	}

	public void setRep_data(String rep_data) {
		this.rep_data = rep_data;
	}

	public String getScscf_name() {
		return scscf_name;
	}

	public void setScscf_name(String scscf_name) {
		this.scscf_name = scscf_name;
	}

	public short getType() {
		return type;
	}

	public void setType(short type) {
		this.type = type;
	}

}

