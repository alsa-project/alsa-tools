/*
 *
 * debug.c
 *
 *	Copyright (C) Aaron Holtzman - May 1999
 *
 *  This file is part of ac3dec, a free Dolby AC-3 stream decoder.
 *	
 *  ac3dec is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  ac3dec is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#include <stdlib.h>
#include "debug.h"

static int debug_level = -1;

// Determine is debug output is required.
// We could potentially have multiple levels of debug info
int debug_is_on(void)
{
	char *env_var;
	
	if(debug_level < 0)
	{
	  env_var = getenv("AC3_DEBUG");

		if (env_var)
		{
			debug_level = 1;
		}
		else
			debug_level = 0;
	}
	
	return debug_level;
}

//If you don't have gcc, then ya don't get debug output
#ifndef __GNUC__
void dprintf(char fmt[],...)
{
	int foo = 0;
}
#endif

