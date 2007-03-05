/**
 * 
 */
package de.fhg.fokus.hss.cx;
import de.fhg.fokus.hss.diam.DiameterConstants;
/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class CxExperimentalResultException extends Exception{
	private int errorCode;
	
	public CxExperimentalResultException(String message, int errorCode){
		super(message);
		this.errorCode = errorCode;
	}

	public CxExperimentalResultException(DiameterConstants.ResultCode resultCode){
		super (resultCode.getName());
		this.errorCode = resultCode.getCode();
	}
	
	public int getErrorCode() {
		return errorCode;
	}

	public void setErrorCode(int errorCode) {
		this.errorCode = errorCode;
	}
}
