/*
 * Controller for Tascam US-X2Y
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

#include <stdio.h>
#include <string.h>
#include "Cus428State.h"

extern int verbose;

void
us428_lights::init_us428_lights()
{
	int i = 0;
	memset(this, 0, sizeof(*this));
	for (i = 0; i < 7; ++i)
		Light[ i].Offset = i + 0x19;
}

int 
Cus428State::LightSend()
{
	int Next = us428ctls_sharedmem->p4outLast + 1;
	if(Next < 0  ||  Next >= N_us428_p4out_BUFS)
		Next = 0;
	memcpy(&us428ctls_sharedmem->p4out[Next].lights, Light, sizeof(us428_lights));
	us428ctls_sharedmem->p4out[Next].type = eLT_Light;
	return us428ctls_sharedmem->p4outLast = Next;
}

void 
Cus428State::SliderChangedTo(int S, unsigned char New)
{
	if ((S >= eFader4 || S < 0) && S != eFaderM)
		return;

	usX2Y_volume V;
	V.SetTo(S, New);
	int Next = us428ctls_sharedmem->p4outLast + 1;
	if (Next < 0  ||  Next >= N_us428_p4out_BUFS)
		Next = 0;
	memcpy(&us428ctls_sharedmem->p4out[Next].vol, &V, sizeof(V));
	us428ctls_sharedmem->p4out[Next].type = eLT_Volume;
	us428ctls_sharedmem->p4outLast = Next;
}


void 
Cus428State::KnobChangedTo(eKnobs K, bool V)
{
	switch (K) {  
	case eK_InputMonitor:
		if (verbose > 1)
			printf("Knob InputMonitor now %i", V);
		if (V) {
			LightSet(eL_InputMonitor, ! LightIs(eL_InputMonitor));
			LightSend();
		}
		if (verbose > 1)
			printf(" Light is %i\n", LightIs(eL_InputMonitor));
		break;
	default:
		if (verbose > 1)
			printf("Knob %i now %i\n", K, V);
	}
}

