/*
 * $Id: memdbg.h 98 2007-02-20 19:55:25Z dvi $
 *
 * Copyright (C) 2006 iptelorg GmbH
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* malloc debug messages
 * History:
 * --------
 *  2006-04-07             created by andrei
 */


#ifndef _memdbg_h
#define _memdbg_h

extern int memdbg;

#ifdef NO_DEBUG
	#ifdef __SUNPRO_C
		#define MDBG(...)
	#else
		#define MDBG(fmt, args...)
	#endif
#else /* NO_DEBUG */
	#ifdef __SUNPRO_C
		#define MDBG(...) LOG(memdbg, __VA_ARGS__)
	#else
		#define MDBG(fmt, args...) LOG(memdbg, fmt,  ## args)
	#endif
#endif /* NO_DEBUG */


#endif
