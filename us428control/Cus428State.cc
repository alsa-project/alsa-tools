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

// Differential wheel tracking constants.
#define W_DELTA_MAX	0xff
#define W_DELTA_MIN	(W_DELTA_MAX >> 1)
// Shuttle speed wheel constants.
#define W_SPEED_MAX	0x3f

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
	}
	else SliderSend(S);
}

void Cus428State::SliderSend(int S)
{
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
		case eK_STOP:
			if (verbose > 1)
				printf("Knob STOP now %i\n", V);
			if (V) TransportToggle(T_STOP);
			Midi.SendMidiControl(K, V);
			break;
		case eK_PLAY:
			if (verbose > 1)
				printf("Knob PLAY now %i", V);
			if (V) TransportToggle(T_PLAY);
			if (verbose > 1)
				printf(" Light is %i\n", LightIs(eL_Play));
			Midi.SendMidiControl(K, V);
			break;
		case eK_REW:
			if (verbose > 1)
				printf("Knob REW now %i", V);
			if (V) TransportToggle(T_REW);
			if (verbose > 1)
				printf(" Light is %i\n", LightIs(eL_Rew));
			Midi.SendMidiControl(K, V);
			break;
		case eK_FFWD:
			if (verbose > 1)
				printf("Knob FFWD now %i", V);
			if (V) TransportToggle(T_F_FWD);
			if (verbose > 1)
				printf(" Light is %i\n", LightIs(eL_FFwd));
			Midi.SendMidiControl(K, V);
			break;
		case eK_RECORD:
			if (verbose > 1)
				printf("Knob RECORD now %i", V);
			if (V) TransportToggle(T_RECORD);
			if (verbose > 1)
				printf(" Light is %i\n", LightIs(eL_Record));
			Midi.SendMidiControl(K, V);
			break;
		case eK_SET:
			if (verbose > 1)
				printf("Knob SET now %i", V);
			bSetLocate = V;
			break;
		case eK_LOCATE_L:
			if (verbose > 1)
				printf("Knob LOCATE_L now %i", V);
			if (V) {
				if (bSetLocate)
					aWheel_L = aWheel;
				else {
					aWheel = aWheel_L;
					LocateSend();
				}
			}
			break;
		case eK_LOCATE_R:
			if (verbose > 1)
				printf("Knob LOCATE_R now %i", V);
			if (V) {
				if (bSetLocate)
					aWheel_R = aWheel;
				else {
					aWheel = aWheel_R;
					LocateSend();
				}
			}
			break;
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
		// Update the absolute wheel position.
		WheelDelta((int) ((unsigned char *) us428_ctls)[W]);
		break;
	}
	Midi.SendMidiControl(Param, ((unsigned char *) us428_ctls)[W]);
}


// Convert time-code (hh:mm:ss:ff:fr) into absolute wheel position.
void Cus428State::LocateWheel ( unsigned char *tc )
{
	aWheel  = (60 * 60 * 30) * (int) tc[0]		// hh - hours    [0..23]
			+ (     60 * 30) * (int) tc[1]		// mm - minutes  [0..59]
			+ (          30) * (int) tc[2]		// ss - seconds  [0..59]
			+                  (int) tc[3];		// ff - frames   [0..29]
}


// Convert absolute wheel position into time-code (hh:mm:ss:ff:fr)
void Cus428State::LocateTimecode ( unsigned char *tc )
{
	int W = aWheel;

	tc[0] = W / (60 * 60 * 30); W -= (60 * 60 * 30) * (int) tc[0];
	tc[1] = W / (     60 * 30); W -= (     60 * 30) * (int) tc[1];
	tc[2] = W / (          30); W -= (          30) * (int) tc[2];
	tc[3] = W;
	tc[4] = 0;
}


// Get the wheel differential.
void Cus428State::WheelDelta ( int W )
{
	// Compute the wheel differential.
	int dW = W - W0;
	if (dW > 0 && dW > +W_DELTA_MIN)
		dW -= W_DELTA_MAX;
	else
	if (dW < 0 && dW < -W_DELTA_MIN)
		dW += W_DELTA_MAX;

	W0 = W;
	aWheel += dW;

	// Can't be less than zero.
	if (aWheel < 0)
		aWheel = 0;

	// Now it's whether we're running transport already...
	if (aWheelSpeed)
		WheelShuttle(dW);
	else
		WheelStep(dW);
}

// Get the wheel step.
void Cus428State::WheelStep ( int dW )
{
	unsigned char step;

	if (dW < 0)
		step = (unsigned char) (((-dW & 0x3f) << 1) | 0x40);
	else
		step = (unsigned char) ((dW << 1) & 0x3f);

	Midi.SendMmcCommand(MMC_CMD_STEP, &step, sizeof(step));
}


// Set the wheel shuttle speed.
void Cus428State::WheelShuttle ( int dW )
{
	unsigned char shuttle[3];
	int V, forward;

	// Update the current absolute wheel shuttle speed.
	aWheelSpeed += dW;

	// Don't make it pass some limits...
	if (aWheelSpeed < -W_SPEED_MAX) aWheelSpeed = -W_SPEED_MAX;
	if (aWheelSpeed > +W_SPEED_MAX) aWheelSpeed = +W_SPEED_MAX;

	// Build the MMC-Shuttle command...
	V = aWheelSpeed;
	forward = (V >= 0);
	if (!forward)
		V = -(V);
	shuttle[0] = (unsigned char) ((V >> 3) & 0x07);		// sh
	shuttle[1] = (unsigned char) ((V & 0x07) << 4);		// sm
	shuttle[2] = (unsigned char) 0;						// sl
	if (!forward)
		shuttle[0] |= (unsigned char) 0x40;

	Midi.SendMmcCommand(MMC_CMD_SHUTTLE, &shuttle[0], sizeof(shuttle));
}


// Send the MMC wheel locate command...
void Cus428State::LocateSend ()
{
	unsigned char MmcData[6];
	// Timecode's embedded on MMC command.
	MmcData[0] = 0x01;
	LocateTimecode(&MmcData[1]);
	// Send the MMC locate command...
	Midi.SendMmcCommand(MMC_CMD_LOCATE, MmcData, sizeof(MmcData));
}


// Toggle application transport state.
void Cus428State::TransportToggle ( unsigned char T )
{
	switch (T) {
	case T_PLAY:
		if (uTransport & T_PLAY) {
			uTransport = T_STOP;
			Midi.SendMmcCommand(MMC_CMD_STOP);
		} else {
		    uTransport &= T_RECORD;
			uTransport |= T_PLAY;
			Midi.SendMmcCommand(MMC_CMD_PLAY);
		}
		break;
	case T_RECORD:
		if (uTransport & T_RECORD) {
			uTransport &= ~T_RECORD;
			Midi.SendMmcCommand(MMC_CMD_RECORD_EXIT);
		} else {
		    uTransport &= T_PLAY;
			uTransport |= T_RECORD;
			Midi.SendMmcCommand(uTransport & T_PLAY ? MMC_CMD_RECORD_STROBE : MMC_CMD_RECORD_PAUSE);
		}
		break;
	default:
		if (uTransport & T) {
			uTransport = T_STOP;
		} else {
			uTransport = T;
		}
		if (uTransport & T_STOP)
			Midi.SendMmcCommand(MMC_CMD_STOP);
		if (uTransport & T_REW)
			Midi.SendMmcCommand(MMC_CMD_REWIND);
		if (uTransport & T_F_FWD)
			Midi.SendMmcCommand(MMC_CMD_FAST_FORWARD);
		break;
	}

	TransportSend();
}


// Set application transport state.
void Cus428State::TransportSet ( unsigned char T, bool V )
{
	if (V) {
		if (T == T_RECORD) {
			uTransport |= T_RECORD;
		} else {
			uTransport  = T;
		}
	} else {
		if (T == T_RECORD) {
			uTransport &= ~T_RECORD;
		} else {
			uTransport  = T_STOP;
		}
	}

	TransportSend();
}

// Update transport status lights.
void Cus428State::TransportSend()
{
	// Common ground for shuttle speed set.
	if (uTransport & T_PLAY)
		aWheelSpeed = ((W_SPEED_MAX + 1) >> 3);
	else if (uTransport & T_REW)
		aWheelSpeed = -(W_SPEED_MAX + 1);
	else if (uTransport & T_F_FWD)
		aWheelSpeed = +(W_SPEED_MAX + 1);
	else
		aWheelSpeed = 0;

	// Lightning feedback :)
	LightSet(eL_Play,   (uTransport & T_PLAY));
	LightSet(eL_Record, (uTransport & T_RECORD));
	LightSet(eL_Rew,    (uTransport & T_REW));
	LightSet(eL_FFwd,   (uTransport & T_F_FWD));
	LightSend();
}

// Reset MMC state.
void Cus428State::MmcReset()
{
	W0 = 0;
	aWheel = aWheel_L = aWheel_R = 0;
	aWheelSpeed = 0;
	bSetLocate = false;
	uTransport = 0;

	TransportSend();
	LocateSend();
}

