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
 * CDiameterPeer Receiver process procedures 
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
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "utils.h"
#include "globals.h"
#include "diameter_api.h"
#include "peerstatemachine.h"
#include "peermanager.h"
#include "config.h"

#include "receiver.h"

extern dp_config *config;		/**< Configuration for this diameter peer 	*/

int dp_add_pid(pid_t pid);
void dp_del_pid(pid_t pid);

void receive_loop();

void receive_message(AAAMessage *msg,serviced_peer_t *sp);
int serviced_peer_connect(serviced_peer_t *sp);

/** prefix for the send FIFO pipes */
#define PIPE_PREFIX "/tmp/cdp_send_"
//int pipe_fd;		/**< file descriptor for reading from the send pipe */
//int pipe_fd_out;	/**< file descriptor for writting to the send pipe */
//str pipe_name;		/**< full path to the pipe	*/

int local_id=0;					/**< incrementing process local variable, to distinguish between different peer send pies */


int fd_exchange_pipe[2];		/**< pipe to pass file descriptors towards this process */

int fd_exchange_pipe_unknown;	/**< pipe to pass file descriptors towards the receiver process for unknown peers */

serviced_peer_t *serviced_peers=0; /**< pointer to the list of peers serviced by this process */


static void log_serviced_peers(int level)
{
	serviced_peer_t *sp;
#ifdef SER_MOD_INTERFACE
	if (!is_printable(level))
#else		
	if (debug<level)
#endif
		return;
	
	LOG(level,"--- Receiver ["ANSI_BLUE"%s"ANSI_GREEN"] Serviced Peers: ---\n", pt[process_no].desc);
	for(sp=serviced_peers;sp;sp=sp->next){
		LOG(level,ANSI_GREEN" Peer: ["ANSI_YELLOW"%.*s"ANSI_GREEN"]  TCP Socket: ["ANSI_YELLOW"%d"ANSI_GREEN"] Recv.State: ["ANSI_YELLOW"%d"ANSI_GREEN"]\n",
				sp->p?sp->p->fqdn.len:0,
				sp->p?sp->p->fqdn.s:0,
				sp->tcp_socket,
				sp->state);
	}
	LOG(level,"--------------------------------------------------------\n");	
}



/**
 * Makes a send pipe for signaling messages to send-out
 * @return
 */
static int make_send_pipe(serviced_peer_t *sp)
{		
	local_id++;
	sp->send_pipe_name.s = shm_malloc(sizeof(PIPE_PREFIX)+64);
	sprintf(sp->send_pipe_name.s,"%s%d_%d_%d",PIPE_PREFIX,getpid(),local_id,(unsigned int) time(0));
	sp->send_pipe_name.len = strlen(sp->send_pipe_name.s);
		
	if (mkfifo(sp->send_pipe_name.s, 0666)<0){
		LOG(L_ERR,"ERROR:make_send_pipe(): FIFO make failed > %s\n",strerror(errno));
		return 0;
	}
	sp->send_pipe_fd = open(sp->send_pipe_name.s, O_RDONLY | O_NDELAY);
	if (sp->send_pipe_fd<0){
		LOG(L_ERR,"ERROR:receiver_init(): FIFO open for read failed > %s\n",strerror(errno));
		return 0;
	}
	// we open it for writing just to keep it alive - won't close when all other writers close it
	sp->send_pipe_fd_out = open(sp->send_pipe_name.s, O_WRONLY);
	if (sp->send_pipe_fd_out<0){
		LOG(L_ERR,"ERROR:receiver_init(): FIFO open for write (keep-alive) failed > %s\n",strerror(errno));
		return 0;
	}
	
	if (sp->p) 
		sp->p->send_pipe_name=sp->send_pipe_name;
	
	return 1;	
}

/**
 * Close a send pipe for signaling messages to send-out
 * @param sp
 */
static void close_send_pipe(serviced_peer_t *sp)
{
	if (sp->send_pipe_name.s) {
		close(sp->send_pipe_fd);
		close(sp->send_pipe_fd_out);
		remove(sp->send_pipe_name.s);
		shm_free(sp->send_pipe_name.s);
		sp->send_pipe_name.s=0;
		sp->send_pipe_name.len=0;
		sp->send_pipe_fd = -1;
		sp->send_pipe_fd_out = -1;
	}	
}


/**
 * Adds a peer in the list of serviced peers by this receiver process.
 * \note This should only be called from the receiver process!!!
 * @param p - the peer to add
 * @returns 1 on success or 0 on error
 */
static serviced_peer_t* add_serviced_peer(peer *p)
{
	serviced_peer_t *sp;
	LOG(L_INFO,"INFO:add_serviced_peer(): Adding serviced_peer_t to receiver for peer [%.*s]\n",
			p?p->fqdn.len:0,
			p?p->fqdn.s:0);
	sp = pkg_malloc(sizeof(serviced_peer_t));
	if (!sp){
		LOG(L_INFO,"INFO:add_serviced_peer(): error allocating pkg mem\n");
		return 0;
	}
	memset(sp,0,sizeof(serviced_peer_t));

	sp->p = p;	
	sp->tcp_socket = -1;
	sp->prev = 0;
	if (serviced_peers) serviced_peers->prev = sp;
	serviced_peers = sp;
	
	if (!make_send_pipe(sp)){
		pkg_free(sp);
		return 0;
	}
	
	return sp;
}

/**
 * Disconnects a serviced peer, but does not deletes it from this receiver's list
 * @param sp
 * @param locked
 */
static void disconnect_serviced_peer(serviced_peer_t *sp,int locked)
{
	if (!sp) return;
	LOG(L_INFO,"INFO:drop_serviced_peer(): [%.*s] Disconnecting from peer \n",
			sp->p?sp->p->fqdn.len:0,
			sp->p?sp->p->fqdn.s:0);
	if (sp->p){
		if (!locked) lock_get(sp->p->lock);
			if (sp->p->I_sock == sp->tcp_socket) sm_process(sp->p,I_Peer_Disc,0,1,sp->tcp_socket);
			if (sp->p->R_sock == sp->tcp_socket) sm_process(sp->p,R_Peer_Disc,0,1,sp->tcp_socket);
			sp->p->send_pipe_name.s = 0;
			sp->p->send_pipe_name.len = 0;
		if (!locked) lock_release(sp->p->lock);		
	}
	sp->tcp_socket = -1;
	close_send_pipe(sp);
}

/**
 * Drops a peer from the list of serviced peers by this receiver process
 * \note This does not actually disconnect, but should be used for hand-overs of peers from one receiver to other
 * \note This should only be called from the receiver process!!!
 * @param p - the peer to drop
 */
static void drop_serviced_peer(serviced_peer_t *sp,int locked)
{
	if (!sp) return;
	LOG(L_INFO,"INFO:drop_serviced_peer(): Dropping serviced_peer_t from receiver for peer [%.*s]\n",
			sp->p?sp->p->fqdn.len:0,
			sp->p?sp->p->fqdn.s:0);

	sp->p=0;
	close_send_pipe(sp);

	if (sp->next) sp->next->prev = sp->prev;
	if (sp->prev) sp->prev->next = sp->next;
	else serviced_peers = sp->next;
	pkg_free(sp);
}


/**
 * Sends a file descriptor to another process through a pipe
 * @param pipe_fd - pipe to send through
 * @param fd - descriptor to send
 * @param p - peer to send pointer to, or null
 * @returns 1 on success, 0 on failure
 */
static int send_fd(int pipe_fd,int fd, peer *p)
{
	struct msghdr msg;
	struct iovec iov[1];
	int ret;
	
#ifdef HAVE_MSGHDR_MSG_CONTROL
	struct cmsghdr* cmsg;
	/* make sure msg_control will point to properly aligned data */
	union {
		struct cmsghdr cm;
		char control[CMSG_SPACE(sizeof(fd))];
	}control_un;
	
	msg.msg_control=control_un.control;
	/* openbsd doesn't like "more space", msg_controllen must not
	 * include the end padding */
	msg.msg_controllen=CMSG_LEN(sizeof(fd));
	
	cmsg=CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
	*(int*)CMSG_DATA(cmsg)=fd;
	msg.msg_flags=0;
#else
	msg.msg_accrights=(caddr_t) &fd;
	msg.msg_accrightslen=sizeof(fd);
#endif
	
	msg.msg_name=0;
	msg.msg_namelen=0;
	
	iov[0].iov_base=&p;
	iov[0].iov_len=sizeof(peer*);
	msg.msg_iov=iov;
	msg.msg_iovlen=1;
	
again:
	ret=sendmsg(pipe_fd, &msg, 0);
	if (ret<0){
		if (errno==EINTR) goto again;
		if ((errno!=EAGAIN) && (errno!=EWOULDBLOCK)){
			LOG(L_CRIT, "ERROR: send_fd: sendmsg failed on %d: %s\n",
					pipe_fd, strerror(errno));
			return 0;
		}
	}
	
	return 1;
}



/* receives a fd and data_len data
 * params: unix_socket 
 *         data
 *         data_len
 *         fd         - will be set to the passed fd value or -1 if no fd
 *                      was passed
 *         flags      - 0, MSG_DONTWAIT, MSG_WAITALL; same as recv_all flags
 * returns: bytes read on success, -1 on error (and sets errno) */
/**
 * Receive a file descriptor from another process
 * @param pipe_fd - pipe to read from
 * @param fd - file descriptor to fill
 * @param flags
 * @return 1 on success or 0 on failure
 */
static int receive_fd(int pipe_fd, int* fd,peer **p)
{
	struct msghdr msg;
	struct iovec iov[1];
	int new_fd;
	int ret;
	
#ifdef HAVE_MSGHDR_MSG_CONTROL
	struct cmsghdr* cmsg;
	union{
		struct cmsghdr cm;
		char control[CMSG_SPACE(sizeof(new_fd))];
	}control_un;
	
	msg.msg_control=control_un.control;
	msg.msg_controllen=sizeof(control_un.control);
#else
	msg.msg_accrights=(caddr_t) &new_fd;
	msg.msg_accrightslen=sizeof(int);
#endif
	
	msg.msg_name=0;
	msg.msg_namelen=0;
	
	iov[0].iov_base=p;
	iov[0].iov_len=sizeof(peer*);
	msg.msg_iov=iov;
	msg.msg_iovlen=1;
	
again:
	ret=recvmsg(pipe_fd, &msg, MSG_DONTWAIT|MSG_WAITALL);
	if (ret<0){
		if (errno==EINTR) goto again;
		if ((errno==EAGAIN)||(errno==EWOULDBLOCK)) goto error;
		LOG(L_CRIT, "ERROR: receive_fd: recvmsg on %d failed: %s\n",
				pipe_fd, strerror(errno));
		goto error;
	}
	if (ret==0){
		/* EOF */
		LOG(L_CRIT, "ERROR: receive_fd: EOF on %d\n", pipe_fd);
		goto error;
	}
	if (ret!=sizeof(peer *)){
		LOG(L_WARN, "WARNING: receive_fd: different number of bytes received than expected (%d from %d)"
				    "trying to fix...\n", ret, sizeof(peer*));
		goto error;
	}
	
#ifdef HAVE_MSGHDR_MSG_CONTROL
	cmsg=CMSG_FIRSTHDR(&msg);
	if ((cmsg!=0) && (cmsg->cmsg_len==CMSG_LEN(sizeof(new_fd)))){
		if (cmsg->cmsg_type!= SCM_RIGHTS){
			LOG(L_ERR, "ERROR: receive_fd: msg control type != SCM_RIGHTS\n");
			goto error;
		}
		if (cmsg->cmsg_level!= SOL_SOCKET){
			LOG(L_ERR, "ERROR: receive_fd: msg level != SOL_SOCKET\n");
			goto error;
		}
		*fd=*((int*) CMSG_DATA(cmsg));
	}else{
		
		LOG(L_ERR, "ERROR: receive_fd: no descriptor passed, cmsg=%p,"
				"len=%d\n", cmsg, (unsigned)cmsg->cmsg_len); 
		*fd=-1;
		*p=0;
		/* it's not really an error */
	}
#else
	if (msg.msg_accrightslen==sizeof(int)){
		*fd=new_fd;
	}else{
		LOG(L_ERR, "ERROR: receive_fd: no descriptor passed,"
				" accrightslen=%d\n", msg.msg_accrightslen); 
		*fd=-1;
	}
#endif
	
	return 1;
error:
	return 0;
}

/**
 * Initializes the receiver.
 * @param sock - the socket to initialize with
 * @param p - the peer to initialize with, or NULL if to initialize as the generic receiver
 */
int receiver_init(peer *p)
{
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd_exchange_pipe)<0){
		 LOG(L_ERR,"ERROR:receiver_init(): socketpair(fd_exchanged_pipe) failed > %s\n",strerror(errno));
		return 0;
	}
	if (p) p->fd_exchange_pipe = fd_exchange_pipe[1];
	else fd_exchange_pipe_unknown = fd_exchange_pipe[1];
	
	return 1;
}

/**
 * The Receiver Process - calls the receiv_loop and it never returns.
 * @param sock - socket to receive data from
 * @returns never, when disconnected it will exit
 */
void receiver_process(peer *p)
{
	LOG(L_INFO,"INFO:receiver_process(): [%.*s] Receiver process doing init on new process...\n",
			p?p->fqdn.len:0,p?p->fqdn.s:0);
	if (p)	
		if (!add_serviced_peer(p)) goto done;
	
	LOG(L_INFO,"INFO:receiver_process(): [%.*s] Receiver process starting up...\n",
			p?p->fqdn.len:0,p?p->fqdn.s:0);

	log_serviced_peers(L_INFO);		

	receive_loop();

done:	
	if (!shutdownx){
		LOG(L_INFO,"ERROR:receiver_process(): [%.*s]... Receiver process cleaning-up - should not happen unless shuting down!\n",
				p?p->fqdn.len:0,p?p->fqdn.s:0);
		
	}
	LOG(L_INFO,"INFO:receiver_process(): [%.*s]... Receiver process cleaning-up.\n",
			p?p->fqdn.len:0,p?p->fqdn.s:0);

	while(serviced_peers){
		disconnect_serviced_peer(serviced_peers,0);
		drop_serviced_peer(serviced_peers,0);
	}
	/* remove pid from list of running processes */
	dp_del_pid(getpid());
	
#ifdef CDP_FOR_SER
			
#else
#ifdef PKG_MALLOC
	#ifdef PKG_MALLOC
		LOG(memlog, "Receiver[%.*s] Memory status (pkg):\n",
				p?p->fqdn.len:0,p?p->fqdn.s:0);
		//pkg_status();
		#ifdef pkg_sums
			pkg_sums();
		#endif 
	#endif
#endif
#endif		
		
	LOG(L_INFO,"INFO:receiver_process(): [%.*s]... Receiver process finished.\n",
			p?p->fqdn.len:0,p?p->fqdn.s:0);
	exit(0);
}


static inline int do_receive(serviced_peer_t *sp)
{
	int cnt,n,version;
	char *dst;
	AAAMessage *dmsg;
	
	switch (sp->state){
		case Receiver_Waiting:
			n = 1; /* wait for version */
			dst = sp->buf;
			break;
			
		case Receiver_Header: 
			n = DIAMETER_HEADER_LEN - sp->buf_len; /* waiting for rest of header */
			dst = sp->buf+sp->buf_len;
			break;
			
		case Receiver_Rest_of_Message:
			n = sp->length - sp->msg_len;	/* waiting for the rest of the message */
			dst = sp->msg+sp->msg_len;
			break;
			
		default:
			LOG(L_ERR,"ERROR:do_receive(): [%.*s] Unknown state %d\n",
					sp->p?sp->p->fqdn.len:0,
					sp->p?sp->p->fqdn.s:0,
					sp->state);			
			return 0;
	}
	
	cnt = recv(sp->tcp_socket,dst,n,0);
	
	if (cnt<=0)	return 0;
	
	switch (sp->state){
		case Receiver_Waiting:			
			version = (unsigned char)(sp->buf[0]);
			if (version!=1) {
		  		LOG(L_ERR,"ERROR:do_receive(): [%.*s] Received Unknown version [%d]\n",
		  				sp->p->fqdn.len,
		  				sp->p->fqdn.s,
		  				(unsigned char)sp->buf[0]);
		  		sp->state = Receiver_Waiting;
		  		sp->buf_len = 0;
			}else{
				sp->state = Receiver_Header;
				sp->buf_len = 1;
			}
			break;
			
		case Receiver_Header:
			sp->buf_len+=cnt;			
			if (sp->buf_len==DIAMETER_HEADER_LEN){
				sp->length = get_3bytes(sp->buf+1);
				if (sp->length>DP_MAX_MSG_LENGTH){
					LOG(L_ERR,"ERROR:do_receive(): [%.*s] Msg too big [%d] bytes\n",
							sp->p?sp->p->fqdn.len:0,
							sp->p?sp->p->fqdn.s:0,
							sp->length);
					sp->state = Receiver_Waiting;					
					return 0;
				}
				LOG(L_DBG,"DBG:receive_loop(): [%.*s] Recv Version %d Length %d\n",
						sp->p?sp->p->fqdn.len:0,
						sp->p?sp->p->fqdn.s:0,
						(unsigned char)(sp->buf[0]),
						sp->length);
				sp->msg = shm_malloc(sp->length);
				if (!sp->msg) {
					LOG_NO_MEM("shm",sp->length);
					sp->state = Receiver_Waiting;					
					return 0;
				}
				
				memcpy(sp->msg,sp->buf,sp->buf_len);
				sp->msg_len=sp->buf_len;
				sp->state = Receiver_Rest_of_Message;
			}
			break;
			
		case Receiver_Rest_of_Message:
			sp->msg_len+=cnt;
			if (sp->msg_len==sp->length){
		    	dmsg = AAATranslateMessage((unsigned char*)sp->msg,(unsigned int)sp->msg_len,1);		    	
				if (dmsg) receive_message(dmsg,sp);
				else
					shm_free(sp->msg);
				sp->msg = 0;
				sp->msg_len = 0;
				sp->buf_len = 0;
				sp->state = Receiver_Waiting;
			}
			break;
			
		default:
			LOG(L_ERR,"ERROR:do_receive(): [%.*s] Unknown state %d\n",
					sp->p?sp->p->fqdn.len:0,
					sp->p?sp->p->fqdn.s:0,
					sp->state);			
			return 0;
	}
	return 1;
}

/**
 * Select on sockets for receiving and sending stuff.
 * Selects on both the socket and on the send pipe.
 * @returns number of bytes read or -1 on error 
 */ 
static inline int select_recv()
{
	fd_set rfds,efds;
	struct timeval tv;
	int n,max=0,cnt=0;
	AAAMessage *msg=0;
	serviced_peer_t *sp,*sp2;
	peer *p;
	int fd=-1;
	
//	if (shutdownx) return -1;	
	n = 0;
	
	while(!n){
   		if (shutdownx&&*shutdownx) break;	

   		log_serviced_peers(L_DBG);		

   		max =-1;

		FD_ZERO(&rfds);
		FD_ZERO(&efds);
		
		
		FD_SET(fd_exchange_pipe[0],&rfds);
		if (fd_exchange_pipe[0]>max) max = fd_exchange_pipe[0];
		
		for(sp=serviced_peers;sp;sp=sp->next){
			if (sp->tcp_socket>=0){
				FD_SET(sp->tcp_socket,&rfds);
				FD_SET(sp->tcp_socket,&efds);
				if (sp->tcp_socket>max) max = sp->tcp_socket;
			}
			if (sp->send_pipe_fd>=0) {
				FD_SET(sp->send_pipe_fd,&rfds);
				if (sp->send_pipe_fd>max) max = sp->send_pipe_fd;
			}			
		}
		
		tv.tv_sec=1;
		tv.tv_usec=0;

//		LOG(L_CRIT,"ERROR:select_recv(): HERE\n");
		
		n = select(max+1,&rfds,0,&efds,&tv);
		if (n==-1){
			if (shutdownx&&*shutdownx) return -1;
			LOG(L_ERR,"ERROR:select_recv(): %s\n",strerror(errno));
			return -1;
		}else
			if (n){
				
				if (FD_ISSET(fd_exchange_pipe[0],&rfds)){
					/* fd exchange */
					LOG(L_DBG,"DBG:select_recv(): There is something on the fd exchange pipe\n");
					p = 0;
					fd = -1;
					if (!receive_fd(fd_exchange_pipe[0],&fd,&p)){
						LOG(L_ERR,"ERROR:select_recv(): Error reading from fd exchange pipe\n");
					}else{	
						LOG(L_DBG,"DBG:select_recv(): fd exchange pipe says fd [%d] for peer %p:[%.*s]\n",fd,
								p,
								p?p->fqdn.len:0,
								p?p->fqdn.s:0);
						if (p){
							sp2=0;
							for(sp=serviced_peers;sp;sp=sp->next)
								if (sp->p==p){
									sp2 = sp;
									break;
								}
							if (!sp2)
								sp2 = add_serviced_peer(p);
							else
								make_send_pipe(sp2);
							sp2->tcp_socket = fd;
							if (p->state == Wait_Conn_Ack){
								p->I_sock = fd;
								sm_process(p,I_Rcv_Conn_Ack,0,0,fd);
							}else{
								p->R_sock = fd;								
							}
						}else{
							sp2 = add_serviced_peer(NULL);
							sp2->tcp_socket = fd;
						}
					}
				}
									
				for(sp=serviced_peers;sp;){
					if (sp->tcp_socket>=0 && FD_ISSET(sp->tcp_socket,&efds)) {
						LOG(L_INFO,"INFO:select_recv(): [%.*s] Peer socket [%d] found on the exception list... dropping\n",
								sp->p?sp->p->fqdn.len:0,
								sp->p?sp->p->fqdn.s:0,
								sp->tcp_socket);
						goto drop_peer;
					}
					if (sp->send_pipe_fd>=0 && FD_ISSET(sp->send_pipe_fd,&rfds)) {					
						/* send */
						LOG(L_DBG,"DBG:select_recv(): There is something on the send pipe\n");
						cnt = read(sp->send_pipe_fd,&msg,sizeof(AAAMessage *));
						if (cnt==0){
							//This is very stupid and might not work well - droped messages... to be fixed
							LOG(L_INFO,"INFO:select_recv(): ReOpening pipe for read. This should not happen...\n");
							close(sp->send_pipe_fd);
							sp->send_pipe_fd = open(sp->send_pipe_name.s, O_RDONLY | O_NDELAY);
							goto receive;
						}
						if (cnt<sizeof(AAAMessage *)){
							if (cnt<0) LOG(L_ERR,"ERROR:select_recv(): Error reading from send pipe\n");
							goto receive;
						}	
						LOG(L_DBG,"DBG:select_recv(): Send pipe says [%p] %d\n",msg,cnt);
						if (sp->tcp_socket<0){
							LOG(L_ERR,"ERROR:select_recv(): got a signal to send something, but the connection was not opened");
						} else {
							while( (cnt=write(sp->tcp_socket,msg->buf.s,msg->buf.len))==-1 ) {
								if (errno==EINTR)
									continue;
								LOG(L_ERR,"INFO:select_recv(): [%.*s] write on socket [%d] returned error> %s... dropping\n",
										sp->p?sp->p->fqdn.len:0,
										sp->p?sp->p->fqdn.s:0,
										sp->tcp_socket,
										strerror(errno));
								AAAFreeMessage(&msg);
								close(sp->tcp_socket);
								goto drop_peer;								
							}
													
							if (cnt!=msg->buf.len){
								LOG(L_ERR,"INFO:select_recv(): [%.*s] write on socket [%d] only wrote %d/%d bytes... dropping\n",
										sp->p?sp->p->fqdn.len:0,
										sp->p?sp->p->fqdn.s:0,
										sp->tcp_socket,
										cnt,
										msg->buf.len);							
								AAAFreeMessage(&msg);
								close(sp->tcp_socket);								
								goto drop_peer;			
							}
						}
						AAAFreeMessage(&msg);
						//don't return, maybe there is something to read
					}
receive:
					/* receive */
					if (sp->tcp_socket>=0 && FD_ISSET(sp->tcp_socket,&rfds)) {
						errno=0;
						cnt = do_receive(sp);
						if (cnt<=0) {
							LOG(L_INFO,"INFO:select_recv(): [%.*s] read on socket [%d] returned %d > %s... dropping\n",
									sp->p?sp->p->fqdn.len:0,
									sp->p?sp->p->fqdn.s:0,
									sp->tcp_socket,
									cnt,
									errno?strerror(errno):"");							
							goto drop_peer;
						}						
					}
					
//next_sp:			
					/* go to next serviced peer */
					sp=sp->next;
					continue;
drop_peer:
					/* drop this serviced peer on error */
					sp2 = sp->next;
					disconnect_serviced_peer(sp,0);
					if (sp->p && sp->p->is_dynamic)
						drop_serviced_peer(sp,0);
					sp = sp2;					 				
				}
			}
		//LOG(L_ERR,".");
	}
	return 1;
}

/** 
 * Receive Loop for Diameter messages.
 * Decodes the message and calls receive_message().
 * @param sock - the socket to receive from
 * @returns when the socket is closed
 */
void receive_loop()
{
    while(!*shutdownx){
    	select_recv();		
    }
}




/**
 * Initiate a connection to a peer.
 * The obtained socket is then sent to the respective receiver.
 * @param p - peer to connect to
 * @returns socket if OK, -1 on error
 */
int peer_connect(peer *p)
{
	int sock;
	unsigned int option = 1;
	
	struct addrinfo *ainfo=0,*res=0,hints;		
	char buf[256],host[256],serv[256];
	int error;

	memset (&hints, 0, sizeof(hints));
	//hints.ai_protocol = IPPROTO_SCTP;
 	//hints.ai_protocol = IPPROTO_TCP;
 	hints.ai_flags = AI_ADDRCONFIG;
	hints.ai_socktype = SOCK_STREAM;
	
	sprintf(buf,"%d",p->port);

	error = getaddrinfo(p->fqdn.s, buf, &hints, &res);

	if (error!=0){
		LOG(L_WARN,"WARNING:peer_connect(): Error opening connection to %.*s:%d >%s\n",
			p->fqdn.len,p->fqdn.s,p->port,gai_strerror(error));
		goto error;
	}
		
	for(ainfo = res;ainfo;ainfo = ainfo->ai_next)
	{
		if (getnameinfo(ainfo->ai_addr,ainfo->ai_addrlen,
			host,256,serv,256,NI_NUMERICHOST|NI_NUMERICSERV)==0){
				LOG(L_WARN,"INFO:peer_connect(): Trying to connect to %s port %s\n",
					host,serv);
		}				

		if ((sock = socket(ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol)) == -1) {
			LOG(L_ERR,"ERROR:peer_connect(): error creating client socket to %s port %s >"
				" %s\n",host,serv,strerror(errno));
			continue;
		}

		if (connect(sock,ainfo->ai_addr,ainfo->ai_addrlen)!=0) {
			LOG(L_WARN,"WARNING:peer_connect(): Error opening connection to to %s port %s >%s\n",
				host,serv,strerror(errno));
			close(sock);		
			continue;
		}
	
		setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
	
		LOG(L_INFO,"INFO:peer_connect(): Peer %.*s:%d connected\n",p->fqdn.len,p->fqdn.s,p->port);

		if (!send_fd(p->fd_exchange_pipe,sock,p)){
			LOG(L_ERR,"ERROR:peer_connect(): [%.*s] Error sending fd to respective receiver\n",p->fqdn.len,p->fqdn.s);
			close(sock);
			goto error;
		}
		
		if (res) freeaddrinfo(res);			
		return sock;
	}
error:
	if (res) freeaddrinfo(res);	
	return -1;	
}


/**
 * Sends a newly connected socket to the receiver process
 * @param sock
 * @return
 */
int receiver_send_socket(int sock, peer *p)
{
	int pipe_fd;
	if (p)
		pipe_fd = p->fd_exchange_pipe;
	else
		pipe_fd = fd_exchange_pipe_unknown;
	
	return send_fd(pipe_fd,sock,p);
}


/**
 * Sends a message to a peer (to be called from other processes).
 * This just writes the pointer to the message in the send pipe. The specific
 * peer process will pick that up and send the message, as only that specific
 * process has the id of socket (we are forking the peers dynamically and as such,
 * the sockets are not visible between processes).
 * @param p - the peer to send to
 * @param msg - the message to send
 * @returns 1 on success, 0 on failure
 */
int peer_send_msg(peer *p,AAAMessage *msg)
{
	int fd,n;
	if (!AAABuildMsgBuffer(msg)) return 0;
	if (!p->send_pipe_name.s) {
		LOG(L_ERR,"ERROR:peer_send_msg(): Peer %.*s has no attached send pipe\n",p->fqdn.len,p->fqdn.s);
		return 0;
	}
	fd = open(p->send_pipe_name.s,O_WRONLY);
	if (fd<0){
		LOG(L_ERR,"ERROR:peer_send_msg(): Peer %.*s error on pipe open > %s\n",p->fqdn.len,p->fqdn.s,strerror(errno));		
		return 0;
	}
	LOG(L_DBG,"DBG:peer_send_msg(): Pipe push [%p]\n",msg);
	n = write(fd,&msg,sizeof(AAAMessage *));
	if (n<0) {
		LOG(L_ERR,"ERROR:peer_send_msg(): Peer %.*s error on pipe write > %s\n",p->fqdn.len,p->fqdn.s,strerror(errno));		
		close(fd);
		return 0;
	}
	if (n!=sizeof(AAAMessage *)) {
		LOG(L_ERR,"ERROR:peer_send_msg(): Peer %.*s error on pipe write > only %d bytes written\n",p->fqdn.len,p->fqdn.s,n);		
		close(fd);
		return 0;
	}
	close(fd);
	return 1;
}

/**
 * Send a message to a peer (only to be called from the receiver process).
 * This directly writes the message on the socket. It is used for transmission during
 * the Capability Exchange procedure, when the send pipes are not opened yet.
 * @param p - the peer to send to
 * @param sock - the socket to send through
 * @param msg - the message to send
 * @param locked - whether the caller locked the peer already
 * @returns 1 on success, 0 on error
 */
int peer_send(peer *p,int sock,AAAMessage *msg,int locked)
{
	int n;
	serviced_peer_t *sp;
//	LOG(L_CRIT,"[%d]\n",sock);

	
	if (!p||!msg||sock<0) return 0;
	LOG(L_DBG,"DBG:peer_send(): [%.*s] sending direct message to peer\n",
			p->fqdn.len,
			p->fqdn.s);
	
	if (!AAABuildMsgBuffer(msg)) return 0;
	
	if (!locked) lock_get(p->lock);

	while( (n=write(sock,msg->buf.s,msg->buf.len))==-1 ) {
		if (errno==EINTR)
			continue;
		LOG(L_ERR,"ERROR:peer_send(): write returned error: %s\n",
			strerror(errno));
		if (p->I_sock==sock) sm_process(p,I_Peer_Disc,0,1,p->I_sock);
		if (p->R_sock==sock) sm_process(p,R_Peer_Disc,0,1,p->R_sock);
		if (!locked) lock_release(p->lock);
		AAAFreeMessage(&msg);		
		return 0;
	}

	if (n!=msg->buf.len){
		LOG(L_ERR,"ERROR:peer_send(): only wrote %d/%d bytes\n",n,msg->buf.len);
		if (!locked) lock_release(p->lock);
		AAAFreeMessage(&msg);		
		return 0;
	}
	if (!locked) lock_release(p->lock);
	
	AAAFreeMessage(&msg);

	/* now switch the peer processing to its dedicated process if this is not a dynamic peer */
	if (!p->is_dynamic){
		LOG(L_DBG,"DBG:peer_send(): [%.*s] switching peer to own and dedicated receiver\n",
				p->fqdn.len,
				p->fqdn.s);
		send_fd(p->fd_exchange_pipe,sock,p);
		for(sp=serviced_peers;sp;sp=sp->next)
			if (sp->p==p){
				drop_serviced_peer(sp,locked);
				break;
			}
	}
	
	return 1;	
}


/**
 * Receives a message and does basic processing or call the sm_process().
 * This gets called from the receive_loop for every message that is received.
 * @param msg - the message received
 * @param sock - socket received on
 */
void receive_message(AAAMessage *msg,serviced_peer_t *sp)
{
	AAA_AVP *avp1,*avp2;
	LOG(L_DBG,"DBG:receive_message(): [%.*s] Recv msg %d\n",
			sp->p?sp->p->fqdn.len:0,
			sp->p?sp->p->fqdn.s:0,
			msg->commandCode);
	
	if (!sp->p){
		switch (msg->commandCode){
			case Code_CE:
				if (is_req(msg)){
					avp1 = AAAFindMatchingAVP(msg,msg->avpList.head,AVP_Origin_Host,0,0);
					avp2 = AAAFindMatchingAVP(msg,msg->avpList.head,AVP_Origin_Realm,0,0);
					if (avp1&&avp2){
						sp->p = get_peer_from_fqdn(avp1->data,avp2->data);
					}
					if (!sp->p) {
						LOG(L_ERR,"ERROR:receive_msg(): Received CER from unknown peer (accept unknown=%d) -ignored\n",
							config->accept_unknown_peers);
						AAAFreeMessage(&msg);
					}else{
						LOG(L_DBG,"DBG:receive_message(): [%.*s] This receiver has no peer associated\n",
								sp->p?sp->p->fqdn.len:0,
								sp->p?sp->p->fqdn.s:0	);						
						//set_peer_pipe();
						make_send_pipe(sp);
						sm_process(sp->p,R_Conn_CER,msg,0,sp->tcp_socket);
					}
				}
				else{
					LOG(L_ERR,"ERROR:receive_msg(): Received CEA from an unknown peer -ignored\n");
					AAAFreeMessage(&msg);
				}
				break;
			default:
				LOG(L_ERR,"ERROR:receive_msg(): Received non-CE from an unknown peer -ignored\n");
				AAAFreeMessage(&msg);				
		}
	}else{
		touch_peer(sp->p);
		switch (sp->p->state){
			case Wait_I_CEA:
				if (msg->commandCode!=Code_CE||is_req(msg)){
					sm_process(sp->p,I_Rcv_Non_CEA,msg,0,sp->tcp_socket);
				}else
					sm_process(sp->p,I_Rcv_CEA,msg,0,sp->tcp_socket);
				break;
			case I_Open:			
				switch (msg->commandCode){
					case Code_CE:
						if (is_req(msg)) sm_process(sp->p,I_Rcv_CER,msg,0,sp->tcp_socket);	
									else sm_process(sp->p,I_Rcv_CEA,msg,0,sp->tcp_socket);
						break;
					case Code_DW:
						if (is_req(msg)) sm_process(sp->p,I_Rcv_DWR,msg,0,sp->tcp_socket);	
									else sm_process(sp->p,I_Rcv_DWA,msg,0,sp->tcp_socket);
						break;
					case Code_DP:
						if (is_req(msg)) sm_process(sp->p,I_Rcv_DPR,msg,0,sp->tcp_socket);	
									else sm_process(sp->p,I_Rcv_DPA,msg,0,sp->tcp_socket);
						break;
					default:
						sm_process(sp->p,I_Rcv_Message,msg,0,sp->tcp_socket);
				}				
				break;				
			case R_Open:			
				switch (msg->commandCode){
					case Code_CE:
						if (is_req(msg)) sm_process(sp->p,R_Rcv_CER,msg,0,sp->tcp_socket);	
									else sm_process(sp->p,R_Rcv_CEA,msg,0,sp->tcp_socket);
						break;
					case Code_DW:
						if (is_req(msg)) sm_process(sp->p,R_Rcv_DWR,msg,0,sp->tcp_socket);	
									else sm_process(sp->p,R_Rcv_DWA,msg,0,sp->tcp_socket);
						break;
					case Code_DP:
						if (is_req(msg)) sm_process(sp->p,R_Rcv_DPR,msg,0,sp->tcp_socket);	
									else sm_process(sp->p,R_Rcv_DPA,msg,0,sp->tcp_socket);
						break;
					default:
						sm_process(sp->p,R_Rcv_Message,msg,0,sp->tcp_socket);
				}				
				break;				
			default:
				LOG(L_ERR,"ERROR:receive_msg(): [%.*s] Received msg while peer in state %d -ignored\n",
						sp->p->fqdn.len,
						sp->p->fqdn.s,
						sp->p->state);
				AAAFreeMessage(&msg);								
		}
	}
}
