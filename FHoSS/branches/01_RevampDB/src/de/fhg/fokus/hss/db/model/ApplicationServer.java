/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class ApplicationServer {
	private int id;
	private String name;
	private String server_name;
	private int default_handling;
	private String service_info;
	private String diameter_address;
	private int rep_data_size_limit;
	private short udr;
	private short pur;
	private short snr;
	private short udr_rep_data;
	private short udr_impu;
	private short udr_ims_user_state;
	private short udr_scscf_name;
	private short udr_ifc;
	private short udr_location;
	private short udr_user_state;
	private short udr_charging_info;
	private short udr_msisdn;
	private short udr_psi_activation;
	private short udr_dsai;
	private short udr_aliases_rep_data;
	private short pur_rep_data;
	private short pur_psi_activation;
	private short pur_dsai;
	private short pur_aliases_rep_data;
	private short snr_rep_data;
	private short snr_impu;
	private short snr_ims_user_state;
	private short snr_scscf_name;
	private short snr_ifc;
	private short snr_psi_activation;
	private short snr_dsai;
	private short snr_aliases_rep_data;
	
	public ApplicationServer(){}

	// getters & setters
	public int getDefault_handling() {
		return default_handling;
	}

	public void setDefault_handling(int default_handling) {
		this.default_handling = default_handling;
	}

	public String getDiameter_address() {
		return diameter_address;
	}

	public void setDiameter_address(String diameter_address) {
		this.diameter_address = diameter_address;
	}

	void setId(int id){
		this.id = id;
	}
	public int getId() {
		return id;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public short getPur() {
		return pur;
	}

	public void setPur(short pur) {
		this.pur = pur;
	}

	public short getPur_aliases_rep_data() {
		return pur_aliases_rep_data;
	}

	public void setPur_aliases_rep_data(short pur_aliases_rep_data) {
		this.pur_aliases_rep_data = pur_aliases_rep_data;
	}

	public short getPur_dsai() {
		return pur_dsai;
	}

	public void setPur_dsai(short pur_dsai) {
		this.pur_dsai = pur_dsai;
	}

	public short getPur_psi_activation() {
		return pur_psi_activation;
	}

	public void setPur_psi_activation(short pur_psi_activation) {
		this.pur_psi_activation = pur_psi_activation;
	}

	public short getPur_rep_data() {
		return pur_rep_data;
	}

	public void setPur_rep_data(short pur_rep_data) {
		this.pur_rep_data = pur_rep_data;
	}

	public int getRep_data_size_limit() {
		return rep_data_size_limit;
	}

	public void setRep_data_size_limit(int rep_data_size_limit) {
		this.rep_data_size_limit = rep_data_size_limit;
	}

	public String getServer_name() {
		return server_name;
	}

	public void setServer_name(String server_name) {
		this.server_name = server_name;
	}

	public String getService_info() {
		return service_info;
	}

	public void setService_info(String service_info) {
		this.service_info = service_info;
	}

	public short getSnr() {
		return snr;
	}

	public void setSnr(short snr) {
		this.snr = snr;
	}

	public short getSnr_aliases_rep_data() {
		return snr_aliases_rep_data;
	}

	public void setSnr_aliases_rep_data(short snr_aliases_rep_data) {
		this.snr_aliases_rep_data = snr_aliases_rep_data;
	}

	public short getSnr_dsai() {
		return snr_dsai;
	}

	public void setSnr_dsai(short snr_dsai) {
		this.snr_dsai = snr_dsai;
	}

	public short getSnr_ifc() {
		return snr_ifc;
	}

	public void setSnr_ifc(short snr_ifc) {
		this.snr_ifc = snr_ifc;
	}

	public short getSnr_impu() {
		return snr_impu;
	}

	public void setSnr_impu(short snr_impu) {
		this.snr_impu = snr_impu;
	}

	public short getSnr_ims_user_state() {
		return snr_ims_user_state;
	}

	public void setSnr_ims_user_state(short snr_ims_user_state) {
		this.snr_ims_user_state = snr_ims_user_state;
	}

	public short getSnr_psi_activation() {
		return snr_psi_activation;
	}

	public void setSnr_psi_activation(short snr_psi_activation) {
		this.snr_psi_activation = snr_psi_activation;
	}

	public short getSnr_rep_data() {
		return snr_rep_data;
	}

	public void setSnr_rep_data(short snr_rep_data) {
		this.snr_rep_data = snr_rep_data;
	}

	public short getSnr_scscf_name() {
		return snr_scscf_name;
	}

	public void setSnr_scscf_name(short snr_scscf_name) {
		this.snr_scscf_name = snr_scscf_name;
	}

	public short getUdr() {
		return udr;
	}

	public void setUdr(short udr) {
		this.udr = udr;
	}

	public short getUdr_aliases_rep_data() {
		return udr_aliases_rep_data;
	}

	public void setUdr_aliases_rep_data(short udr_aliases_rep_data) {
		this.udr_aliases_rep_data = udr_aliases_rep_data;
	}

	public short getUdr_charging_info() {
		return udr_charging_info;
	}

	public void setUdr_charging_info(short udr_charging_info) {
		this.udr_charging_info = udr_charging_info;
	}

	public short getUdr_dsai() {
		return udr_dsai;
	}

	public void setUdr_dsai(short udr_dsai) {
		this.udr_dsai = udr_dsai;
	}

	public short getUdr_ifc() {
		return udr_ifc;
	}

	public void setUdr_ifc(short udr_ifc) {
		this.udr_ifc = udr_ifc;
	}

	public short getUdr_impu() {
		return udr_impu;
	}

	public void setUdr_impu(short udr_impu) {
		this.udr_impu = udr_impu;
	}

	public short getUdr_ims_user_state() {
		return udr_ims_user_state;
	}

	public void setUdr_ims_user_state(short udr_ims_user_state) {
		this.udr_ims_user_state = udr_ims_user_state;
	}

	public short getUdr_location() {
		return udr_location;
	}

	public void setUdr_location(short udr_location) {
		this.udr_location = udr_location;
	}

	public short getUdr_msisdn() {
		return udr_msisdn;
	}

	public void setUdr_msisdn(short udr_msisdn) {
		this.udr_msisdn = udr_msisdn;
	}

	public short getUdr_psi_activation() {
		return udr_psi_activation;
	}

	public void setUdr_psi_activation(short udr_psi_activation) {
		this.udr_psi_activation = udr_psi_activation;
	}

	public short getUdr_rep_data() {
		return udr_rep_data;
	}

	public void setUdr_rep_data(short udr_rep_data) {
		this.udr_rep_data = udr_rep_data;
	}

	public short getUdr_scscf_name() {
		return udr_scscf_name;
	}

	public void setUdr_scscf_name(short udr_scscf_name) {
		this.udr_scscf_name = udr_scscf_name;
	}

	public short getUdr_user_state() {
		return udr_user_state;
	}

	public void setUdr_user_state(short udr_user_state) {
		this.udr_user_state = udr_user_state;
	}
	
}
