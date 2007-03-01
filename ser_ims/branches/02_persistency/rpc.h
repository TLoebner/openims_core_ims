/* $Id$
 *
 * SER Remote Procedure Call Interface
 *
 * Copyright (C) 2005 iptelorg GmbH
 *
 * This file is part of ser, a free SIP server.
 *
 * ser is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * For a license to use the ser software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact iptel.org by e-mail at the following addresses:
 *    info@iptel.org
 *
 * ser is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _RPC_H
#define _RPC_H

/*
 * TODO: Add the possibility to add printf-like formatted string to fault
 */

enum rpc_flags {
	RET_ARRAY = (1 << 0),
	RET_VALUE = (1 << 1)
};
	

/* Send the result to the caller */
typedef int (*rpc_send_f)(void* ctx);                                      /* Send the reply to the client */
typedef void (*rpc_fault_f)(void* ctx, int code, char* fmt, ...);          /* Signal a failure to the client */
typedef int (*rpc_add_f)(void* ctx, char* fmt, ...);                       /* Add a new piece of data to the result */
typedef int (*rpc_scan_f)(void* ctx, char* fmt, ...);                      /* Retrieve request parameters */
typedef int (*rpc_printf_f)(void* ctx, char* fmt, ...);                    /* Add printf-like formated data to the result set */
typedef int (*rpc_struct_add_f)(void* ctx, char* fmt, ...);                /* Create a new structure */
typedef int (*rpc_struct_scan_f)(void* ctx, char* fmt, ...);               /* Scan attributes of a structure */
typedef int (*rpc_struct_printf_f)(void* ctx, char* name, char* fmt, ...); /* Struct version of rpc_printf */

/*
 * RPC context, this is what RPC functions get as a parameter and use
 * it to obtain the value of the parameters of the call and reference
 * to the result structure that will be returned to the caller
 */
typedef struct rpc {
	rpc_fault_f fault;
	rpc_send_f send;
	rpc_add_f add;
	rpc_scan_f scan;
	rpc_printf_f printf;
	rpc_struct_add_f struct_add;
	rpc_struct_scan_f struct_scan;
	rpc_struct_printf_f struct_printf;
} rpc_t;


/*
 * RPC Function Prototype
 */

typedef void (*rpc_function_t)(rpc_t* rpc, void* ctx);


/*
 * Remote Procedure Call Export
 */
typedef struct rpc_export {
	const char* name;        /* Name of the RPC function (null terminated) */
	rpc_function_t function; /* Pointer to the function */
	const char** doc_str;  /* Documentation strings, method signature and description */
	unsigned int flags;      /* Various flags, reserved for future use */
} rpc_export_t;


#endif /* _RPC_H */
