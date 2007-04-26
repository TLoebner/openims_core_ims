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

import java.io.ByteArrayInputStream;
import java.io.FileReader;
import java.io.IOException;

import org.xml.sax.*;             // The main SAX package
import org.xml.sax.helpers.*;     // SAX helper classes

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class ShDataParser extends DefaultHandler {

	private XMLReader parser;
	private InputSource inputSource;
	
	private StringBuffer accumulator;
	private String element;

	private ShData shData;
	
	private PublicIdentity publicIdentifiers;
	private TransparentData repositoryData;
	private ShIMSData shIMSData;
	private CSLocationInformation csLocationInformation;
	private PSLocationInformation psLocationInformation;
	private CSUserState csUserState;
	private PSUserState psUserState;
	private ShDataExtension shDataExtension;
	
	public ShDataParser(InputSource inputSource){
		this.inputSource =  inputSource;
		initParser();
	}
	
	public ShData getShData() {
		return shData;
	}

	public void setShData(ShData shData) {
		this.shData = shData;
	}


	public void initParser(){
		
		try{
			org.xml.sax.XMLReader parser=new org.apache.xerces.parsers.SAXParser();

			// Specify that we don't want validation.  This is the SAX2
			// API for requesting parser features.
			
			parser.setFeature("http://xml.org/sax/features/validation", false);

			// handlers
			parser.setContentHandler(this);
			parser.setErrorHandler(this);

			// Then tell the parser to parse input from that source
			parser.parse(inputSource);
        }
		catch(IOException e){
			e.printStackTrace();	
		}
		catch (SAXException e){
			e.printStackTrace();
		}
    }

	
    // Called at the beginning of parsing.  We use it as an init() method
    public void startDocument() {
        accumulator = new StringBuffer();
    }

    // When the parser encounters plain text (not XML elements), it calls
    // this method, which accumulates them in a string buffer.
    // Note that this method may be called multiple times, even with no
    // intervening elements.
    public void characters(char[] buffer, int start, int length) {
        accumulator.append(buffer, start, length);
    }

    // At the beginning of each new element, erase any accumulated text.
    
    public void startElement(String namespaceURL, String localName,
                             String qname, Attributes attr) {
    	accumulator.setLength(0);
    	if (localName.equalsIgnoreCase(ShDataTags.ShData)){
    		shData = new ShData();
    	}
    	else if (localName.equalsIgnoreCase(ShDataTags.PublicIdentifiers)){
    		publicIdentifiers = new PublicIdentity();
    	}
    	else if (localName.equalsIgnoreCase(ShDataTags.RepositoryData)){
    		repositoryData = new TransparentData();
    		shData.addRepositoryData(repositoryData);
    	}
    	else if (localName.equalsIgnoreCase(ShDataTags.Sh_IMS_Data)){
    		shIMSData = new ShIMSData();
    	}
    	else if (localName.equalsIgnoreCase(ShDataTags.CSLocationInformation)){
    		csLocationInformation = new CSLocationInformation();
    	}
    	else if (localName.equalsIgnoreCase(ShDataTags.PSLocationInformation)){
    		psLocationInformation = new PSLocationInformation();
    	}
    	else if (localName.equalsIgnoreCase(ShDataTags.CSUserState)){
    		csUserState = new CSUserState();
    	}
    	else if (localName.equalsIgnoreCase(ShDataTags.PSUserState)){
    		psUserState = new PSUserState();
    	}
    	else if (localName.equalsIgnoreCase(ShDataTags.ShDataExtension)){
    		shDataExtension = new ShDataExtension();
    	}
    }

    // Take special action when we reach the end of selected elements.
    // Although we don't use a validating parser, this method does assume
    // that the web.xml file we're parsing is valid.

    public void endElement(String namespaceURL, String localName, String qname){
    	if (localName.equals(ShDataTags.ShData)){
    	}
    	else if (localName.equals(ShDataTags.PublicIdentifiers)){
    		shData.setPublicIdentifiers(publicIdentifiers);
    		publicIdentifiers = null;
    	}
    	else if (localName.equals(ShDataTags.RepositoryData)){
    		shData.addRepositoryData(repositoryData);
    		repositoryData = null;
    	}
    	else if (localName.equals(ShDataTags.Sh_IMS_Data)){
    		shData.setShIMSData(shIMSData);
    		shIMSData = null;
    	}
    	else if (localName.equals(ShDataTags.CSLocationInformation)){
    		shData.setCsLocationInformation(csLocationInformation);
    		csLocationInformation = null;
    	}
    	else if (localName.equals(ShDataTags.PSLocationInformation)){
    		shData.setPsLocationInformation(psLocationInformation);
    		psLocationInformation = null;
    	}
    	else if (localName.equals(ShDataTags.CSUserState)){
    		shData.setCsUserState(csUserState);
    		csUserState = null;
    	}
    	else if (localName.equals(ShDataTags.PSUserState)){
    		shData.setPsUserState(psUserState);
    		psUserState = null;
    	}
    	else if (localName.equals(ShDataTags.ShDataExtension)){
    		shData.setShDataExtension(shDataExtension);
    		shDataExtension = null;
    	}
    }

    // Called at the end of parsing.  Used here to print our results.
    public void endDocument() {
    }

    // Issue a warning
    public void warning(SAXParseException exception) {
        System.err.println("WARNING: line " + exception.getLineNumber() + ": "+
                           exception.getMessage());
    }

    // Report a parsing error
    public void error(SAXParseException exception) {
        System.err.println("ERROR: line " + exception.getLineNumber() + ": " +
                           exception.getMessage());
    }

    // Report a non-recoverable error and exit
    public void fatalError(SAXParseException exception) throws SAXException {
        System.err.println("FATAL: line " + exception.getLineNumber() + ": " +
                           exception.getMessage());
        throw(exception);
    }	
    
    public static void main(String args[]){
    	
    		InputSource input;// = new InputSource(new FileReader("files//online1.xml"));
    		
    		String inputString = 
    			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" +
    			"<Sh-Data xmlns=\"urn:ietf:params:xml:ns:pidf\" >" +
    			"" +
    			
    			"</Sh-Data>";
    		
    		input = new InputSource(new ByteArrayInputStream(inputString.getBytes()));
    		ShDataParser parser = new ShDataParser(input);
    		  		
    		ShData shData = parser.getShData();
    		System.out.println(shData.toString());

 /*   	catch(IOException e){
    		e.printStackTrace();
    	}
*/
    }


}

