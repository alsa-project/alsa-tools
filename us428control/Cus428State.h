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

#ifndef Cus428State_h
#define Cus428State_h

#include "Cus428_ctls.h"

class Cus428State: public us428_lights{
 public:
	Cus428State(struct us428ctls_sharedmem* Pus428ctls_sharedmem)
		:us428ctls_sharedmem(Pus428ctls_sharedmem)
		,MuteInputMonitor(0)
		,Mute(0)
		,SelectInputMonitor(0)
		,Select(0)
		,us428_ctls(0)
		{
			init_us428_lights();
			for (int v = 0; v < 5; ++v) {
				Volume[v].init(v);
			}
		}
	enum eKnobs{
		eK_RECORD =	72,
		eK_PLAY,
		eK_STOP,
		eK_FFWD,
		eK_REW,
		eK_SOLO,
		eK_REC,
		eK_NULL,
		eK_InputMonitor,	// = 80
		eK_BANK_L,
		eK_BANK_R,
		eK_LOCATE_L,
		eK_LOCATE_R,
		eK_SET =	85,
		eK_INPUTCD =	87,
		eK_HIGH =	90,
		eK_HIMID,
		eK_LOWMID,
		eK_LOW,
		eK_Select0 =	96,
		eK_Mute0 =	104,
		eK_Mute1,
		eK_Mute2,
		eK_Mute3,
		eK_Mute4,
		eK_Mute5,
		eK_Mute6,
		eK_Mute7,
		eK_AUX1 =	120,
		eK_AUX2,
		eK_AUX3,
		eK_AUX4,
		eK_ASGN,
		eK_F1,
		eK_F2,
		eK_F3,
	};
	void InitDevice(void);
	void KnobChangedTo(eKnobs K, bool V);
	void SliderChangedTo(int S, unsigned char New);
	void WheelChangedTo(E_In84 W, char Diff);
	Cus428_ctls *Set_us428_ctls(Cus428_ctls *New) {
		Cus428_ctls *Old = us428_ctls;
		us428_ctls = New;
		return Old;
	}
 private:
	int LightSend();
	void SendVolume(usX2Y_volume &V);
	struct us428ctls_sharedmem* us428ctls_sharedmem;
	bool   StateInputMonitor() {
		return  LightIs(eL_InputMonitor);
	}
	usX2Y_volume_t	Volume[5];
	char		MuteInputMonitor,
			Mute,
			SelectInputMonitor,
			Select;
	Cus428_ctls	*us428_ctls;
};

extern Cus428State* OneState;

#endif
