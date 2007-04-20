#ifndef __DIAMETER_BASE_ACCT_H
#define __DIAMETER_BASE_ACCT_H

/*
 * acct.h, acct.c provide the accounting portion of Diameter based 
 * protocol.
 * 
 * \author Shengyao Chen shc -at- fokus dot fraunhofer dot de
 */

#include "cdp_load.h"

/* Command code used in the accounting portion of Diameter base protocol. */
#define ACR 	271
#define ACA 	271

//AAAMessage * ACR_create(AAASessionId sessId, unsigned int type, struct cdp_binds* cdpb);

//void test(int x);
#endif /*__DIAMETER_BASE_ACCT_H*/
