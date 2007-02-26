/*
 * $id$ main.c $date $author$ Dragos Vingarzan dvi vingarzan@gmail.com
 *
 * Copyright (C) 2005 Fhg Fokus
 *
 */

#include <stdlib.h>
#include <sys/wait.h> 
#include <signal.h>

#include "main.h"
#include "cdp/diameter_peer.h"
#include "server.h"
#include "cdp/worker.h"
#include "cdp/timer.h"
#include "db.h"
#include "cdp/globals.h"
#include "server.h"

static void main_sig_handler(int signo)
{
	if ( getpid()==*dp_first_pid ) {
		/* I'am the main thread */
		switch (signo) {
			case 0: break; /* do nothing*/
			case SIGPIPE:
				LOG(L_CRIT, "WARNING: SIGPIPE received and ignored\n");
				break;
			case SIGINT:
			case SIGTERM:
				if (signo==SIGINT)
					DBG("SIGINT received, program terminates\n");
				else
					DBG("SIGTERM received, program terminates\n");
				diameter_peer_destroy();
				exit(0);
				break;
			case SIGHUP: /* ignoring it*/
				DBG("SIGHUP received, ignoring it\n");
				break;
			case SIGUSR1:
				LOG(memlog, "Memory status (shm):\n");
				shm_status();
				break;
			default:
				LOG(L_CRIT, "WARNING: unhandled signal %d\n", signo);
		}
	}
	return;
}

int main_set_signal_handlers()
{
	/* install signal handler */
	if (signal(SIGINT, main_sig_handler)==SIG_ERR) {
		LOG(L_ERR,"ERROR:main: cannot install SIGINT signal handler\n");
		goto error;
	}
	if (signal(SIGPIPE, main_sig_handler) == SIG_ERR ) {
		LOG(L_ERR,"ERROR:main: cannot install SIGPIPE signal handler\n");
		goto error;
	}
	if (signal(SIGUSR1, main_sig_handler)  == SIG_ERR ) {
		LOG(L_ERR,"ERROR:main: cannot install SIGUSR1 signal handler\n");
		goto error;
	}
	if (signal(SIGTERM , main_sig_handler)  == SIG_ERR ) {
		LOG(L_ERR,"ERROR:main: cannot install SIGTERM signal handler\n");
		goto error;
	}
	if (signal(SIGHUP , main_sig_handler)  == SIG_ERR ) {
		LOG(L_ERR,"ERROR:main: cannot install SIGHUP signal handler\n");
		goto error;
	}
	if (signal(SIGUSR2 , main_sig_handler)  == SIG_ERR ) {
		LOG(L_ERR,"ERROR:main: cannot install SIGUSR2 signal handler\n");
		goto error;
	}	
	
	return 1;
error:
	return 0;
}

/* Configuration */

char *CDiameterPeer_config=0;
extern int debug;


int main(int argc,char* argv[])
{
	LOG(L_NOTICE,"Starting main !\n");
	if (argc<3){
		LOG(L_CRIT,"Debug level and/or Configuration file was not provided as parameter!\n"); 
		LOG(L_INFO,"Usage: main <debug> <cfg_filename> \n\n");
		return -1;
	}
	debug=atoi(argv[1]);
	CDiameterPeer_config = argv[2];
	init_memory(0);
				
	main_set_signal_handlers();

	if (!diameter_peer_init(CDiameterPeer_config)){
		LOG(L_CRIT,"CRITICAL:Error on diameter_peer_init\n");
		return -1;
	}
	
	cb_add(process_incoming,0);

	if (!diameter_peer_start(1)){
		LOG(L_CRIT,"CRITICAL:Error on diameter_peer_start\n");
		return -1;
	}
	//sleep(30);
	diameter_peer_destroy();
	
	if (debug>1) destroy_memory(1);
	else destroy_memory(0);

	return 0;	
}



