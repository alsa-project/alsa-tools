#include <sound/asequencer.h>
#include "Cus428State.h"

class Cus428Midi {
 public:
	Cus428Midi():
		Seq(0){}

	int CreatePorts(){
		int Err;
		if (0 <= (Err = snd_seq_open(&Seq, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK))) {
			snd_seq_set_client_name(Seq, "US-428");
			Err = snd_seq_create_simple_port(Seq, "Controls",
							 SNDRV_SEQ_PORT_CAP_READ
							 //|SNDRV_SEQ_PORT_CAP_WRITE	FIXME: Next Step is to make Lights switchable
							 |SNDRV_SEQ_PORT_CAP_SUBS_READ
							 /*|SNDRV_SEQ_PORT_CAP_SUBS_WRITE*/,
							 SNDRV_SEQ_PORT_TYPE_MIDI_GENERIC);
			if (Err >= 0) {
				Port = Err;
				snd_seq_ev_clear(&Ev);
				snd_seq_ev_set_direct(&Ev);
				snd_seq_ev_set_source(&Ev, Port);
				snd_seq_ev_set_subs(&Ev);
			}
		}
		return Err;
	}

	int SendMidiControl(char Param, char Val){
		snd_seq_ev_set_controller(&Ev, 15, Param, Val & 0x7F);
		SubMitEvent();
		return 0;
	} 

	int SendMidiControl(Cus428State::eKnobs K, bool Down){
		return SendMidiControl(KnobParam[K - Cus428State::eK_RECORD], Down ? 0x7F : 0);
	}

 private:
	snd_seq_t *Seq;
	int Port;
	snd_seq_event_t Ev;
	int SubMitEvent(){
		snd_seq_event_output(Seq, &Ev);
		snd_seq_drain_output(Seq);
		return 0;
	}
	static char KnobParam[];
};

extern Cus428Midi Midi;
