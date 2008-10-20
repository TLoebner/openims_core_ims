
#include "upref.h"


/* marks content in offsets */ 
int mark_content(upref_struct* h){

    str content; 
    short i, counter = 0 ;
    short start = 0 ;
    delim_catalog catalog[256], * c_p ; 
    memset(catalog, 0x00, sizeof(catalog)) ;
    c_p = (delim_catalog *) catalog ;

    if( (h->content.s == NULL) || (h->content.len <=0)){
	LOG(L_DBG, M_NAME"::Empty header body, no preferences to parse!\n") ; 
    	return 0 ;
    }

    content.len  = h->content.len ;
    content.s = h->content.s ;

   for(i = 0 ; i < content.len ; i++, (content.s)++){
   
       switch(*(content.s)){
       
       case '>':
	   if(start == 0){
	       catalog[0].sc_offset = i ;
	       start = 1 ;
	   }
       break;

       case ';':
	   if(start == 1){
	       counter++ ;
	       catalog[counter].sc_offset = i ;
	   }
       break ;

       case '=':
       	   if(start == 1) catalog[counter].eq_offset = i ;
       break; 
       default:
       break; 
       }
   }

   counter++ ;
   CATALOG_SHMCPY(h->catalog, h->catalog_len, c_p, counter) ;
	    
// PRINT_USER_PREF_CATALOG(h) ;
return h->catalog_len ;
}

/* user preference mapper */ 
void pref_mapper(int class_no, upref_struct* upref, int index, 
	unsigned char* label_start, upref_tag_type ttype){

    int cnt= 0, res = 0 ;
    unsigned char* label_end = label_start + class_no ;
    unsigned char offset;
    char* c_p = NULL, * target_p = NULL;
    char** label_pp ; 
    upref_bool end_loop = FALSE ;
    str val ;

    if(class_no != 0)
	MAP_CLSNO2_ARR(class_no, label_pp) ;
	
    if( (ttype == ENUM_PREF) || (ttype == STR_PREF) ) {

	val.s = label_end + 3 ;

	if( (index+1) < upref->catalog_len){
	    val.len = upref->catalog[index+1].sc_offset ;
	} else { 
	    unsigned char* length_p = (upref->content.s + upref->content.len) ;
	    val.len = (short) ( length_p - (unsigned char *)val.s ) ;
	}

    } else if (ttype == NUM_PREF ){

	    val.s = label_end + 2 ;   
	    val.len = upref->catalog[index+1].sc_offset ;

    } else {/* bool pref */ }
    
    
    target_p = label_start ;
    while( ( (c_p = label_pp[cnt]) != '\0') && end_loop != TRUE ){

	if( strncmp(target_p, c_p, class_no) == 0 ){
	    MAP_ELM2_ENUMTYPE(class_no, cnt, res) 
	    switch(res){
		case TEXT: upref->text = TRUE ; break;
		case DATA: upref->data = TRUE ; break;
		case AUDIO: upref->audio = TRUE ; break ; 
		case VIDEO: upref->video = TRUE ; break ; 
		case CONTROL: upref->control = TRUE ; break;
		case ISFOCUS: upref->isfocus = TRUE ; break ;
		case AUTOMATA: upref->automata = TRUE ; break ;
		case MOBILITY:
// 		    parse_enum_values(upref->mobility, enum_mobility_vals, val) ;
		    break ;
	        case CLASS:
// 		    parse_enum_values(upref->class_, enum_class_vals, val) ;
		break ;
		case APPLICATION: upref->application = TRUE ; break ;
		default: 
		break ; 
	    }
	    end_loop = TRUE ;

	} else { 
	    cnt++ ; 
	}

    }
}

void parse_enum_values(upref_enum* up_enum_p, upref_avp* eval_catalog[], str match){

int i, j, comma_no = 0 ;
int list_no = 0 ;
char* t_p = match.s ;
short offset_list[256] ;
	
    for(i = 0 ; i < match.len ; i++, t_p++) 
	if( *(t_p) == ',') {
	offset_list[comma_no] = i ;
	comma_no++ ;
	}

    up_enum_p->val_list_len = comma_no + 1 ;
    up_enum_p->val_list = (short *) shm_malloc(sizeof(short)*up_enum_p->val_list_len) ;

    t_p = match.s ;

    for(i = 0 ; i < up_enum_p->val_list_len; i++){
	for(j = 0 ; eval_catalog[j]->catalog_name != NULL ; j++){
	
	    if(strncmp(t_p, eval_catalog[j]->catalog_name, (offset_list[i]-1))==0){
		    up_enum_p->val_list[list_no] = eval_catalog[j]->enum_value ;		
		    list_no++ ;
	    }
	}
    t_p = match.s + offset_list[i] + 1 ;	
    }

}

void parse_str_values(upref_str* up_str_p, str copy){
//  	    STR_SHM_DUP( &copy, up_str_p, "parse_str_values") ;
}

upref_tag_type get_label_properties(upref_struct* user_pref, int index, 
	unsigned char** label_begin, unsigned char** label_end){

upref_tag_type tag_type = UNDEF ; 

    *label_begin = user_pref->content.s + ((user_pref->catalog[index].sc_offset) +1)  ;

    if( user_pref->catalog[index].eq_offset != 0 ) {
	
	*label_end = user_pref->content.s + (user_pref->catalog[index].eq_offset) ;

	if( ((char) *((*label_end) + 1)) != '"')
	    tag_type = NUM_PREF ; 
	else 
	    tag_type = STR_PREF ;
	
    } else {
	tag_type = BOOL_PREF ;
	if( (index+1) > user_pref->catalog_len)
	    *label_end = user_pref->content.s + user_pref->content.len ;
	else 
	    *label_end = user_pref->content.s +  (user_pref->catalog[index+1].sc_offset) ;
    }

return tag_type ;
}

/* allocate user preference structure and bind it to actual contact */ 
int create_user_pref(sip_msg* msg, r_contact* contact){

    int i, j;
    unsigned char* label_start ;
    unsigned char* label_end ;
    upref_tag_type tag_type ;
    contact_t* orig_contact ;

    if(parse_headers(msg, HDR_CONTACT_F, 0) != 0 || contact == NULL ){
	LOG(L_ERR,"ERR::"M_NAME"::%s@%d> Error in prologue!\n", __FILE__, __LINE__) ;
   	return 0 ; 
    }

    orig_contact = ((contact_body_t *)(msg->contact->parsed))->contacts ;

    contact->user_pref = (struct _upref_struct *) shm_malloc(sizeof(struct _upref_struct)) ;
    memset(contact->user_pref, 0x00, sizeof(struct _upref_struct)) ;
    STR_SHM_DUP( &(contact->user_pref->content), &(msg->contact->body), "create_upref") ; /* mod.h macro */ 

    mark_content(contact->user_pref) ; 

    for(j = 1 ; j < contact->user_pref->catalog_len ; j++){

	tag_type = get_label_properties(contact->user_pref, j, &label_start, &label_end) ;
		
	LOG(L_INFO,"start for token %.*s, index:%d\n", (label_end - label_start), label_start, j); 

	for(i = 0 ; i < CLASS_NUMBER ; i++){
	    if((label_end - label_start) == class_list[i]){
		pref_mapper(class_list[i], contact->user_pref, j, label_start,  tag_type) ;
	    }	
	}
    }

out_of_memory: 
    if(contact->user_pref) shm_free(contact->user_pref) ;
    

    LOG(L_INFO,"INFO::"M_NAME"::create_user_pref() created preferences for contact:%.*s \n", 
	    orig_contact->uri.len, orig_contact->uri.s) ;
return  0;
}


/* free user preference structure */ 
void del_user_pref(r_contact* contact){

    if( (contact == NULL) || (contact->user_pref == NULL) ){
	LOG(L_ERR,"ERR::"M_NAME"::%s@%d> Error in prologue!\n", __FILE__, __LINE__) ;
   	return ; 
    }

    if(contact->user_pref->content.s != NULL) shm_free(contact->user_pref->content.s) ;
    if(contact->user_pref->catalog != NULL) shm_free(contact->user_pref->catalog) ;
    shm_free(contact->user_pref) ;

return ;
}

