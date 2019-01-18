#ifndef PORTS_H_
#define PORTS_H_

typedef enum {
	CONTROL				= 0,
	NOTIFY				= 1,
	MIDI_IN				= 2,
	MIDI_OUT_1			= 3,
	MIDI_OUT_2			= 4,
	MIDI_OUT_3			= 5,
	MIDI_OUT_4			= 6,

	KNOBS				= 7,
	PLAY				= 0,
	MODE				= 1,
	NR_STEPS			= 2,
	BASE				= 3,
	ROOT				= 4,
	SIGNATURE			= 5,
	OCTAVE				= 6,
	SCALE				= 7,
	AUTOPLAY_BPM		= 8,
	AUTOPLAY_BPB		= 9,
	SELECTION_CH		= 10,
	SELECTION_OCTAVE	= 11,
	SELECTION_VELOCITY	= 12,
	SELECTION_DURATION	= 13,

	CH					= 14,
	PITCH				= 0,
	VELOCITY			= 1,
	MIDI_PORT			= 2,
	MIDI_CHANNEL		= 3,
	NOTE_OFFSET			= 4,
	CH_SIZE				= 5,

	KNOBS_SIZE			= 34
} PortIndex;

#endif /* PORTS_H_ */
