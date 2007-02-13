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
 * 
 * CDiameterPeer Acceptor initialization
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>

#include "acceptor.h"
#include "utils.h"
#include "globals.h"
#include "tcp_accept.h"
#include "config.h"
#include "worker.h"


/* defined in ../diameter_peer.c */
int dp_add_pid(pid_t pid);
void dp_del_pid(pid_t pid);

/**
 * Called from diameter_peer_start, after a process was forked for this.
 * - opens the listening sockets and binds them to the addresses.
 * @param cfg - the dp_config file, which contains the list of acceptors
 * @returns - never. As this is forked, when finished it is expected to exit on itself.
 */
void acceptor_process(dp_config *cfg)
{
	int i,sock,k;
	LOG(L_INFO,"INFO:Acceptor process starting up...\n");
	listening_socks = pkg_malloc((cfg->acceptors_cnt+1)*sizeof(int));
	if (!listening_socks){
		LOG_NO_MEM("pkg",(cfg->acceptors_cnt+1)*sizeof(int));
		goto done;
	}
	memset(listening_socks,0,(cfg->acceptors_cnt+1)*sizeof(int));	
	k=0;
	for(i=0;i<cfg->acceptors_cnt;i++)
		if (create_socket(cfg->acceptors[i].port,cfg->acceptors[i].bind,&sock)){
			listening_socks[k++]=sock;			
		}	

	
	LOG(L_INFO,"INFO:... Acceptor opened sockets. Entering accept loop ...\n");		
	accept_loop();

	for(i=0;listening_socks[i];i++)
		close(listening_socks[i]);
	
	if (listening_socks) pkg_free(listening_socks);
#ifdef CDP_FOR_SER
#else
#ifdef PKG_MALLOC
	#ifdef PKG_MALLOC
		LOG(memlog, "Acceptor Memory status (pkg):\n");
		//pkg_status();
		#ifdef pkg_sums
			pkg_sums();
		#endif 
	#endif
#endif
		dp_del_pid(getpid());		
#endif		
done:		
	LOG(L_INFO,"INFO:... Acceptor process finished\n");
	exit(0);
}
