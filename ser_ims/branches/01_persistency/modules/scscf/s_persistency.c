/*
 * $Id$
 *
 * Copyright (C) 2004-2007 FhG Fokus
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
 * S-CSCF persistency operations
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */

#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

#include "s_persistency.h"

extern persistency_mode_t scscf_persistency_mode;/**< the type of persistency					*/
extern char* scscf_persistency_location;		/**< where to dump the persistency data 		*/ 


int scscf_persistency_keep_count=3;				/**< how many old snapshots to keep				*/

extern auth_hash_slot_t *auth_data;				/**< Authentication vector hash table 			*/
extern int auth_data_hash_size;					/**< authentication vector hash table size 		*/


/**
 * Writes the binary data to a snapshot file.
 * The file names contain the time of the dump. If a file was dumped 
 * just partialy it will contain a ".part" in the name. 
 * A link to the last complete file is created each time.
 * Old files are deleted and only scscf_persistency_keep_count time-stamped-files are kept.
 * @param x - the binary data to write
 * @param prepend_fname - what to prepend to the file_name
 * @returns 1 on success or 0 on failure
 */
int bin_dump_to_file(bin_data *x,char *prepend_fname)
{
	char c_part[256],c_time[256],c_last[256];
	time_t now;
	FILE *f;
	int k;
	
	now = time(0);
	sprintf(c_part,"%s/%s_%.10u.bin.part",scscf_persistency_location,prepend_fname,(unsigned int)now);
	sprintf(c_time,"%s/%s_%.10u.bin",scscf_persistency_location,prepend_fname,(unsigned int)now);
	sprintf(c_last,"%s/_%s.bin",scscf_persistency_location,prepend_fname);

	/* first dump to a partial file */	
	f = fopen(c_part,"w");
	if (!f){
		LOG(L_ERR,"ERR:"M_NAME":bin_dump_to_file: error when opening file <%s> for writting [%s]\n",c_part,strerror(errno));
		return 0;
	}
	k = fwrite(x->s,1,x->len,f);
	LOG(L_INFO,"INFO:"M_NAME":bin_dump_to_file: Dumped %d bytes into %s.\n",k,c_part);
	fclose(f);			
	
	/* then rename it as a complete file with timestamp */
	if (rename(c_part,c_time)<0){
		LOG(L_ERR,"ERR:"M_NAME":bin_dump_to_file: error when renaming  <%s> -> <%s> [%s]\n",c_part,c_time,strerror(errno));
		return 0;
	}
	
	/* then link the last snapshot to it */
	if (k==x->len) {
		if (remove(c_last)<0 && errno!=ENOENT){
			LOG(L_ERR,"ERR:"M_NAME":bin_dump_to_file: error when removing symlink <%s> [%s]\n",c_last,strerror(errno));
			return 0;
		}
		if (symlink(c_time,c_last)<0){
			LOG(L_ERR,"ERR:"M_NAME":bin_dump_to_file: error when symlinking <%s> -> <%s> [%s]\n",c_time,c_last,strerror(errno));
			return 0;
		}
		/* then remove old snapshots */
		{
			struct dirent **namelist;
			int i,n,k=scscf_persistency_keep_count;
			int len=strlen(prepend_fname);					
			n = scandir(scscf_persistency_location,&namelist,0,alphasort);
			if (n>0){
				for(i=n-1;i>=0;i--){
					if (strlen(namelist[i]->d_name)>len &&
						memcmp(namelist[i]->d_name,prepend_fname,len)==0) {
						if (k) k--;
						else {							
							sprintf(c_part,"%s/%s",scscf_persistency_location,namelist[i]->d_name);
							remove(c_part);
						}
					}
					free(namelist[i]);
				}
				free(namelist);
			}
		}
		return 1;	
	}
	else return 0;
}

/**
 * Reloads the snapshot from the link to the last time-stamped-file.
 * @param x - where to load
 * @param prepend_fname - with what to prepend the filename
 * @returns 1 on success, 0 on error
 */
int bin_load_from_file(bin_data *x,char *prepend_fname)
{
	char c[256];
	FILE *f;
	int k;
	
	sprintf(c,"%s/_%s.bin",scscf_persistency_location,prepend_fname);
	f = fopen(c,"r");
	if (!f) {
		LOG(L_ERR,"ERR:"M_NAME":bin_load_from_file: error opening %s : %s\n",c,strerror(errno));
		return 0;
	}
	bin_alloc(x,1024);
	while(!feof(f)){
		bin_expand(x,1024);
		k = fread(x->s+x->len,1,1024,f);
		x->len+=k;
	}
	LOG(L_INFO,"INFO:"M_NAME":bin_load_from_file: Read %d bytes from %s.\n",x->len,c);
	fclose(f);	
	return 1;
}



/**
 * Creates a snapshots of the authorization data and then calls the dumping function.
 * @returns 1 on success or 0 on failure
 */
int make_snapshot_auth()
{
	bin_data x;
	auth_userdata *aud;
	int i;
	
	if (!bin_alloc(&x,256)) goto error;
	
	for(i=0;i<auth_data_hash_size;i++){
		auth_data_lock(i);
		aud = auth_data[i].head;
		while(aud){
			if (!bin_encode_auth_userdata(&x,aud)) goto error;
			aud = aud->next;
		}
		auth_data_unlock(i);
	}
	bin_print(&x);
	switch (scscf_persistency_mode){
		case NO_PERSISTENCY:
			LOG(L_ERR,"ERR:"M_NAME":make_snapshot_auth: Snapshot done but persistency was disabled...\n");
			i = 0;
			break;
		case WITH_FILES:
			i = bin_dump_to_file(&x,"authdata");
			break;
		case WITH_DATABASE:
			LOG(L_ERR,"ERR:"M_NAME":make_snapshot_auth: Snapshot done but WITH_DATABASE not implemented...\n");
			i = 0;
			break;
		default:
			LOG(L_ERR,"ERR:"M_NAME":make_snapshot_auth: Snapshot done but no such mode %d\n",scscf_persistency_mode);
			i = 0;
	}
	
	bin_free(&x);
	return i;
error:
	return 0;
}  

int load_snapshot_auth()
{
	bin_data x;
	auth_userdata *aud;
	switch (scscf_persistency_mode){
		case NO_PERSISTENCY:
			LOG(L_ERR,"ERR:"M_NAME":load_snapshot_auth: Persistency support was disabled\n");
			return 0;
			break;
		case WITH_FILES:
			if (!bin_load_from_file(&x,"authdata")) goto error;		
			break;
		case WITH_DATABASE:
			LOG(L_ERR,"ERR:"M_NAME":load_snapshot_auth: WITH_DATABASE not implemented...\n");
			goto error;
			break;
		default:
			LOG(L_ERR,"ERR:"M_NAME":load_snapshot_auth: Can't resume because no such mode %d\n",scscf_persistency_mode);
			goto error;
	}
	bin_print(&x);
	x.max=0;
	LOG(L_INFO,"INFO:"M_NAME":load_snapshot_auth: max %d len %d\n",x.max,x.len);
	while(x.max<x.len){
		aud = bin_decode_auth_userdata(&x);
		if (!aud) return 0;
		LOG(L_INFO,"INFO:"M_NAME":load_snapshot_auth: Loaded auth_userdata for <%.*s>\n",aud->private_identity.len,aud->private_identity.s);
		auth_data_lock(aud->hash);
		aud->prev = auth_data[aud->hash].tail;
		aud->next = 0;
		if (auth_data[aud->hash].tail) auth_data[aud->hash].tail->next = aud;
		auth_data[aud->hash].tail = aud;
		if (!auth_data[aud->hash].head) auth_data[aud->hash].head = aud;
		auth_data_unlock(aud->hash);
	}
	bin_free(&x);
	return 1;
error:
	return 0;
	
}


/**
 * Timer callback for persistency dumps
 * @param ticks - what's the time
 * @param param - a given parameter to be called with
 */
void auth_persistency_timer(unsigned int ticks, void* param)
{
	make_snapshot_auth();	 	
}


