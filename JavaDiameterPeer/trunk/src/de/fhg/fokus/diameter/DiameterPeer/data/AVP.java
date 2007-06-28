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

import java.util.Vector;

/**
 * \package de.fhg.fokus.diameter.DiameterPeer.data
 * Provides methods to create and manipulate of AVPs and Diameter messages.
 */

/**
 * This class defines the basic AVP data structure.
 * 
 * The attributes can be accessed directly.
 * <ul>
 * <li> If you know that one AVP is a grouped AVP then you can parse its content and fill 
 * the childs Vector by calling the avp.ungroup() method. This will also set the is_ungrouped flag.
 * <li> To create a new AVP just call one of the constructors. Don't forget to set the data with 
 * one of the setData functions or add the child avps if grouped.
 * <li> If you have child avps the data that you set with setData will be discarded on encoding,
 * as the grouped avps have priority.
 * </ul>
 * 
 * @author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */
public class AVP {
	
	/** The AVP code */
	public int code=0;
	
	/** The AVP Vendor Specific flag. If not set, vendor_id will not be used */
	public boolean flag_vendor_specific=false;
	
	/** The AVP Mandatory Flag */
	public boolean flag_mandatory=true;
	
	/** The AVP Protected Flag indicating the need for end-to-end security */
	public boolean flag_protected=false;
	
	/** The Vendor Identifier. Should only be used when the vendor specific flag is set */
	public int vendor_id=0;
		
	/** The length of the encoded data of this AVP */
	public int encoded_length=0;
	
	
	/** The binary data actually contained inside */
	public byte[] data={};
	
	/** The integer value, precomputed for faster operation if the data is an integer */
	public int int_data;
	
	/** Vector of child AVPs if the avps is a grouped one */
	public Vector childs = null;
	
	/** Indication if this AVP actually is a grouped AVP */
	public boolean is_ungrouped=false;

	
	/**
	 * Dumb constructor used on decoding
	 */
	public AVP()
	{
	}
	
	/**
	 * Creates a new AVP.
	 * 
	 * @param Code				AVP code.
	 * @param Vendor_Specific   true, if this AVP is vendor specific.
	 * @param Mandatory			true, if mandatory tag should be set.	
	 * @param Protected			true, if protected tag should be send.
	 * @param Vendor_id			Vendor-Id.
	 */
	public AVP(int Code,boolean Vendor_Specific,boolean Mandatory,boolean Protected,
			int Vendor_id)
	{
		this.code = Code;
		this.flag_vendor_specific = Vendor_Specific;
		this.flag_mandatory = Mandatory;
		this.flag_protected = Protected;
		this.vendor_id = Vendor_id;
		if (this.vendor_id!=0) this.flag_vendor_specific = true;
	}
	
	
	/**
	 * Creates a new AVP.
	 * 
	 * @param Code				AVP code.
	 * @param Mandatory			true, if mandatory tag should be set.	
	 * @param Vendor_id			Vendor-Id
	 */
	public AVP(int Code,boolean Mandatory,int Vendor_id)
	{
		this.code = Code;
		this.flag_mandatory = Mandatory;
		this.vendor_id = Vendor_id;
		if (this.vendor_id!=0) this.flag_vendor_specific = true;
	}

	/**
	 * Sets the data to an array of bytes.
	 * 
	 * @param Data 	Date field of an AVP represented in byte[].
	 */
	public void setData(byte[] Data)
	{
		this.data = Data;
		for(int i=0;i<4&&i<data.length;i++)
			int_data = (int_data<<8)|(0xFF & data[i]);		
	}

	/**
	 * Sets the data to an array of bytes starting from start for len bytes.
	 * 
	 * @param Data	Date field of an AVP represented in byte[].
	 * @param start Start position. 
	 * @param len	Length of the array.
	 */
	public void setData(byte[] Data,int start,int len)
	{
		if (len<Data.length - start)
			len = Data.length - start;
		this.data = new byte[len];
		System.arraycopy(Data,start,this.data,0,len);
		for(int i=0;i<4&&i<data.length;i++)
			int_data = (int_data<<8)|(0xFF & data[i]);
		
	}
	
	/**
	 * Sets the data to an array of char.
	 * 
	 * @param Data	Data field of an AVP represented in char[].
	 */
	public void setData(char[] Data)
	{
		this.data = new byte[Data.length];
		System.arraycopy(Data,0,this.data,0,Data.length);
		for(int i=0;i<4&&i<data.length;i++)
			int_data = (int_data<<8)|(0xFFFF & data[i]);		
	}

	/**
	 * Sets the data to the value inside the String.
	 * 
	 * @param Data	Data field of an AVP represented in String.
	 */
	public void setData(String Data)
	{
		this.data = Data.getBytes();
		for(int i=0;i<4&&i<data.length;i++)
			int_data = (int_data<<8)|(0xFF & data[i]);
	}

	/**
	 * Sets the data to a 4 byte integer value
	 *
	 * @param Data	Data field of an AVP represented in int.
	 */
	public void setData(int Data)
	{
		data = new byte[4];
		data[0] = (byte)((Data>>24) &0xFF);
		data[1] = (byte)((Data>>16) &0xFF);
		data[2] = (byte)((Data>> 8) &0xFF);
		data[3] = (byte)((Data    ) &0xFF);
		int_data = Data;
	}

	/**
	 * Get int value.
	 * 
	 * @return The data field of an AVP converted to int.
	 */
	public int getIntData() {
		return int_data;
	}
	
	/**
	 * Get byte[] value.
	 * 
	 * @return The data field of an AVP converted to byte[]. 
	 */
	public byte[] getData() {
		return data;
	}
	
	/**
	 * Adds one child avp to the avp and converts it to a grouped one.
	 * 
	 * @param child Child AVP added to this AVP.
	 */
	public void addChildAVP(AVP child)
	{
		if (!is_ungrouped) is_ungrouped = true;
		if (childs == null) childs = new Vector();
		childs.add(child);
	}
	
	/**
	 * Returns the count of the child AVPs if the AVP is a grouped one, or 0 if not.
	 * 
	 * @return the count of child avps, 0 if not.
	 */
	public int getChildCount()
	{
		if (!is_ungrouped||childs==null) return 0;
		return childs.size();
	}
	
	/**
	 * Returns the child AVP at index if the AVP is a grouped one.
	 * 
	 * @param index	The position of the child AVP.
	 * @return the found AVP or null if out of bounds.
	 */
	public AVP getChildAVP(int index)
	{
		if (!is_ungrouped||childs==null) return null;
		if (index<childs.size())
			return (AVP)childs.get(index);
		else return null;
	}
	
	/**
	 * Deletes the given AVP from the list of child avps.
	 * 
	 * @param avp The Child AVP which should be deleted. 
	 */
	public void deleteChildAVP(AVP avp)
	{
		if (!is_ungrouped||childs==null) return;
		childs.remove(avp);
	}
	
	/**
	 * Searches for an AVP inside the Vector of child avps 
	 * if the AVP is a grouped one.
	 * 
	 * @param Code		AVP code.
	 * @return the found AVP, null if not found.
	 */
	public AVP findChildAVP(int Code)
	{
		AVP avp;
		if (!is_ungrouped) return null;
		for(int i=0;i<childs.size();i++){
			avp = (AVP) childs.get(i);
			if (avp.code == Code) {
				return avp;
			}
		}
		return null;
	}
	
	/**
	 * Searches for an AVP inside the Vector of child avps if the AVP is a grouped one.
	 * 
	 * @param Code		AVP code.
	 * @param Mandatory	true, if the AVP is set with Mandatory tag.
	 * @param Vendor_id Vendor-Id of the AVP.
	 * @return the found AVP, null if not found.
	 */
	public AVP findChildAVP(int Code,boolean Mandatory,int Vendor_id)
	{
		AVP avp;
		if (!is_ungrouped) return null;
		for(int i=0;i<childs.size();i++){
			avp = (AVP) childs.get(i);
			if (avp.code == Code &&
				avp.flag_mandatory == Mandatory &&
				avp.vendor_id == Vendor_id) 
					return avp;
		}
		return null;
	}
	
	/**
	 * Searches for all AVPs with the same code inside the Vector of child avps 
	 * if the AVP is a grouped one.
	 * 
	 * @param Code		AVP code.
	 * @return the found AVP[], null if not found.
	 */
	public AVP[] findChildAVPs(int Code)
	{
		
		AVP[] avpset;
		int j = 0, count = 0;
		AVP avp;
		
		if (!is_ungrouped) return null;
		
		for(int i=0;i<childs.size();i++){
			avp = (AVP) childs.get(i);
			if (avp.code == Code)
					count++;
		}
		
		if (count == 0) return null;
		avpset = new AVP[count];
		for(int i=0;i<childs.size();i++){
			avp = (AVP) childs.get(i);
			if (avp.code == Code) {
				avpset[j++] = avp;
				if (j == count) break;
			}
		}

		return avpset;
	}

	/**
	 *	Ungroups the AVP data into childs
	 *
	 *	@throws AVPDecodeException If upgroup fails.
	 */
	public void ungroup() throws AVPDecodeException
	{
		int i = 0;
		AVP x;
		if (is_ungrouped) return;
		is_ungrouped = true;
		childs = new Vector();
		while (i<data.length){
			x = Codec.decodeAVP(data,i);
			childs.add(x);
			i+=x.encoded_length;
		}
	}
	
	/**
	 * Groups the child AVPs into the data field.
	 *
	 */
	public void group() 
	{
		Vector temp;
		int i,len=0;
		byte[] t;
		if (!is_ungrouped||childs==null||childs.size()==0) {
			data = new byte[0];
		}
		temp = new Vector();
		for(i=0;i<childs.size();i++){
			t = Codec.encodeAVP((AVP)childs.get(i));
			temp.add(t);
			len += t.length;
		}
		data = new byte[len];
		len = 0;
		for(i=0;i<temp.size();i++){
			t = (byte[]) temp.get(i);
			System.arraycopy(t,0,data,len,t.length);
			len += t.length;
		}
	}
	
	/**
	 * Human readable version of the AVP for logging.
	 */
	public String toString()
	{
		StringBuffer x = new StringBuffer();

		int i;
		x.append("AVP:");
		x.append(" Code=");x.append(code);
		if (flag_vendor_specific) x.append(" V");
		if (flag_mandatory) x.append(" M");
		if (flag_protected) x.append(" P");
		x.append(" Len=");x.append(data.length);
		if (flag_vendor_specific){
			x.append(" V_ID=");x.append(vendor_id);
		}
		x.append(" Data=0x");
		for(i=0;i<data.length;i++){
			x.append(hexa[(data[i]>>4)&0x0F]);
			x.append(hexa[data[i]&0x0F]);
		}
		x.append(" INT_Data=");x.append(int_data);
		x.append(" Char_Data=\"");
		for(i=0;i<data.length;i++)
			if (data[i]>=32&&data[i]<=126)
				x.append((char)data[i]);
			else 
				x.append(".");
		x.append("\"");
		if (is_ungrouped){
			x.append(" {");
			for(i=0;i<childs.size();i++){
				if (i!=0) x.append(",");
				x.append(childs.get(i).toString());
			}
			x.append("}");
		}
		return x.toString();
	}

	
	private static char[] hexa={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	
	/** Session-Id AVP code. */
	public static final int Session_Id			= 263;
	
	/** Origin-Host AVP code. */
	public static final int Origin_Host			= 264;
	
	/** Origin-Realm AVP code. */
	public static final int Origin_Realm		= 296;

	/** Destination-Host AVP code. */
	public static final int Destination_Host	= 293;
	
	/** Destination-Realm AVP code. */
	public static final int Destination_Realm	= 283;
	
	/** Auth-Application-Id AVP code */
	public static final int Auth_Application_Id	= 258;
	
	/** Acct-Application-Id AVP code */
	public static final int Acct_Application_Id	= 259;
	
	/** Vendor-Specific-Application-Id AVP code */
	public static final int Vendor_Specific_Application_Id	= 260;
	
	/** Vendor-Id AVP code */
	public static final int Vendor_Id			= 266;
	
	/** Result-Code AVP code */
	public static final int Result_Code			= 268;
	
	/** Host-IP-Address AVP code */
	public static final int Host_IP_Address		= 257;
	
	/** Product-Name AVP code */
	public static final int Product_Name		= 269;
	
	/** Disconnect-Cause AVP code */
	public static final int Disconnect_Cause	= 273;
	
}
