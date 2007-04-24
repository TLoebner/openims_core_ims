/*
 * acct.h, acct.c provides the accounting portion of Diameter based 
 * protocol.
 * 
 * \author Shengyao Chen shc -at- fokus dot fraunhofer dot de
 * \author Joao Filipe Placido joao-f-placido -at- ptinovacao dot pt
 */
#include "acct.h"

//extern struct tm_binds tmb;          /**< Structure with pointers to tm funcs 		*/

int acc_sessions_hash_size;						/**< size of the accounting session hash table 		*/
acc_session_hash_slot *acc_sessions=0;			/**< the hash table									*/


/**
 * Computes the hash for a string.
 * @param id - the string to compute for
 * @returns the hash % acc_sessions_hash_size
 */
inline unsigned int get_acc_session_hash(str* id)
{
	if (!id) return 0;
	if (id->len==0) return 0;
#define h_inc h+=v^(v>>3)
	char* p;
	register unsigned v;
	register unsigned h;
  	
	h=0;
	for (p=id->s; p<=(id->s+id->len-4); p+=4){
		v=(*p<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
		h_inc;
	}
	v=0;
	for (;p<(id->s+id->len); p++) {
		v<<=8;
		v+=*p;
	}
	h_inc;
	
	h=((h)+(h>>11))+((h>>13)+(h>>23));
	return (h)%acc_sessions_hash_size;
#undef h_inc 
}

/**
 * Initialize the accounting sessions hash table.
 * @param hash_size - size of the acc_sessions hash table
 * @returns 1 if OK, 0 on error
 */
int acc_sessions_init(int hash_size)
{
	int i;
	
	acc_sessions_hash_size = hash_size;
	acc_sessions = shm_malloc(sizeof(acc_session_hash_slot)*acc_sessions_hash_size);

	if (!acc_sessions) return 0;

	memset(acc_sessions,0,sizeof(acc_session_hash_slot)*acc_sessions_hash_size);
	
	for(i=0;i<acc_sessions_hash_size;i++){
		acc_sessions[i].lock = lock_alloc();
		if (!acc_sessions[i].lock){
			LOG(L_ERR,"ERR:acc_sessions_init(): Error creating lock\n");
			return 0;
		}
		acc_sessions[i].lock = lock_init(acc_sessions[i].lock);
	}
			
	return 1;
}

/**
 * Destroy the accounting sessions hash table.
 */
void acc_sessions_destroy()
{
	int i;
	acc_session *s,*ns;
	for(i=0;i<acc_sessions_hash_size;i++){
		s_lock(i);
			s = acc_sessions[i].head;
			while(s){
				ns = s->next;
				free_acc_session(s);
				s = ns;
			}
		s_unlock(i);
		lock_dealloc(acc_sessions[i].lock);
	}
	shm_free(acc_sessions);
}

/**
 * Locks the required slot of the accounting sessions hash table.
 * @param hash - index of the slot to lock
 */
inline void s_lock(unsigned int hash)
{
//	LOG(L_CRIT,"GET %d\n",hash);
	lock_get(acc_sessions[(hash)].lock);
//	LOG(L_CRIT,"GOT %d\n",hash);	
}

/**
 * UnLocks the required slot of the accounting sessions hash table
 * @param hash - index of the slot to unlock
 */
inline void s_unlock(unsigned int hash)
{
	lock_release(acc_sessions[(hash)].lock);
//	LOG(L_CRIT,"RELEASED %d\n",hash);	
}



/**
 * Finds and returns an accounting session from the hash table.
 * \note Locks the hash slot if ok! Call s_unlock(acc_session->hash) when you are finished)
 * @param dlgid - the app-level id (e.g. SIP dialog)
 * @returns the acc_session* or NULL if not found
 */
acc_session* get_acc_session(str* dlgid)
{
	acc_session *s=0;
	unsigned int hash = get_acc_session_hash(dlgid);

	s_lock(hash);
		s = acc_sessions[hash].head;
		while(s){
			if (s->dlgid->len == dlgid->len &&
				strncasecmp(s->dlgid->s,dlgid->s,dlgid->len)==0) {
					return s;
				}
			s = s->next;
		}
	s_unlock(hash);
	return 0;
}





/**
 * Creates a new acc_session structure.
 * Does not add the structure to the list
 * @param dlgid - application-level id
 * @returns the new acc_session* or NULL acc_sesssion
 */
acc_session* new_acc_session(str* dlgid)
{
	acc_session *s;
	
	s = shm_malloc(sizeof(acc_session));
	if (!s) {
		LOG(L_ERR,"ERR:new_acc_session(): Unable to alloc %d bytes\n",
			sizeof(acc_session));
		goto error;
	}
	memset(s,0,sizeof(acc_session));
	
	s->hash = get_acc_session_hash(dlgid);		
	//STR_SHM_DUP(d->call_id,call_id,"shm");
	//STR_SHM_DUP(d->aor,aor,"shm");	
	return s;
error:
	if (s){
		shm_free(s);		
	}
	return 0;
}


/**
 * Creates and adds an accounting session to the hash table.
 * \note Locks the hash slot if OK! Call s_unlock(acc_session->hash) when you are finished)
 * @param s - acc_session to add
 * @returns the new acc_session* or NULL acc_session
 */
acc_session* add_acc_session(acc_session* s)
{
	s_lock(s->hash);
		s->next = 0;
		s->prev = acc_sessions[s->hash].tail;
		if (s->prev) s->prev->next = s;
		acc_sessions[s->hash].tail = s;
		if (!acc_sessions[s->hash].head) acc_sessions[s->hash].head = s;

		return s;
}




/**
 * Frees an accounting session.
 * @param s - the accounting session to free
 */
void free_acc_session(acc_session *s)
{
	if (!s) return;
	if (s->sID->s) shm_free(s->sID->s);	
	shm_free(s);
}




/****************************** API FUNCTIONS ********************************/
/**
 * Creates an acc_session.
 * @param peer - accounting server FQDN.
 * @param dlgid - application-level id of accountable event or session
 * @returns the new acc_session
 */
acc_session* AAACreateAccSession(str* peer, str* dlgid)
{
	acc_session* s = NULL;
	
	LOG(L_INFO, "INF: AAACreateAccSession\n");
		
	s = new_acc_session(dlgid);
	if (!s) return 0;		
	 
	//s = get_acc_session(dlgid);
	//if(s) goto error;
	
	s->state = ACC_ST_IDLE;

	s->peer_fqdn = shm_malloc(sizeof(str));
	s->peer_fqdn->len = peer->len;
	s->peer_fqdn->s = peer->s;

	s->dlgid = shm_malloc(sizeof(str));
	s->dlgid->len = dlgid->len;
	s->dlgid->s = dlgid->s;
	LOG(L_INFO, "s->dlgid: %.*s\n", s->dlgid->len, s->dlgid->s);
	
	s->sID = shm_malloc(sizeof(str));
	s->sID->len = 0;
	s->sID->s = 0;
	/* generates a new session-ID */
	if (generate_sessionID( s->sID, 0 )!=1) goto error;
	//LOG(L_INFO, "s->sID: %.*s\n", s->sID->len, s->sID->s);
	
	s->prev = NULL;
	s->next = NULL;
	
	// TODO: include pointer to sm_process function in acc_session
	//		to allow accounting client SM and server SM?
	/*
	if () {
		s->sm_process = acc_cli_sm_process;
	} else {
		s->sm_process = acc_serv_sm_process;
	}*/
		
	add_acc_session(s);
	
	return s;

error:
	if (s) 
		LOG(L_ERR, "ERR: AAACreateAccSession: AAAAccSession exists\n");
	else 	
		LOG(L_ERR, "ERR: AAACreateAccSession: Error on new AAAAccSession \
			generation\n");
	
	return NULL;
}


/**
 * Get an acc_session based on the application-level id.
 * @param dlgid app-level id 
 *  
 */
acc_session* AAAGetAccSession(str *dlgid) 
{
	acc_session* s;
	
	LOG(L_INFO, "INF: AAAGetAccSession\n");
	s = get_acc_session(dlgid);
	if (!s) LOG(L_ERR, "ERR: AAAGetAccSession: acc_session does not exist\n");
	
	return s;
}





//static inline int acct_add_avp(AAAMessage *m,char *d,int len,int avp_code,
//	int flags,int vendorid,int data_do,const char *func)
//{
//	AAA_AVP *avp;
//	if (vendorid!=0) flags |= AAA_AVP_FLAG_VENDOR_SPECIFIC;
//	avp = cdpb.AAACreateAVP(avp_code,flags,vendorid,d,len,data_do);
//	if (!avp) {
//		//LOG(L_ERR,"ERR:"M_NAME":%s: Failed creating avp\n",func);
//		return 0;
//	}
//	if (cdpb.AAAAddAVPToMessage(m,avp,m->avpList.tail)!=AAA_ERR_SUCCESS) {
//		//LOG(L_ERR,"ERR:"M_NAME":%s: Failed adding avp to message\n",func);
//		cdpb.AAAFreeAVP(&avp);
//		return 0;
//	}
//	return 1;
//}
//
///* add Accounting-Record-Type AVP */
//int acct_add_accounting_record_type(AAAMessage* msg, unsigned int data)
//{
//	char x[4];
//	set_4bytes(x,data);
//	
//	return
//	acct_add_avp(msg, x, 4,
//		AVP_Accounting_Record_Type,
//		AAA_AVP_FLAG_MANDATORY,
//		0,
//		AVP_DUPLICATE_DATA,
//		__FUNCTION__);
//}
//
///* add Accounting-Record-Number AVP */
//int acct_add_accounting_record_number(AAAMessage* msg, unsigned int data)
//{
//	char x[4];
//	set_4bytes(x,data);
//	
//	return
//	acct_add_avp(msg, x, 4,
//		AVP_Accounting_Record_Number,
//		AAA_AVP_FLAG_MANDATORY,
//		0,
//		AVP_DUPLICATE_DATA,
//		__FUNCTION__);
//}

/**
 * Create an ACR based on sessionId and accounting type. 
 * 
 * @param type 	- accounting type: start, interim, stop, event  
 * @returns the created ACR message, if error return NULL
 */
//AAAMessage* ACR_create(AAASessionId sessId, unsigned int type, struct cdp_binds* cdpb)
//{
//	AAAMessage* acr = NULL;
//	
//	AAASessionId sid = {0,0};
//	/*
//	 * if type == event 
//	 */
//	if (type == AAA_ACCT_EVENT) {
//	 	sessId = cdpb->AAACreateSession();
//		//acr = cdpb.AAACreateRequest(IMS_Rf, ACR, Flag_Proxyable, &sessId);
//		/* orgin host realm, dest host realm are added by peer */ 
//		//if (!acct_add_accounting_record_type(acr, type)) goto error;
//		//if (!acct_add_accounting_record_number(acr, 0)) goto error;
//	 }
//	 return acr;
//
//	/* if type == start, create a new accounting session. */
//	/* if type == interim, sid cannot be NULL, new ACR is created based
//	 * 		      on this session.
//	 * if type == stop, sid cannot be NULL, new ACR is created based on 
//	 * 			  this session, and the session is closed. 
//	 */
//	
//	
//
//	// TODO not forget to call this line: cdpb.AAADropSession(&sessId);
//error: return NULL; 	
//}





