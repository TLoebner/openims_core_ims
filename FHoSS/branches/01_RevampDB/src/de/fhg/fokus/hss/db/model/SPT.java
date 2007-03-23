/*
  *  Copyright (C) 2004-2007 FhG Fokus
  *
  * This file is part of Open IMS Core - an open source IMS CSCFs & HSS
  * implementation
  *
  * Open IMS Core is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License, or
  * (at your option) any later version.
  *
  * For a license to use the Open IMS Core software under conditions
  * other than those described here, or to purchase support for this
  * software, please contact Fraunhofer FOKUS by e-mail at the following
  * addresses:
  *     info@open-ims.org
  *
  * Open IMS Core is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * It has to be noted that this Open Source IMS Core System is not
  * intended to become or act as a product in a commercial context! Its
  * sole purpose is to provide an IMS core reference implementation for
  * IMS technology testing and IMS application prototyping for research
  * purposes, typically performed in IMS test-beds.
  *
  * Users of the Open Source IMS Core System have to be aware that IMS
  * technology may be subject of patents and licence terms, as being
  * specified within the various IMS-related IETF, ITU-T, ETSI, and 3GPP
  * standards. Thus all Open IMS Core users have to take notice of this
  * fact and have to agree to check out carefully before installing,
  * using and extending the Open Source IMS Core System, if related
  * patents and licenses may become applicable to the intended usage
  * context. 
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  
  * 
  */

package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class SPT {
	private int id;
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
	private int id_tp;
	
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

}
