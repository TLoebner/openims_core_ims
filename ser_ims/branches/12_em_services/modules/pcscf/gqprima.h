/*
 * gqprima.h
 *
 *  Created on: Aug 28, 2009
 *      Author: adi
 */

#ifndef GQPRIMA_H_
#define GQPRIMA_H_
#include "dlg_state.h"
#include "../cdp/cdp_load.h"
#include "pcc_avp.h"

#define FL_APPEND(list,add)                                                      \
do {                                                                             \
  (add)->next = NULL;															 \
  (add)->prev = (list)->tail;													 \
  if ((list)->tail) {                                                            \
	  ((list)->tail)->next=(add);												 \
      (add)->next = NULL;                                                        \
  } else {                                                                       \
      (list)->head = (add);                                                      \
  }                                                                              \
  (list)->tail=(add);                                                        	 \
} while (0);

int gqprima_AAR(AAAMessage *aar,struct sip_msg *req, struct sip_msg *res, char *str1,int relatch);
int gqprima_AAA(AAAMessage *dia_msg);


typedef struct _t_binding_unit
{
	enum ip_type v;
	str addr;
	int port_start;
	int port_end; //only for multi-ports

	struct _t_binding_unit *prev,*next;
}t_binding_unit;

typedef struct _t_binding_list
{
	t_binding_unit *head,*tail;
}t_binding_list;



#endif /* GQPRIMA_H_ */
