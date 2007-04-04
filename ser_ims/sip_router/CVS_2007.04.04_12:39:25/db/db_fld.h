/* 
 * $Id$ 
 *
 * Copyright (C) 2001-2003 FhG Fokus
 * Copyright (C) 2006-2007 iptelorg GmbH
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

#ifndef _DB_FLD_H
#define _DB_FLD_H  1

/** \ingroup DB_API @{ */

#include <time.h>
#include "../str.h"
#include "db_gen.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


enum db_fld_type {
    DB_NONE = 0,   /* Bumper */
    DB_INT,        /* 32-bit integer */
    DB_FLOAT,      /* 32-bit fixed-precision number */
    DB_DOUBLE,     /* double data type */
    DB_CSTR,       /* Zero-terminated string */
    DB_STR,        /* str structure */
    DB_DATETIME,   /* Date and time in number of seconds since 1-Jan-1970 */
    DB_BLOB,       /* Generic binary object*/
    DB_BITMAP      /* Bitmap of flags */
};

enum db_fld_op {
	DB_EQ = 0, /* The value of the field must be equal */
	DB_LT,     /* The value of the field must be less than */
	DB_GT,     /* The value of the field must be greater than */
	DB_LEQ,    /* The value of the field must be let than or equal */
	DB_GEQ     /* The value of the field must be greater than or equal */
};

enum db_flags {
    DB_NULL = (1 << 0)
};

typedef struct db_fld {
	db_gen_t gen;  /* Generic part of the structure */
    char* name;
    enum db_fld_type type;
    unsigned int flags;
    union {
		int          int4;   /* integer value */
		float        flt;    /* float value */
		double       dbl;    /* double value */
		time_t       time;   /* unix time value */
		char*  cstr;         /* NULL terminated string */
		str          str;    /* str string value */
		str          blob;   /* Blob data */
		unsigned int bitmap; /* Bitmap data type, 32 flags, should be enough */ 
		long long    int8;   /* 8-byte integer */
    } v;                     /* union of all possible types */
	enum db_fld_op op;
} db_fld_t;

#define DB_FLD_LAST(fld) ((fld).name == NULL)
#define DB_FLD_EMPTY(fld) ((fld) == NULL || (fld)[0].name == NULL)

struct db_fld* db_fld(size_t n);
void db_fld_free(struct db_fld* fld, size_t n);

int db_fld_init(struct db_fld* fld, size_t n);
void db_fld_release(struct db_fld* fld, size_t n);


#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* _DB_FLD_H */
