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
 * CDiameterPeer TCP Acceptor Process 
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
#include <stdio.h>

#include "utils.h"
#include "globals.h"
#include "tcp_accept.h"
#include "receiver.h"

/* defined in ../diameter_peer.c */
int dp_add_pid(pid_t pid);


int *listening_socks=0;	/**< array of sockets listening for connections */


extern int h_errno;

/**
 * Creates a socket and binds it.
 * @param listen_port - port to listen to
 * @param bind_to - IP address to bind to - if empty, will bind to 0.0.0.0 (all)
 * @param sock - socket to be update with the identifier of the opened one
 * @returns 1 on success, 0 on error
 */
int create_socket(int listen_port,str bind_to,unsigned int *sock)
{
	unsigned char servip[4];
	struct sockaddr_in servaddr;
	unsigned int server_sock4=-1;
	unsigned int option;
	struct hostent *host=0;

	if (bind_to.len){
		host = gethostbyname(bind_to.s);
		if (!host){
			LOG(L_WARN,"WARNING:create_sockets(): Error binding to %.*s:%d >%s\n",
				bind_to.len,bind_to.s,listen_port,strerror(h_errno));
			goto error;
		}
	}
		
	LOG(L_DBG,"DBG:create_sockets: create socket and bind for IPv4...\n");

	if ((server_sock4 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		LOG(L_ERR,"ERROR:create_sockets: error creating server socket IPv4:"
			" %s\n",strerror(errno));
		goto error;
	}
	option = 1;
	setsockopt(server_sock4,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));

	memset( &servip, 0, sizeof(servip) );
	if (bind_to.len==0) {
		*((unsigned int*)servip) = INADDR_ANY;
	}else{
		memcpy( &servip, host->h_addr_list[0], 4);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(listen_port);
	servaddr.sin_addr.s_addr = *(unsigned int*)servip;

	if ((bind( 	server_sock4,
				(struct sockaddr*) &servaddr,
				sizeof(struct sockaddr_in)))==-1 ) {
		LOG(L_ERR,"ERROR:init_tcp_shell: error binding server socket IPv4:"
			" %s\n",strerror(errno));
		goto error;
	}

	if (listen( server_sock4, 4) == -1) {
		LOG(L_ERR,"ERROR:init_tcp_shell: error listening on server socket "
			"IPv4: %s\n",strerror(errno) );
		goto error;
	}

	*sock = server_sock4;	
	
	LOG(L_INFO,"INFO:create_sockets: IPv4 Listening on %d.%d.%d.%d:%d\n",
		servip[0],servip[1],servip[2],servip[3],listen_port);
	
	return 1;
error:
	
	if (server_sock4!=-1) close(server_sock4);
	return 0;
	
}

/**
 * Accepts an incoming connection by forking a receiver process.
 * @param server_sock - socket that this connection came in to
 * @param new_sock - socket to be update with the value of the accepter socket
 * @returns 1 on success, 0 on error
 */
inline static int accept_connection(int server_sock,int *new_sock)
{
	unsigned int length;
	struct sockaddr_in remote;
	int pid;
		
	/* do accept */
	length = sizeof( struct sockaddr_in);
	*new_sock = accept( server_sock, (struct sockaddr*)&remote, &length);

	if (*new_sock==-1) {
		LOG(L_ERR,"ERROR:accept_connection(): accept failed!\n");
		goto error;
	} else {
		LOG(L_INFO,"INFO:accept_connection(): new tcp connection accepted!\n");
		
	}
	
	receiver_init(*new_sock,0);

	#ifdef CDP_FOR_SER
		pid = fork_process(server_sock,"receiver R",0);
	#else
		pid = fork();
	#endif	
	if (pid<0){
		LOG(L_ERR,"ERROR:accept_connection(): fork() failed > %s\n",strerror(errno));
		goto error;
	}
	if (pid==0){		
		/* child */
		dp_add_pid(pid);
		receiver_process(*new_sock);
		LOG(L_CRIT,"ERROR:accept_connection(): receiver_process finished without exit!\n");
		exit(-1);
	}else{
		/* parent */
		LOG(L_INFO,"INFO:accept_connection(): Receiver process forked [%d]\n",pid);
	}
	
	return 1;
error:
	return 0;
}

/**
 * Accept loop that listens for incoming connections on all listening sockets.
 * When a connection is received, accept_connection() is called.
 * @returns only on shutdown
 */
void accept_loop()
{
	fd_set listen_set;
	struct timeval timeout;
	int i=0,max_sock=0,nready;
	int new_sock;
	

	while(listening_socks[i]){
		if (listening_socks[i]>max_sock) max_sock=listening_socks[i];
		i++;
	}

	while(1){
		if (shutdownx && *shutdownx) break;
		
		timeout.tv_sec=2;
		timeout.tv_usec=0;	
		FD_ZERO(&listen_set);
		i=0;
		while(listening_socks[i]){
			FD_SET(listening_socks[i],&listen_set);					
			i++;
		}

		nready = select( max_sock+1, &listen_set, 0, 0, &timeout);
		if (nready == 0){
			LOG(L_DBG,"DBG:accept_loop(): No connection attempts\n");
			continue;
		}
		if (nready == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				LOG(L_ERR,"ERROR:accept_loop(): select fails: %s\n",
					strerror(errno));
				sleep(2);
				continue;
			}
		}

		i=0;
		while(listening_socks[i]){
			if (FD_ISSET(listening_socks[i],&listen_set)){
				accept_connection(listening_socks[i],&new_sock);
			}
			i++;
		}
	}
}

