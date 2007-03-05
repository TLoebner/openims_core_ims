/**
 * 
 */
package de.fhg.fokus.hss.db.model;

import java.util.Set;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class IMPU {
	// table fields
	private int id;
	private String identity;
	private short type;
	private short barring;
	private short user_state;
	private int id_impu_implicitset;
	private String wildcard_psi;
	private String display_name;
	private short psi_activation;
	private short can_register;

	// associations 
	private SP sp;
	private ChargingInfo chargingInfo;
	public IMPU(){}

	public short getBarring() {
		return barring;
	}

	public void setBarring(short barring) {
		this.barring = barring;
	}

	public String getDisplay_name() {
		return display_name;
	}

	public void setDisplay_name(String display_name) {
		this.display_name = display_name;
	}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getId_impu_implicitset() {
		return id_impu_implicitset;
	}

	public void setId_impu_implicitset(int id_impu_implicitset) {
		this.id_impu_implicitset = id_impu_implicitset;
	}

	public String getIdentity() {
		return identity;
	}

	public void setIdentity(String identity) {
		this.identity = identity;
	}

	public short getType() {
		return type;
	}

	public void setType(short type) {
		this.type = type;
	}

	public String getWildcard_psi() {
		return wildcard_psi;
	}

	public void setWildcard_psi(String wildcard_psi) {
		this.wildcard_psi = wildcard_psi;
	}

	public ChargingInfo getChargingInfo() {
		return chargingInfo;
	}

	public void setChargingInfo(ChargingInfo chargingInfo) {
		this.chargingInfo = chargingInfo;
	}

	public SP getSp() {
		return sp;
	}

	public void setSp(SP sp) {
		this.sp = sp;
	}

	public short getCan_register() {
		return can_register;
	}

	public void setCan_register(short can_register) {
		this.can_register = can_register;
	}

	public short getPsi_activation() {
		return psi_activation;
	}

	public void setPsi_activation(short psi_activation) {
		this.psi_activation = psi_activation;
	}

	public short getUser_state() {
		return user_state;
	}

	public void setUser_state(short user_state) {
		this.user_state = user_state;
	}
	
}
