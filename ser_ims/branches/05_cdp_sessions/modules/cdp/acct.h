#ifndef __DIAMETER_BASE_ACCT_H
#define __DIAMETER_BASE_ACCT_H

/*
 * acct.h, acct.c provide the accounting portion of Diameter based 
 * protocol.
 * 
 * \author Shengyao Chen shc -at- fokus dot fraunhofer dot de
 * \author Joao Filipe Placido joao-f-placido -at- ptinovacao dot pt
 */

#include "cdp_load.h"
#include "../../locking.h"

/* Command code used in the accounting portion of Diameter base protocol. */
#define ACR 	271
#define ACA 	271



/** Accounting states definition */
typedef enum {
	ACC_ST_IDLE			= 0,	/**< Idle */
	ACC_ST_PENDING_S	= 1,	/**< Pending Session */
	ACC_ST_PENDING_E	= 2,	/**< Pending Event */
	ACC_ST_PENDING_B	= 3,	/**< Pending Buffered */
	ACC_ST_OPEN	  		= 4,	/**< Open */
	ACC_ST_PENDING_I	= 5,	/**< Pending Interim */
	ACC_ST_PENDING_L	= 6		/**< PendingL - sent accounting stop */
} acc_state_t;


/** Accounting events definition */
typedef enum {
	ACC_EV_START				= 101,	/**< Client or device "requests access" (SIP session establishment) */
	ACC_EV_EVENT				= 102,	/**< Client or device requests a one-time service (e.g. SIP MESSAGE) */
	ACC_EV_BUFFEREDSTART		= 103,	/**< Records in storage */
	ACC_EV_RCV_SUC_ACA_START	= 104,	/**< Successful accounting start answer received */
	ACC_EV_SNDFAIL				= 105,	/**< Failure to send */
	ACC_EV_RCV_FAILED_ACA_START	= 106,	/**< Failed accounting start answer received */
	ACC_EV_STOP					= 107,	/**< User service terminated */
	ACC_EV_INTERIM				= 108,	/**< Interim interval elapses */
	ACC_EV_RCV_SUC_ACA_INTERIM	= 109,	/**< Successful accounting interim answer received */
	ACC_EV_RCV_FAILED_ACA_INTERIM=110,	/**< Failed accounting interim answer received */
	ACC_EV_RCV_SUC_ACA_EVENT	= 111,	/**< Successful accounting event answer received */
	ACC_EV_RCV_FAILED_ACA_EVENT	= 112,	/**< Failed accounting event answer received */
	ACC_EV_RCV_SUC_ACA_STOP		= 113,	/**< Successful accounting stop answer received */
	ACC_EV_RCV_FAILED_ACA_STOP	= 114,	/**< Failed accounting stop answer received */
} acc_event_t;



/** Structure for accounting sessions */
typedef struct _acc_session {
	unsigned int hash;						/**< hash for the accounting session 			*/
	
	str* sID;								/**< session id */
	str* peer_fqdn;							/**< FQDN of peer */
	str* dlgid;       						/**< application-level identifier, combines application session (e.g. SIP dialog) or event with diameter accounting session */
	acc_state_t state;						/**< current state */
	unsigned int acct_record_number; 		/**< number of last accounting record within this session */
	
	struct _acc_session *next;				/**< next accounting session in this hash slot 		*/
	struct _acc_session *prev;				/**< previous accounting session in this hash slot	*/
} acc_session;


/** Structure for an accounting session hash slot */
typedef struct {
	acc_session *head;						/**< first accounting session in this hash slot */
	acc_session *tail;						/**< last accounting session in this hash slot 	*/
	gen_lock_t *lock;					/**< slot lock 									*/	
} acc_session_hash_slot;



inline unsigned int get_acc_session_hash(str* id);

int acc_session_init(int hash_size);

void acc_session_destroy();

inline void s_lock(unsigned int hash);
inline void s_unlock(unsigned int hash);



void free_acc_session(acc_session *s);

//AAAMessage * ACR_create(AAASessionId sessId, unsigned int type, struct cdp_binds* cdpb);

//void test(int x);
#endif /*__DIAMETER_BASE_ACCT_H*/
