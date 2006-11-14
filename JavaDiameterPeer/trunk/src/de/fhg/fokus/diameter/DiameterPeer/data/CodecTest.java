/*
 * $Id$
 *
 * Copyright (C) 2004-2006 FhG Fokus
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

package de.fhg.fokus.diameter.DiameterPeer.data;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Arrays;

import org.apache.log4j.Logger;


/**
 * This class tests the DiameterCodec for correct behaviour
 * 
 * @author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */
public class CodecTest {
	
	/** The logger */
	private static final Logger LOGGER = Logger.getLogger(CodecTest.class);

	static byte[] readData(String filename)
	{
		FileInputStream fin;
		byte[] b = null;
		int count = 0;
		int len;
		try {
			fin = new FileInputStream(filename);
			len = fin.available();
			b = new byte[len];
			count = fin.read(b);
			if (count == -1) b = null;
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
		return b;
	}
	static boolean writeData(String filename,byte[] data)
	{
		FileOutputStream fout;
		try {
			File f = new File(filename);
			if (!f.exists()) f.createNewFile();
			fout = new FileOutputStream(f);			
			fout.write(data);
			fout.close();
			return true;
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
		return false;
	}
	
	public static void main(String[] args) {
		DiameterMessage msg = null;
		byte[] original={},bootstrapped={};
		long duration;
		int repeat = 1000000;
		if (args.length<1) {
			LOGGER.debug("No input file specified. Please specify it as first parameter!");
			System.exit(-1);
		}
		if (args.length<2) {
			LOGGER.debug("No output file specified. Please specify it as second parameter!");
			System.exit(-1);
		}
		original = readData(args[0]);
		if (original==null){
			LOGGER.debug("Error reading data from file. Aborting!");
			System.exit(-1);			
		}
		duration = System.currentTimeMillis();
		try {
			for(int i=0;i<repeat;i++)
				msg = Codec.decodeDiameterMessage(original,0);
		} catch (DiameterMessageDecodeException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			LOGGER.debug("Error decoding AVP. Aborting!");
			System.exit(-1);			
		}
//		try {
//			for(int i=0;i<repeat;i++)
//				avp.ungroup();
//		} catch (AVPDecodeException e1) {
//			// TODO Auto-generated catch block
//			e1.printStackTrace();
//			LOGGER.debug("Error Ungrouping AVP. Aborting!");
//			System.exit(-1);			
//		}
		for(int i=0;i<repeat;i++)
			bootstrapped = Codec.encodeDiameterMessage(msg);

		duration = System.currentTimeMillis() - duration;
		
		LOGGER.debug("Speed: "+repeat/duration*1000+" ops/sec");
		LOGGER.debug(msg);
				
		bootstrapped = Codec.encodeDiameterMessage(msg);
		if (!writeData(args[1],bootstrapped)){
			LOGGER.debug("Error writing data to file. Aborting!");
			System.exit(-1);					
		}
		if (Arrays.equals(original,bootstrapped)){
			LOGGER.debug("Bootstrapping Succesfull...");		
		}
		else{
			LOGGER.debug("Bootstrapping failed!");
			System.exit(-1);								
		}

		/* Test of AVP creation */
//		avp = new AVP(123,false,16777216);
//		avp.setData("Bubu was here");
//		bootstrapped = AVP.encode(avp);
//		if (!writeData(args[1],bootstrapped)){
//			LOGGER.debug("Error writing data to file. Aborting!");
//			System.exit(-1);					
//		}
		
		
	}
}
