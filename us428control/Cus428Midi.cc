/*
 *
 * Copyright (c) 2003 by Karsten Wiese <annabellesgarden@yahoo.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <alsa/asoundlib.h>
#include "Cus428Midi.h"


char Cus428Midi::KnobParam[] = {
	0x17,
	0x16,
	0x15,
	0x14,
	0x13,
	0x2A,
	0x29,
	0x28,
	-1,
	0x10,
	0x11,
	0x18,
	0x19,
	0x1A,
	-1,
	-1,
	-1,
	-1,
	0x2C,
	0x2D,
	0x2E,
	0x2F,
	-1,
	-1,
	0x20,
	0x21,
	0x22,
	0x23,
	0x24,
	0x25,
	0x26,
	0x27,
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	0x30,
	0x31,
	0x32,
	0x33,
	0x34,
	0x35,
	0x36,
	0x37,
	};
