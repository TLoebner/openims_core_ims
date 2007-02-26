/*
 * $Id: dprint.c 98 2007-02-20 19:55:25Z dvi $
 *
 * 2003-02-03 added by bogdan; created by andrei
 *
 *
 * Copyright (C) 2002-2003 Fhg Fokus
 *
 * This file is part of disc, a free diameter server/client.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */




#include "dprint.h"
 
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


void dprint(int lev,char * format, ...)
{
	va_list ap;

/*	fprintf(stderr, "(%d) ",  getpid());*/
	fprintf(stderr, "%2d(%d) ", process_no, getpid());
	switch(lev){
		case L_ALERT:
			fprintf(stderr,"\033[00;31m");//blink red
			break;
		case L_CRIT:
			fprintf(stderr,"\033[01;31m");//red
			break;
		case L_ERR:
			fprintf(stderr,"\033[01;33m");//yellow
			break;
		case L_WARN:
			fprintf(stderr,"\033[01;34m");//blue
			break;
		case L_NOTICE:
			fprintf(stderr,"\033[01;36m");//yellow
			break;
		case L_INFO:
			fprintf(stderr,"\033[01;32m");//yellow
			break;
		case L_DBG:
			fprintf(stderr,"\033[01;30m");//gray
			break;
		case L_MEM:
			fprintf(stderr,"\033[01;30m");//gray
			break;
	}
	va_start(ap, format);
	vfprintf(stderr,format,ap);
	fprintf(stderr,"\033[00m");
	fflush(stderr);
	va_end(ap);
}
