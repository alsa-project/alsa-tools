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
#include <alsa/asoundlib.h>
#include "Cus428Midi.h"

extern int verbose;



void us428_lights::init_us428_lights()
{
	int i = 0;
	memset(this, 0, sizeof(*this));
	for (i = 0; i < 7; ++i)
		Light[ i].Offset = i + 0x19;
}


void Cus428State::InitDevice(void)
{
	if (us428ctls_sharedmem->CtlSnapShotLast >= 0)
		SliderChangedTo(eFaderM, ((unsigned char*)(us428ctls_sharedmem->CtlSnapShot + us428ctls_sharedmem->CtlSnapShotLast))[eFaderM]);
}


int Cus428State::LightSend()
{
	int Next = us428ctls_sharedmem->p4outLast + 1;
	if(Next < 0  ||  Next >= N_us428_p4out_BUFS)
		Next = 0;
	memcpy(&us428ctls_sharedmem->p4out[Next].lights, Light, sizeof(us428_lights));
	us428ctls_sharedmem->p4out[Next].type = eLT_Light;
	return us428ctls_sharedmem->p4outLast = Next;
}

void Cus428State::SendVolume(usX2Y_volume &V)
{
	int Next = us428ctls_sharedmem->p4outLast + 1;
	if (Next < 0  ||  Next >= N_us428_p4out_BUFS)
		Next = 0;
	memcpy(&us428ctls_sharedmem->p4out[Next].vol, &V, sizeof(V));
	us428ctls_sharedmem->p4out[Next].type = eLT_Volume;
	us428ctls_sharedmem->p4outLast = Next;
}

void Cus428State::SliderChangedTo(int S, unsigned char New)
{
	if (StateInputMonitor() && S <= eFader3
	    || S == eFaderM) {
		usX2Y_volume &V = Volume[S >= eFader4 ? eFader4 : S];
		V.SetTo(S, New);
		if (S == eFaderM || !LightIs(eL_Mute0 + S))
			SendVolume(V);
	} else
		Midi.SendMidiControl(0x40 + S, ((unsigned char*)us428_ctls)[S] / 2);

}


void Cus428State::KnobChangedTo(eKnobs K, bool V)
{
	switch (K & ~(StateInputMonitor() ? 3 : -1)) {
	case eK_Select0:
		if (V) {
			int S = eL_Select0 + (K & 7);
			Light[eL_Select0 / 8].Value = 0;
			LightSet(S, !LightIs(S));
			LightSend();
		}
		break;
	case eK_Mute0:
		if (V) {
			int M = eL_Mute0 + (K & 7);
			LightSet(M, !LightIs(M));
			LightSend();
			if (StateInputMonitor()) {
				usX2Y_volume V = Volume[M - eL_Mute0];
				if (LightIs(M))
					V.LH = V.LL = V.RL = V.RH = 0;
				SendVolume(V);
			}
		}
		break;
	default:
		switch (K) {
		case eK_InputMonitor:
			if (verbose > 1)
				printf("Knob InputMonitor now %i", V);
			if (V) {
				if (StateInputMonitor()) {
					SelectInputMonitor = Light[0].Value;
					MuteInputMonitor = Light[2].Value;
				} else {
					Select = Light[0].Value;
					Mute = Light[2].Value;
				}
				LightSet(eL_InputMonitor, ! StateInputMonitor());
				Light[0].Value = StateInputMonitor() ? SelectInputMonitor : Select;
				Light[2].Value = StateInputMonitor() ? MuteInputMonitor : Mute;
				LightSend();
			}
			if (verbose > 1)
				printf(" Light is %i\n", LightIs(eL_InputMonitor));
			break;
		default:
			if (verbose > 1)
				printf("Knob %i now %i\n", K, V);
			Midi.SendMidiControl(K, V);
		}
	}
}


void Cus428State::WheelChangedTo(E_In84 W, char Diff)
{
	char Param;
	switch (W) {
	case eWheelPan:
		if (StateInputMonitor() && Light[0].Value) {
			int index = 0;

			while( index < 4 && (1 << index) !=  Light[0].Value)
				index++;

			if (index >= 4)
				return;

			Volume[index].PanTo(Diff, us428_ctls->Knob(eK_SET));
			if (!LightIs(eL_Mute0 + index))
				SendVolume(Volume[index]);
			return;
		}
		Param = 0x4D;
		break;
	case eWheelGain:
		Param = 0x48;
		break;
	case eWheelFreq:
		Param = 0x49;
		break;
	case eWheelQ:
		Param = 0x4A;
		break;
	case eWheel:
		Param = 0x60;
		break;
	}
	Midi.SendMidiControl(Param, ((unsigned char*)us428_ctls)[W]);
}
