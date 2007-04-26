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
package de.fhg.fokus.hss.sh.data;

import java.util.Vector;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class ShData {

	private PublicIdentity publicIdentifiers;
	private Vector<TransparentData> repositoryDataList;
	private ShIMSData shIMSData;
	private CSLocationInformation csLocationInformation;
	private PSLocationInformation psLocationInformation;
	private CSUserState csUserState;
	private PSUserState psUserState;
	private ShDataExtension shDataExtension;
	
	public ShData(){}

	public String toString(){
		StringBuffer sBuffer = new StringBuffer();
		sBuffer.append("<Sh-Data>");
		
		// append all the child elements
		if (publicIdentifiers != null){
			sBuffer.append(publicIdentifiers.toString());
		}
		if (repositoryDataList != null && repositoryDataList.size() > 0){
			for (int i = 0; i < repositoryDataList.size(); i++){
				sBuffer.append(repositoryDataList.get(i).toString());
			}
		}
		if (shIMSData != null){
			sBuffer.append(shIMSData.toString());
		}
		if (csLocationInformation != null){
			sBuffer.append(csLocationInformation.toString());
		}
		if (psLocationInformation != null){
			sBuffer.append(psLocationInformation.toString());
		}
		if (csUserState != null){
			sBuffer.append(csUserState.toString());
		}
		if (psUserState != null){
			sBuffer.append(psUserState.toString());
		}
		if (shDataExtension != null){
			sBuffer.append(shDataExtension.toString());
		}
		sBuffer.append("</Sh-Data>");
		return sBuffer.toString();
	}
	
	public void addRepositoryData(TransparentData data){
		if (repositoryDataList == null){
			repositoryDataList = new Vector<TransparentData>();
		}
		repositoryDataList.add(data);
	}
	
	// getters & setters
	public CSLocationInformation getCsLocationInformation() {
		return csLocationInformation;
	}

	public void setCsLocationInformation(CSLocationInformation csLocationInformation) {
		this.csLocationInformation = csLocationInformation;
	}

	public CSUserState getCsUserState() {
		return csUserState;
	}

	public void setCsUserState(CSUserState csUserState) {
		this.csUserState = csUserState;
	}

	public PSLocationInformation getPsLocationInformation() {
		return psLocationInformation;
	}

	public void setPsLocationInformation(PSLocationInformation psLocationInformation) {
		this.psLocationInformation = psLocationInformation;
	}

	public PSUserState getPsUserState() {
		return psUserState;
	}

	public void setPsUserState(PSUserState psUserState) {
		this.psUserState = psUserState;
	}

	public PublicIdentity getPublicIdentifiers() {
		return publicIdentifiers;
	}

	public void setPublicIdentifiers(PublicIdentity publicIdentifiers) {
		this.publicIdentifiers = publicIdentifiers;
	}

	public Vector<TransparentData> getRepositoryData() {
		return repositoryDataList;
	}

	public void setRepositoryDataList(Vector<TransparentData> repositoryDataList) {
		this.repositoryDataList = repositoryDataList;
	}

	public ShDataExtension getShDataExtension() {
		return shDataExtension;
	}

	public void setShDataExtension(ShDataExtension shDataExtension) {
		this.shDataExtension = shDataExtension;
	}

	public ShIMSData getShIMSData() {
		return shIMSData;
	}

	public void setShIMSData(ShIMSData shIMSData) {
		this.shIMSData = shIMSData;
	}
	
}
