/**
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
 * patents and licences may become applicable to the intended usage 
 * context.  
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */
 
/**
 * \file 
 * CDiameterPeer Diameter IMS IANA defined Command Codes
 * 
 * This is a compilation of different 3GPP TSs:
 * - TS 29.209 for IMS_Gq
 * - TS 29.229 for IMS_Cx IMS_Dx
 * - TS 29.329 for IMS_Sh IMS_Ph
 * 
 *  \author Dragos Vingarzan dragos dot vingarzan -at- fokus dot fraunhofer dot de
 *  
 */
 
#ifndef __DIAMETER_IMS_CODE_CMD_H
#define __DIAMETER_IMS_CODE_CMD_H


/*	Command Codes alocated for IMS	*/
/*		The Gq Interface 			*/
#define IMS_AAR		265		/**< Bearer-Authorization		Request	*/
#define IMS_AAA		265		/**< Bearer-Authorization		Answer	*/
#define IMS_RAR		258		/**< Re-Auth					Request */
#define IMS_RAA		258		/**< Re-Auth					Answer	*/
#define IMS_STR		275		/**< Session Termination 		Request */
#define IMS_STA		275		/**< Session Termination 		Answer	*/
#define IMS_ASR		274		/**< Abort-Session-Request		Request */
#define IMS_ASA		274		/**< Abort-Session-Request		Answer	*/
/* The Gx Interface */
#define IMS_CCR		272
#define IMS_CCA		272
/*		The Cx/Dx Interface 			*/
#define IMS_UAR		300		/**< User-Authorization			Request	*/
#define IMS_UAA		300		/**< User-Authorization			Answer	*/
#define IMS_SAR		301		/**< Server-Assignment			Request */
#define IMS_SAA		301		/**< Server-Assignment			Answer	*/
#define IMS_LIR		302		/**< Location-Info				Request */
#define IMS_LIA		302		/**< Location-Info				Answer	*/
#define IMS_MAR		303		/**< Multimedia-Auth			Request */
#define IMS_MAA		303		/**< Multimedia-Auth			Answer	*/
#define IMS_RTR		304		/**< Registration-Termination	Request */
#define IMS_RTA		304		/**< Registration-Termination	Answer	*/
#define IMS_PPR		305		/**< Push-Profile				Request */
#define IMS_PPA		305		/**< Push-Profile				Answer	*/
/**		The Sh/Ph Interface 			*/
#define IMS_UDR		306		/**< User-Data					Request */
#define IMS_UDA		306		/**< User-Data					Answer	*/
#define IMS_PUR		307		/**< Profile-Update				Request */
#define IMS_PUA		307		/**< Profile-Update				Answer	*/
#define IMS_SNR		308		/**< Subscriber-Notifications	Request */
#define IMS_SNA		308		/**< Subscriber-Notifications	Answer	*/
#define IMS_PNR		309		/**< Push-Notification			Request */
#define IMS_PNA		309		/**< Push-Notification			Answer	*/
/**	Allocated Command Codes, not used yet	*/
#define IMS_10R		310
#define IMS_10A		310
#define IMS_11R		311
#define IMS_11A		311
#define IMS_12R		312
#define IMS_12A		312
#define IMS_13R		313
#define IMS_13A		313


#endif /* __DIAMETER_IMS_CODE_CMD_H */
