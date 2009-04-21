/**
 * \file
 * 
 * Emergency-CSCF - Dialog State Operations
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *  \author Alberto Diez - Changes to handle release_call
 *  \author Ancuta Onofrei ancuta_onofrei@yahoo.com	
 */
 
#include <time.h>

#include "dlg_state.h"
#include "../tm/tm_load.h"
#include "../sl/sl_funcs.h"
#include "../../mem/shm_mem.h"
#include "../../parser/parse_rr.h"

#include "sip.h"
//#include "release_call.h"

extern struct tm_binds tmb;

int e_dialogs_hash_size;						/**< size of the dialog hash table 					*/
e_dialog_hash_slot *e_dialogs=0;				/**< the hash table									*/
extern int ecscf_dialogs_expiration_time;		/**< default expiration time for dialogs			*/
extern int ecscf_dialogs_enable_release;	/**< if to enable dialog release		*/

extern str ecscf_name_str;
extern str ecscf_record_route_mo;				/**< the record route header for Mobile Originating */
extern str ecscf_record_route_mt;				/**< the record route header for Mobile Terminating */
extern str ecscf_record_route_mo_uri;			/**< just the record route uri for Mobile Originating */
extern str ecscf_record_route_mt_uri;			/**< just the record route uri for Mobile Terminating */

time_t d_time_now;								/**< dialogs current time							*/

extern int ecscf_min_se;

int (*sl_reply)(struct sip_msg* _msg, char* _str1, char* _str2); 
int supports_extension(struct sip_msg *m, str *extension);
int requires_extension(struct sip_msg *m, str *extension);


#define strtotime(src,dest) \
{\
	int i;\
	(dest)=0;\
	for(i=0;i<(src).len;i++)\
		if ((src).s[i]>='0' && (src).s[i]<='9')\
			(dest) = (dest)*10 + (src).s[i] -'0';\
}

#define FParam_INT(val) { \
	 .v = { \
		.i = val \
	 },\
	.type = FPARAM_INT, \
	.orig = "int_value", \
};

#define FParam_STRING(val) { \
	.v = { \
		.str = STR_STATIC_INIT(val) \
	},\
	.type = FPARAM_STR, \
	.orig = val, \
};


/**
 * Computes the hash for a string.
 * @param call_id - the string to compute for
 * @returns the hash % ecscf_dialogs_hash_size
 */
inline unsigned int get_e_dialog_hash(str call_id)
{
	if (call_id.len==0) return 0;
#define h_inc h+=v^(v>>3)
	char* p;
	register unsigned v;
	register unsigned h;
  	
	h=0;
	for (p=call_id.s; p<=(call_id.s+call_id.len-4); p+=4){
		v=(*p<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
		h_inc;
	}
	v=0;
	for (;p<(call_id.s+call_id.len); p++) {
		v<<=8;
		v+=*p;
	}
	h_inc;
	
	h=((h)+(h>>11))+((h>>13)+(h>>23));
	return (h)%e_dialogs_hash_size;
#undef h_inc 
}

/**
 * Initialize the E-CSCF dialogs hash table.
 * @param hash_size - size of the dialog hash table
 * @returns 1 if OK, 0 on error
 */
int e_dialogs_init(int hash_size)
{
	int i;
	
	e_dialogs_hash_size = hash_size;
	e_dialogs = shm_malloc(sizeof(e_dialog_hash_slot)*e_dialogs_hash_size);

	if (!e_dialogs) return 0;

	memset(e_dialogs,0,sizeof(e_dialog_hash_slot)*e_dialogs_hash_size);
	
	for(i=0;i<e_dialogs_hash_size;i++){
		e_dialogs[i].lock = lock_alloc();
		if (!e_dialogs[i].lock){
			LOG(L_ERR,"ERR:"M_NAME":d_hash_table_init(): Error creating lock\n");
			return 0;
		}
		e_dialogs[i].lock = lock_init(e_dialogs[i].lock);
	}
			
	return 1;
}

/**
 * Destroy the hash table
 */
void e_dialogs_destroy()
{
	int i;
	e_dialog *d,*nd;
	for(i=0;i<e_dialogs_hash_size;i++){
		d_lock(i);
			d = e_dialogs[i].head;
			while(d){
				nd = d->next;
				free_e_dialog(d);
				d = nd;
			}
		d_unlock(i);
		lock_dealloc(e_dialogs[i].lock);
	}
	shm_free(e_dialogs);
}

/**
 * Locks the required slot of the dialog hash table.
 * @param hash - index of the slot to lock
 */
inline void d_lock(unsigned int hash)
{
//	LOG(L_CRIT,"GET %d\n",hash);
	lock_get(e_dialogs[(hash)].lock);
//	LOG(L_CRIT,"GOT %d\n",hash);	
}

/**
 * UnLocks the required slot of the dialog hash table
 * @param hash - index of the slot to unlock
 */
inline void d_unlock(unsigned int hash)
{
	lock_release(e_dialogs[(hash)].lock);
//	LOG(L_CRIT,"RELEASED %d\n",hash);	
}

/**
 * Refresh the current dialog time.
 * @returns the current time
 */
inline int d_act_time()
{
	d_time_now=time(0);
	return d_time_now;
}

extern int* ecscf_dialog_count;
extern int ecscf_max_dialog_count;
extern gen_lock_t* ecscf_dialog_count_lock;

/**
 * Locks the dialog counter variable
 */
inline void e_dialog_count_lock()
{
	lock_get(ecscf_dialog_count_lock);
}

/**
 * UnLocks the dialog counter variable
 */
inline void e_dialog_count_unlock()
{
        lock_release(ecscf_dialog_count_lock);
}


/**
 * Try to increment the dialog count
 * @returns 1 on success or 0 if the total number of dialogs is already reached
 */
inline int e_dialog_count_increment ()
{
    if (ecscf_max_dialog_count<0) return 1;
    e_dialog_count_lock();	
	if (*ecscf_dialog_count<ecscf_max_dialog_count){
    	(*ecscf_dialog_count)++;
    	e_dialog_count_unlock();
    	return 1;
	} else {
    	e_dialog_count_unlock();
    	return 0;
	}
	LOG(L_DBG,"DBG:"M_NAME":e_dialog_count_increment(): E-CSCF Dialog counter value is %d\n", *ecscf_dialog_count);
}

/**
 * Decrement the dialog count
 */
inline void e_dialog_count_decrement()
{
    if (ecscf_max_dialog_count<0) return ;
    e_dialog_count_lock();
    (*ecscf_dialog_count)--;
    e_dialog_count_unlock();
	LOG(L_DBG,"DBG:"M_NAME":e_dialog_count_decrement(): E-CSCF Dialog counter value is %d\n", *ecscf_dialog_count);    
}


/**
 * Creates a new e_dialog structure.
 * Does not add the structure to the list
 * @param call_id - call_id of the dialog
 * @param aor - aor of the user
 * @param dir - the direction
 * @returns the new e_dialog* or NULL e_dialog
 */
e_dialog* new_e_dialog(str call_id,str aor, enum e_dialog_direction dir)
{
	e_dialog *d;
	
	if (!e_dialog_count_increment()) return 0;
	d = shm_malloc(sizeof(e_dialog));
	if (!d) {
		LOG(L_ERR,"ERR:"M_NAME":new_e_dialog(): Unable to alloc %d bytes\n",
			sizeof(e_dialog));
		goto error;
	}
	memset(d,0,sizeof(e_dialog));
	
	d->hash = get_e_dialog_hash(call_id);		
	STR_SHM_DUP(d->call_id,call_id,"shm");
	STR_SHM_DUP(d->aor,aor,"shm");	
	d->direction = dir;
	d->is_releasing = 0;
	return d;
error:
out_of_memory:
	if (d){
		shm_free(d);		
	}
	e_dialog_count_decrement();
	return 0;
}

/**
 * Creates and adds a dialog to the hash table.
 * \note Locks the hash slot if OK! Call d_unlock(e_dialog->hash) when you are finished)
 * \note make sure that is_e_dialog(call_id) returns 0 or there will be unreachable duplicates!
 * @param call_id - call_id of the dialog
 * @param aor - aor of the user
 * @param dir - the direction
 * @returns the new e_dialog* or NULL e_dialog
 */
e_dialog* add_e_dialog(str call_id,str aor,enum e_dialog_direction dir)
{
	e_dialog *d;
	
	d = new_e_dialog(call_id,aor,dir);
	if (!d) return 0;		
	
	d_lock(d->hash);
		d->next = 0;
		d->prev = e_dialogs[d->hash].tail;
		if (d->prev) d->prev->next = d;
		e_dialogs[d->hash].tail = d;
		if (!e_dialogs[d->hash].head) e_dialogs[d->hash].head = d;

		return d;
}

/**
 * Finds out if a dialog is in the hash table.
 * @param call_id - call_id of the dialog
 * @param aor - aor of the user
 * @returns 1 if found, 0 if not found
 */
int is_e_dialog(str call_id,str aor,enum e_dialog_direction dir)
{
	e_dialog *d=0;
	unsigned int hash = get_e_dialog_hash(call_id);

	d_lock(hash);
		d = e_dialogs[hash].head;
		while(d){
				if (d->direction == dir &&
					d->aor.len == aor.len &&
					d->call_id.len == call_id.len &&
					strncasecmp(d->aor.s,aor.s,aor.len)==0 &&
					strncasecmp(d->call_id.s,call_id.s,call_id.len)==0) {
					d_unlock(hash);
					return 1;
				}
			d = d->next;
		}
	d_unlock(hash);
	return 0;
}

/**
 * Finds out if a dialog is in the hash table.
 * @param call_id - call_id of the dialog
 * @param dir - the direction
 * @returns 1 if found, 0 if not found
 */
int is_e_dialog_dir(str call_id,enum e_dialog_direction dir)
{
	e_dialog *d=0;
	unsigned int hash = get_e_dialog_hash(call_id);

	d_lock(hash);
		d = e_dialogs[hash].head;
		while(d){
				if (d->direction == dir &&
				d->call_id.len == call_id.len &&
				strncasecmp(d->call_id.s,call_id.s,call_id.len)==0) {
					d_unlock(hash);
					return 1;
				}
			d = d->next;
		}
	d_unlock(hash);
	return 0;
}

/**
 * Finds and returns a dialog from the hash table.
 * \note Locks the hash slot if ok! Call d_unlock(e_dialog->hash) when you are finished)
 * @param call_id - call_id of the dialog
 * @param aor - aor of the user
 * @returns the e_dialog* or NULL if not found
 */
e_dialog* get_e_dialog(str call_id,str aor)
{
	e_dialog *d=0;
	unsigned int hash = get_e_dialog_hash(call_id);

	d_lock(hash);
		d = e_dialogs[hash].head;
		while(d){
			if (d->aor.len == aor.len &&
				d->call_id.len == call_id.len &&
				strncasecmp(d->aor.s,aor.s,aor.len)==0 &&
				strncasecmp(d->call_id.s,call_id.s,call_id.len)==0) {
					return d;
				}
			d = d->next;
		}
	d_unlock(hash);
	return 0;
}

/**
 * Finds and returns a dialog from the hash table.
 * \note Locks the hash slot if ok! Call d_unlock(e_dialog->hash) when you are finished)
 * @param call_id - call_id of the dialog
 * @param dir - the direction
 * @returns the e_dialog* or NULL if not found
 */
e_dialog* get_e_dialog_dir(str call_id,enum e_dialog_direction dir)
{
	e_dialog *d=0;
	unsigned int hash = get_e_dialog_hash(call_id);

	d_lock(hash);
		d = e_dialogs[hash].head;
		while(d){
			if (d->direction == dir &&
				d->call_id.len == call_id.len &&
				strncasecmp(d->call_id.s,call_id.s,call_id.len)==0) {
					return d;
				}
			d = d->next;
		}
	d_unlock(hash);
	return 0;
}

/**
 * Finds and returns a dialog from the hash table.
 * \note the table should be locked already for the call_id in the parameter
 * @param call_id - call_id of the dialog
 * @param dir - the direction
 * @returns the e_dialog* or NULL if not found
 */
e_dialog* get_e_dialog_dir_nolock(str call_id,enum e_dialog_direction dir)
{
	e_dialog *d=0;
	unsigned int hash = get_e_dialog_hash(call_id);

	d = e_dialogs[hash].head;
	while(d){
		if (d->direction == dir &&
			d->call_id.len == call_id.len &&
			strncasecmp(d->call_id.s,call_id.s,call_id.len)==0) {
				return d;
			}
		d = d->next;
	}
	return 0;
}

//static str Reason={"Reason: SIP ;cause=503 ;text=\"Session Time-out on E-CSCF\"\r\n",59};
/** 
 * Terminates a dialog - called before del_e_dialog to send out terminatination messages.
 * @param d - the dialog to terminate
 * @returns - 1 if the requests were sent and the dialog will be deleted, 0 on error (you will have to delete the
 * dialog yourself!) 
 */
int terminate_e_dialog(e_dialog *d)
{
	if (!ecscf_dialogs_enable_release) return 0;	
	switch (d->method){
		case DLG_METHOD_INVITE:
			//if (release_call_s(d,Reason)<=0){
				//dialog has expired and not confirmed
				// or error releasing dialog
				del_e_dialog(d);
			//}
			return 1;
			break;
		case DLG_METHOD_SUBSCRIBE:
			//if (!release_subscription(d)){
				//error releasing the subscription - just drop silently
				del_e_dialog(d);
			//}
			return 1;
			break;
		default:
			LOG(L_ERR,"ERR:"M_NAME":terminate_e_dialog(): Not implemented yet for method[%d]!\n",d->method);
			return 0;
	}
}

/**
 * Deletes a dialog from the hash table
 * \note Must be called with a lock on the dialogs slot
 * @param d - the dialog to delete
 */
void del_e_dialog(e_dialog *d)
{
	LOG(L_INFO,"DBG:"M_NAME":del_e_dialog(): Deleting dialog <%.*s> DIR[%d]\n",d->call_id.len,d->call_id.s,d->direction);
	if (d->prev) d->prev->next = d->next;
	else e_dialogs[d->hash].head = d->next;
	if (d->next) d->next->prev = d->prev;
	else e_dialogs[d->hash].tail = d->prev;
	free_e_dialog(d);
}


/**
 * Frees a dialog.
 * @param d - the dialog to delete
 */
void free_e_dialog(e_dialog *d)
{
	if (!d) return;
	if (d->call_id.s) shm_free(d->call_id.s);
	if (d->aor.s) shm_free(d->aor.s);	
	if (d->method_str.s) shm_free(d->method_str.s);
	if (d->dialog_s) tmb.free_dlg(d->dialog_s);
	if (d->dialog_c) tmb.free_dlg(d->dialog_c);
	if (d->refresher.s) shm_free(d->refresher.s); 		
	if (d->event.s) shm_free(d->event.s); 		
	shm_free(d);
	e_dialog_count_decrement(); 	
}

/**
 * Prints the list of dialogs.
 * @param log_level - level to log at
 */
void print_e_dialogs(int log_level)
{
	e_dialog *d;
	int i;
	if (debug<log_level) return; /* to avoid useless calls when nothing will be printed */
	d_act_time();
	LOG(log_level,"INF:"M_NAME":----------  E-CSCF Dialog List begin --------------\n");
	for(i=0;i<e_dialogs_hash_size;i++){
		d_lock(i);
			d = e_dialogs[i].head;
			while(d){
				LOG(log_level,"INF:"M_NAME":[%4d] Dir["ANSI_MAGENTA"%d"ANSI_GREEN
					"] Call-ID:<"ANSI_BLUE"%.*s"ANSI_GREEN
					"> AOR:<"ANSI_RED"%.*s"ANSI_GREEN
					">\n",i,				
					d->direction,
					d->call_id.len,d->call_id.s,
					d->aor.len,d->aor.s);
				LOG(log_level,"INF:"M_NAME":\t\tMethod:["ANSI_MAGENTA"%d"ANSI_GREEN
					"] State:["ANSI_MAGENTA"%d"ANSI_GREEN
					"] Exp:["ANSI_MAGENTA"%4d"ANSI_GREEN"] Ref:["ANSI_MAGENTA"%.*s"ANSI_GREEN"] Event:["ANSI_MAGENTA"%.*s"ANSI_GREEN"]\n",				
					d->method,
					d->state,
					(int)(d->expires - d_time_now),
					d->refresher.len,d->refresher.s,
					d->event.len,d->event.s);
					
				d = d->next;
			} 		
		d_unlock(i);
	}
	LOG(log_level,"INF:"M_NAME":----------  E-CSCF Dialog List end   --------------\n");	
}



/**
 * Returns the e_dialog_direction from the direction string.
 * @param direction - "orig" or "term"
 * @returns the e_dialog_direction if ok or #DLG_MOBILE_UNKNOWN if not found
 */
enum e_dialog_direction get_dialog_direction(char *direction)
{
	switch(direction[0]){
		case 'o':
		case 'O':
		case '0':
			return DLG_MOBILE_ORIGINATING;
		case 't':
		case 'T':
		case '1':
			return DLG_MOBILE_TERMINATING;
		default:
			LOG(L_CRIT,"ERR:"M_NAME":get_dialog_direction(): Unknown direction %s",direction);
			return DLG_MOBILE_UNKNOWN;
	}
}
/**
 * Finds the AOR for a dialog
 * @param msg - the SIP message to add to
 * @param d - the dialog direction
 * @param aor - aor to fill
 * @returns 1 if found, 0 if not
 */
static inline int find_dialog_aor(struct sip_msg *msg,enum e_dialog_direction d,str *aor)
{
	if (msg->first_line.type!=SIP_REQUEST)
		return 0;
	switch(d){
		case DLG_MOBILE_ORIGINATING:
			*aor = cscf_get_asserted_identity(msg); 
			if (!aor->len) return cscf_get_from_uri(msg, aor);
			break;
		case DLG_MOBILE_TERMINATING:
			*aor = cscf_get_called_party_id(msg,0);	
			if (!aor->len) *aor = cscf_get_identity_from_ruri(msg);
			if (!aor->len) return 0;
			return 1;
			break;
		default:
			LOG(L_CRIT,"ERR:"M_NAME":find_dialog_aor(): Unknown direction %d",d);
			return 0;
	}
	return 1;
}
/**
 * Find the dialog direction from the first Route header.
 * @param msg - the SIP message
 */
static inline enum e_dialog_direction find_dialog_route_dir(struct sip_msg *msg)
{
	str r;	
	r = cscf_get_first_route(msg,0);
	
	LOG(L_DBG,"DBG:"M_NAME":find_dialog_route_dir(): Route <%.*s>\n",r.len,r.s);

	LOG(L_DBG, "DBG:"M_NAME":find_dialog_route_dir(): rr_mo %.*s rr_mt %.*s: \n",
			ecscf_record_route_mo_uri.len,ecscf_record_route_mo_uri.s,
			ecscf_record_route_mt_uri.len,ecscf_record_route_mt_uri.s);
	
	if (!r.len) return DLG_MOBILE_UNKNOWN;
	
	if (r.len >= ecscf_record_route_mo_uri.len &&
		strncasecmp(r.s,ecscf_record_route_mo_uri.s,ecscf_record_route_mo_uri.len)==0){
		LOG(L_DBG, "DBG:"M_NAME": mobile originating\n");
		return DLG_MOBILE_ORIGINATING;
	}
	if (r.len >= ecscf_record_route_mt_uri.len &&
		strncasecmp(r.s,ecscf_record_route_mt_uri.s,ecscf_record_route_mt_uri.len)==0){
		LOG(L_DBG, "DBG:"M_NAME": mobile terminating\n");
		return DLG_MOBILE_TERMINATING;
	}
	return DLG_MOBILE_UNKNOWN;
}

/**
 * Find out if a message is within a saved dialog.
 * @param msg - the SIP message
 * @param str1 - the direction of the dialog ("orig"/"term")
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if in, #CSCF_RETURN_FALSE else or #CSCF_RETURN_BREAK on error
 */
int E_is_in_dialog(struct sip_msg* msg, char* str1, char* str2)
{
	str call_id;
	enum e_dialog_direction dir = get_dialog_direction(str1);
	enum e_dialog_direction dirmsg = find_dialog_route_dir(msg);
	

//	LOG(L_CRIT,"%d - %d\n",dir,dirmsg);
//	TODO: dirmsg used ot not?
//	if (dir!=dirmsg) return CSCF_RETURN_FALSE;				
			
//	print_e_dialogs(L_ERR);
	call_id = cscf_get_call_id(msg,0);
	if (!call_id.len){
		
		return CSCF_RETURN_FALSE;
	}
	
	if (is_e_dialog_dir(call_id,dir))
		return CSCF_RETURN_TRUE;
	else {
		LOG(L_ERR, "ERR:"M_NAME":E_is_in_dialog: could not find the dialog "
				"with the callid %.*s and dir %s\n", call_id.len, call_id.s, str1);
		return CSCF_RETURN_FALSE;
	}
}


str s_OTHER={"<OTHER>",7};
str s_INVITE={"INVITE",6};
str s_SUBSCRIBE={"SUBSCRIBE",9};
/**
 * Return e_dialog_method for a method string.
 * @param method - the string containing the method
 * @returns the e_dialog_method corresponding if known or #DLG_METHOD_OTHER if not
 */
static enum e_dialog_method get_dialog_method(str method)
{
	if (method.len == s_INVITE.len &&
		strncasecmp(method.s,s_INVITE.s,s_INVITE.len)==0) return DLG_METHOD_INVITE;
	if (method.len == s_SUBSCRIBE.len &&
		strncasecmp(method.s,s_SUBSCRIBE.s,s_SUBSCRIBE.len)==0) return DLG_METHOD_SUBSCRIBE;
	return DLG_METHOD_OTHER;
}

#ifdef WITH_IMS_PM
/** 
 * Returns the Method string give the method enum id
 * @param method - the enum id
 * @returns the string method
 */	
static str get_dialog_method_str(enum e_dialog_method method)
{
	switch(method){
		case DLG_METHOD_INVITE:
			return s_INVITE;
		case DLG_METHOD_SUBSCRIBE:
			return s_SUBSCRIBE;
		default:
			return s_OTHER;
	}	
}	
#endif

static fparam_t fp_422 = FParam_INT(422);
static fparam_t fp_se_small = FParam_STRING("Session Interval Too Small");

/**
 * Send a 422 Session Interval Too Small.
 * @param msg - the msg to respond to
 * @param str1 - not used
 * @param str2 - not used
 * @returns
 */
int E_422_session_expires(struct sip_msg* msg, char* str1, char* str2)
{
	str hdr = {pkg_malloc(32), 0};

	if (!hdr.s) {
		LOG(L_ERR, "ERR:"M_NAME":E_422_session_expires(): no memory for hdr\n");
		goto error;
	}

	hdr.len = snprintf(hdr.s, 31, "Min-SE: %d\r\n", ecscf_min_se);

	if (!cscf_add_header_rpl(msg, &hdr)) {
		LOG(L_ERR, "ERR:"M_NAME":E_422_session_expires(): Can't add header\n");
		goto error;
 	}
	
	return sl_reply(msg, (char *)&fp_422, (char *)&fp_se_small);

error:
	if (hdr.s) pkg_free(hdr.s);
	return CSCF_RETURN_FALSE;
}


static str s_refresher = {"refresher=", 10};
static str str_ext_timer = {"timer", 5};
static str str_min_se = {"Min-SE:",7};
static str str_se = {"Session-Expires:",16}; 
static str str_require = {"Require:",8}; 


/**
 * Checks if Session-Expires value is over Min_SE local policy
 * @param msg - the initial request
 * @param str1 - not used
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not
*/
int E_check_session_expires(struct sip_msg* msg, char* str1, char* str2)
{
	time_t t_time;
	time_t min_se_time = 0;
	str ses_exp = {0,0};
 	str min_se = {0,0};
	str new_min_se = {0,0};
	str new_ses_exp = {0,0};
	struct hdr_field *h_se, *h_min_se;
	str refresher;

	ses_exp = cscf_get_session_expires_body(msg, &h_se);
	t_time = cscf_get_session_expires(ses_exp, &refresher);
	
	if (!t_time || t_time >= ecscf_min_se)
		return CSCF_RETURN_TRUE;
	if (!supports_extension(msg, &str_ext_timer)) //does not suports timer extension
	{
		//add Min-SE header with its minimum interval
		min_se = cscf_get_min_se(msg, &h_min_se);
		if (min_se.len) {
			strtotime(min_se, min_se_time);
			if (min_se_time < ecscf_min_se)
				cscf_del_header(msg, h_min_se);
			else
				return CSCF_RETURN_TRUE;
		}
		new_min_se.len = 11/*int value*/ + str_min_se.len+3;
		new_min_se.s = pkg_malloc(new_min_se.len+1);
		if (!new_min_se.s) {
			LOG(L_ERR,"ERR:"M_NAME":E_check_session_expires: Error allocating %d bytes\n",new_min_se.len);
			goto error;
		}
		new_min_se.len = snprintf(new_min_se.s, new_min_se.len, "%.*s %d\r\n",str_min_se.len, str_min_se.s, ecscf_min_se);
		min_se_time = ecscf_min_se;
		cscf_add_header(msg, &new_min_se, HDR_OTHER_T);
		if (t_time < ecscf_min_se) {
			cscf_del_header(msg, h_se);
			new_ses_exp.len = 11 + str_se.len+3;
			new_ses_exp.s = pkg_malloc(new_ses_exp.len+1);
			if (!new_ses_exp.s) {
				LOG(L_ERR,"ERR:"M_NAME":E_check_session_expires: Error allocating %d bytes\n",new_ses_exp.len);
				goto error;
			}
			new_ses_exp.len = snprintf(new_ses_exp.s, new_ses_exp.len, "%.*s %d\r\n",str_se.len, str_se.s, ecscf_min_se);
			t_time = ecscf_min_se;
			cscf_add_header(msg, &new_ses_exp, HDR_OTHER_T);
		}
		return CSCF_RETURN_TRUE;
	}
error:
	if (new_min_se.s) pkg_free(new_min_se.s);
	if (new_ses_exp.s) pkg_free(new_ses_exp.s);
	return CSCF_RETURN_FALSE;
}		


/**
 * Saves a dialog.
 * @param msg - the initial request
 * @param str1 - direction - "orig" or "term"
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_BREAK on error 
 */
int E_save_dialog(struct sip_msg* msg, char* str1, char* str2)
{
	str call_id;
	e_dialog *d;
	str aor;
	char buf1[256],buf2[256];
	str uri,tag,ruri,x;	
	time_t t_time;
	str ses_exp = {0,0};
	str refresher = {0,0};
	str event = {0,0};
	struct hdr_field *h;
	unsigned int hash;
	
	enum e_dialog_direction dir = get_dialog_direction(str1);
	
	if (!find_dialog_aor(msg,dir,&aor)){
		LOG(L_ERR,"ERR:"M_NAME":E_save_dialog(): Error retrieving %s contact\n",str1);
		return CSCF_RETURN_BREAK;
	}		
		
	call_id = cscf_get_call_id(msg,0);
	if (!call_id.len)
		return CSCF_RETURN_FALSE;

	LOG(L_INFO,"DBG:"M_NAME":E_save_dialog(%s): Call-ID <%.*s>\n",str1,call_id.len,call_id.s);

	if (is_e_dialog(call_id,aor,dir)){
		LOG(L_ERR,"ERR:"M_NAME":E_save_dialog: dialog already exists!\n");	
		return CSCF_RETURN_TRUE;
	}
	
	d = add_e_dialog(call_id,aor,dir);
	if (!d) return CSCF_RETURN_FALSE;

	d->method = get_dialog_method(msg->first_line.u.request.method);
	STR_SHM_DUP(d->method_str,msg->first_line.u.request.method,"shm");
	d->first_cseq = cscf_get_cseq(msg,0);
	d->last_cseq = d->first_cseq;
	d->state = DLG_STATE_INITIAL;

	d->uac_supp_timer = supports_extension(msg, &str_ext_timer);

	ses_exp = cscf_get_session_expires_body(msg, &h);
	t_time = cscf_get_session_expires(ses_exp, &refresher);
	if (!t_time) {
		d->expires = d_act_time() + 60;
		d->lr_session_expires = 0;
	} else {
		d->expires = d_act_time() + t_time;
		d->lr_session_expires = t_time;
		if (refresher.len)
			STR_SHM_DUP(d->refresher, refresher, "DIALOG_REFRESHER");
	}
	
	cscf_get_from_tag(msg,&tag);
	cscf_get_from_uri(msg,&x);
	uri.len = snprintf(buf1,256,"<%.*s>",x.len,x.s);
	uri.s = buf1;	
	cscf_get_to_uri(msg,&x);
	ruri.len = snprintf(buf2,256,"<%.*s>",x.len,x.s);
	ruri.s = buf2;
	 
	tmb.new_dlg_uac(&call_id,
						&tag,
						d->first_cseq,&uri,
						&ruri,
						&d->dialog_c);
	
	tmb.new_dlg_uas(msg,99,&d->dialog_s);
	
	event = cscf_get_event(msg);
	if (event.len){
		STR_SHM_DUP(d->event,event,"shm");
	}
	else
		d->event = event;

	d_unlock(d->hash);
	
	print_e_dialogs(L_INFO);
	
	return CSCF_RETURN_TRUE;	
out_of_memory:
	if (d){
		hash = d->hash;
		del_e_dialog(d);
		d_unlock(hash);
	}
	return CSCF_RETURN_ERROR;	
}

/**
 * Updates dialog on reply message
 * @param msg - the SIP message 
 * @param d - dialog to modify
 * @returns 1 on success or 0 on error
 */
int update_dialog_on_reply(struct sip_msg *msg, e_dialog *d)
{
	struct hdr_field *h_req;
	struct hdr_field *h=0;
	int res=0;
	time_t t_time=0;
	str ses_exp = {0,0};
	str refresher = {0,0};
	str new_ses_exp = {0,0};
	str new_ext = {0,0};
	int expires = 0;

	ses_exp = cscf_get_session_expires_body(msg, &h);
	t_time = cscf_get_session_expires(ses_exp, &refresher);
	if (!t_time) //i.e not session-expires header in response
	{
		if (!d->uac_supp_timer || !d->lr_session_expires)
		{
			expires = cscf_get_expires_hdr(msg);
			if (expires >= 0)
			{
			     d->expires = d_act_time()+expires;
			}
			else
			{
			     d->expires = d_act_time()+ecscf_dialogs_expiration_time;
			}
		}
		else// uac supports timer, but no session-expires header found in response
		{
			d->expires = d_act_time()+d->lr_session_expires;
			
			new_ses_exp.len = 11/*int value*/ + str_se.len+s_refresher.len+8;
			new_ses_exp.s = pkg_malloc(new_ses_exp.len+1);
			if (!new_ses_exp.s) {
				LOG(L_ERR,"ERR:"M_NAME":update_dialog_on_reply: Error allocating %d bytes\n",new_ses_exp.len);
				goto error;
			}
			new_ses_exp.len = snprintf(new_ses_exp.s, new_ses_exp.len, "%.*s %d; %.*suac\r\n",str_se.len, str_se.s, (int)d->lr_session_expires ,s_refresher.len, s_refresher.s);
			cscf_add_header(msg, &new_ses_exp, HDR_OTHER_T);
			if (!requires_extension(msg, &str_ext_timer)) //must have require timer extenstion
			{
				/* walk through all Require headers to find first require header*/
				res = parse_headers(msg, HDR_EOH_F, 0);
				if (res == -1) {
					ERR("Error while parsing headers (%d)\n", res);
					return 0; /* what to return here ? */
				}
				
				h_req = msg->require;
				while (h_req) {
					if (h_req->type == HDR_REQUIRE_T) {
						if (h_req->body.s[new_ext.len-1]=='\n')
						{
							new_ext.len = str_require.len + 1/* */+h_req->body.len + 7;/*, timer*/
							new_ext.s = pkg_malloc(new_ext.len);
							if (!new_ext.s) {
								LOG(L_ERR,"ERR:"M_NAME":update_dialog_on_reply: Error allocating %d bytes\n",new_ext.len);
								goto error;
							}			
							new_ext.len = snprintf(new_ext.s, str_require.len, "%.*s %.*s, timer\r\n", str_require.len, str_require.s, h_req->body.len-2, h_req->body.s);
						}
						else
						{
							new_ext.len = str_require.len + 1/*space*/ + h_req->body.len + 9;/*, timer\r\n*/
							new_ext.s = pkg_malloc(new_ext.len);
							if (!new_ext.s) {
								LOG(L_ERR,"ERR:"M_NAME":update_dialog_on_reply: Error allocating %d bytes\n",new_ext.len);
								goto error;
							}			
							new_ext.len = snprintf(new_ext.s, str_require.len, "%.*s %.*s, timer\r\n", str_require.len, str_require.s, h_req->body.len, h_req->body.s);
						}
						cscf_del_header(msg, h_req);
						cscf_add_header(msg, &new_ext, HDR_REQUIRE_T);
						break;
					}
					h_req = h_req->next;
				}
			}
		}
	}
	else{
		d->expires = d_act_time() + t_time;
		d->lr_session_expires = t_time;
	}
	return 1;
error:
	if (new_ses_exp.s) pkg_free(new_ses_exp.s);
	if (new_ext.s) pkg_free(new_ext.s);
	return 0;	
}

/**
 * Updates a dialog.
 * If the initial request was:
 * - INVITE - refreshes the expiration or looks for the BYE and destroys the dialog 
 * if found
 * - SUBSCRIBE - looks for the Subscription-state in NOTIFY, refreshes the expiration 
 * and if terminated destroys the dialog
 * - When adding more dialogs, add the refreshal methods here or they will expire and will
 * be destroyed. Also add the termination to reduce the memory consumption and improve the
 * performance.
 * @param msg - the request/response
 * @param str1 - direction - "orig" or "term"
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_BREAK on error 
 */
int E_update_dialog(struct sip_msg* msg, char* str1, char* str2)
{
	str call_id;
	e_dialog *d;
	int response;
	int cseq;

	struct hdr_field *h=0;
	struct sip_msg *req=0;
	int expires=0;
	str totag;
	time_t t_time;
	str ses_exp = {0,0};
	str refresher = {0,0};

	enum e_dialog_direction dir = get_dialog_direction(str1);
		
//	if (!find_dialog_aor(msg,str1,&aor)){
//		req = cscf_get_request_from_reply(msg);		
//		if (!find_dialog_aor(req,str1,&aor)){
//			LOG(L_ERR,"ERR:"M_NAME":E_update_dialog(%s): Error retrieving %s contact\n",str1,str1);
//			return CSCF_RETURN_BREAK;
//		}
//	}		
		
	call_id = cscf_get_call_id(msg,0);
	if (!call_id.len)
		return CSCF_RETURN_FALSE;

	LOG(L_DBG,"DBG:"M_NAME":E_update_dialog(%s): Call-ID <%.*s>\n",str1,call_id.len,call_id.s);

	d = get_e_dialog_dir(call_id,dir);
//	if (!d && msg->first_line.type==SIP_REPLY){
//		/* Try to get the dialog from the request */
//		if (!req) req = cscf_get_request_from_reply(msg);		
//		if (!find_dialog_aor(req,str1,&aor)){
//			LOG(L_ERR,"ERR:"M_NAME":E_update_dialog(%s): Error retrieving %s contact\n",str1,str1);
//			return CSCF_RETURN_BREAK;
//		}		
//		d = get_e_dialog_dir(call_id,aor);		
//	}
	if (!d){
		LOG(L_INFO,"INFO:"M_NAME":E_update_dialog: dialog does not exists!\n");	
		return CSCF_RETURN_FALSE;
	}


	if (msg->first_line.type==SIP_REQUEST){
		/* Request */
		LOG(L_DBG,"DBG:"M_NAME":E_update_dialog(%s): Method <%.*s> \n",str1,
			msg->first_line.u.request.method.len,msg->first_line.u.request.method.s);
		cseq = cscf_get_cseq(msg,&h);
		if (cseq>d->last_cseq) d->last_cseq = cseq;
		if (get_dialog_method(msg->first_line.u.request.method) == DLG_METHOD_INVITE)
		{
			d->uac_supp_timer = supports_extension(msg, &str_ext_timer);
	
			ses_exp = cscf_get_session_expires_body(msg, &h);
			t_time = cscf_get_session_expires(ses_exp, &refresher);
			if (!t_time)
			{
				d->expires = d_act_time()+ecscf_dialogs_expiration_time;
				d->lr_session_expires = 0;
			}
			else
			{
				d->expires = d_act_time() + t_time;
				d->lr_session_expires = t_time;
				if (refresher.len)
					STR_SHM_DUP(d->refresher, refresher, "DIALOG_REFRESHER");
			}
		}
		else if (d->method == DLG_METHOD_SUBSCRIBE &&
			msg->first_line.u.request.method.len == 6 &&
			strncasecmp(msg->first_line.u.request.method.s,"NOTIFY",6)==0)
		{
			// Subscription-State header is mandatory for NOTIFY. See RFC 3265, Section 7.2
			expires = cscf_get_subscription_state(msg);
			if (expires >= 0)
			{
				d->expires = d_act_time()+expires;
			}
			else
			{
				d->expires = d_act_time()+ecscf_dialogs_expiration_time;
			}
		}
		else
		{
                        expires = cscf_get_expires_hdr(msg);
                        if (expires >= 0)
                        {
                                d->expires = d_act_time()+expires;
                        }
                        else
                        {
				d->expires = d_act_time()+ecscf_dialogs_expiration_time;
                        }

			d->lr_session_expires = 0;		
		}
	}else{
		/* Reply */
		response = msg->first_line.u.reply.statuscode;
		LOG(L_DBG,"DBG:"M_NAME":E_update_dialog(%s): <%d> \n",str1,response);
		cseq = cscf_get_cseq(msg,&h);
		if (cseq==0 || h==0) return CSCF_RETURN_FALSE;
		if (d->first_cseq==cseq && d->method_str.len == ((struct cseq_body *)h->parsed)->method.len &&
			strncasecmp(d->method_str.s,((struct cseq_body *)h->parsed)->method.s,d->method_str.len)==0 &&
			d->state < DLG_STATE_CONFIRMED){
			/* reply to initial request */
			if (response<200){
				d->state = DLG_STATE_EARLY;
				d->expires = d_act_time()+300;
			}else
			if (response>=200 && response<300){
				d->state = DLG_STATE_CONFIRMED;
				update_dialog_on_reply(msg, d);
				
				/*I save the dialogs only here because
				 * i only want to release confirmed dialogs*/
				cscf_get_to_tag(msg,&totag);
				tmb.update_dlg_uas(d->dialog_s,response,&totag);
				tmb.dlg_response_uac(d->dialog_c,msg,IS_NOT_TARGET_REFRESH);				
			}else
				if (response>300){
					d->state = DLG_STATE_TERMINATED;
					d_unlock(d->hash);				
					struct cell * t = tmb.t_gett();
					if(t->nr_of_outgoings < 2)
						return E_drop_dialog(msg,str1,str2);
				}				
		}else{
			/* reply to subsequent request */			
			if (!req) req = cscf_get_request_from_reply(msg);
			
			/* destroy dialogs on specific methods */
			switch (d->method){
				case DLG_METHOD_OTHER:							
					d->expires = d_act_time()+ecscf_dialogs_expiration_time;
					d->lr_session_expires = 0;
					break;
				
				case DLG_METHOD_INVITE:
					if (req && req->first_line.u.request.method.len==3 &&
						strncasecmp(req->first_line.u.request.method.s,"BYE",3)==0){
						d->state = DLG_STATE_TERMINATED;
						d_unlock(d->hash);				
						return E_drop_dialog(msg,str1,str2);
					}
					update_dialog_on_reply(msg, d);
					break;
				case DLG_METHOD_SUBSCRIBE:
//					if (req && req->first_line.u.request.method.len==9 &&
//						strncasecmp(req->first_line.u.request.method.s,"SUBSCRIBE",9)==0 &&
//						cscf_get_expires_hdr(msg)==0){						
//						d->state = DLG_STATE_TERMINATED;
//						d_unlock(d->hash);				
//						return P_droe_dialog(msg,str1,str2);
//					}
					if (req && req->first_line.u.request.method.len==6 &&
						strncasecmp(req->first_line.u.request.method.s,"NOTIFY",6)==0){
						expires = cscf_get_subscription_state(req);
						if (expires==0){						
							d->state = DLG_STATE_TERMINATED;
							d_unlock(d->hash);				
							return E_drop_dialog(msg,str1,str2);
						}else if (expires>0){
							d->expires = d_act_time() + expires;
						}
					}
					break;
			}
			if (cseq>d->last_cseq) d->last_cseq = cseq;						
		}
	}
	
	d_unlock(d->hash);
	
	print_e_dialogs(L_INFO);
	
	return CSCF_RETURN_TRUE;	
out_of_memory:
	if (d) d_unlock(d->hash);
	return CSCF_RETURN_ERROR;	
}


/**
 * Drops and deletes a dialog.
 * @param msg - the request/response
 * @param str1 - direction - "orig" or "term"
 * @param str2 - not used
 * @returns #CSCF_RETURN_TRUE if ok, #CSCF_RETURN_FALSE if not or #CSCF_RETURN_BREAK on error 
 */
int E_drop_dialog(struct sip_msg* msg, char* str1, char* str2)
{
	str call_id;
	e_dialog *d;
	int hash;
//	struct sip_msg *req;
	enum e_dialog_direction dir = get_dialog_direction(str1);
	
	
//	if (!find_dialog_aor(msg,str1,&aor)){
//		LOG(L_ERR,"ERR:"M_NAME":E_is_in_dialog(): Error retrieving %s contact\n",str1);
//		return CSCF_RETURN_BREAK;
//	}		
		
	call_id = cscf_get_call_id(msg,0);
	if (!call_id.len)
		return CSCF_RETURN_FALSE;

	LOG(L_DBG,"DBG:"M_NAME":E_drop_dialog(%s): Call-ID <%.*s> DIR[%d]\n",
		str1,call_id.len,call_id.s,
		dir);

	d = get_e_dialog_dir(call_id,dir);
//	if (!d && msg->first_line.type==SIP_REPLY){
//		/* Try to get the dialog from the request */
//		req = cscf_get_request_from_reply(msg);		
//		if (!find_dialog_aor(req,str1,&aor)){
//			LOG(L_ERR,"ERR:"M_NAME":E_update_dialog(%s): Error retrieving %s contact\n",str1,str1);
//			return CSCF_RETURN_BREAK;
//		}		
//		d = get_e_dialog(call_id,aor);		
//	}
	if (!d){
		LOG(L_ERR,"ERR:"M_NAME":E_drop_dialog: dialog does not exists!\n");	
		return CSCF_RETURN_FALSE;
	}

	hash = d->hash;
	
	del_e_dialog(d);
		
	d_unlock(hash);
	
	print_e_dialogs(L_INFO);
	
	return CSCF_RETURN_TRUE;	
}

/**
 * Drop all dialogs belonging to one AOR.
 *  on deregistration for example.
 * @param aor - the public identity of the user
 * @returns the number of dialogs dropped 
 */
int E_drop_all_dialogs(str aor)
{
	e_dialog *d,*dn;
	int i,cnt=0;;
	
	LOG(L_DBG,"DBG:"M_NAME":E_drop_all_dialogs: Called for <%.*s>\n",aor.len,aor.s);

	for(i=0;i<e_dialogs_hash_size;i++){
		d_lock(i);
			d = e_dialogs[i].head;
			while(d){
				dn = d->next;
				if (d->direction == DLG_MOBILE_ORIGINATING &&
					d->aor.len == aor.len &&
					strncasecmp(d->aor.s,aor.s,aor.len)==0) {
					if (!terminate_e_dialog(d)) del_e_dialog(d);
					cnt++;
				}						
				d = dn;
			}
		d_unlock(i);
	}
	//print_e_dialogs(L_INFO);	
	return cnt;
}


#ifdef WITH_IMS_PM
	static str zero={0,0};
#endif


/**
 * The dialog timer looks for expires dialogs and removes them
 * @param ticks - the current time
 * @param param - pointer to the dialogs list
 */
void dialog_timer(unsigned int ticks, void* param)
{
	e_dialog *d,*dn;
	int i;
	#ifdef WITH_IMS_PM
		int dialog_cnt[DLG_METHOD_MAX+1];
		for(i=0;i<=DLG_METHOD_MAX;i++)
			dialog_cnt[i]=0;
	#endif
		
	LOG(L_DBG,"DBG:"M_NAME":dialog_timer: Called at %d\n",ticks);
	if (!e_dialogs) e_dialogs = (e_dialog_hash_slot*)param;

	d_act_time();
	
	for(i=0;i<e_dialogs_hash_size;i++){
		d_lock(i);
			d = e_dialogs[i].head;
			while(d){
				dn = d->next;
				if (d->expires<=d_time_now) {
					if (!terminate_e_dialog(d)) del_e_dialog(d);
				}
				#ifdef WITH_IMS_PM
					else dialog_cnt[d->method]++;
				#endif						
				d = dn;
			}
		d_unlock(i);
	}
	print_e_dialogs(L_INFO);
	#ifdef WITH_IMS_PM
		for(i=0;i<=DLG_METHOD_MAX;i++)
			IMS_PM_LOG11(RD_NbrDialogs,get_dialog_method_str(i),dialog_cnt[i]);		
	#endif						
}



