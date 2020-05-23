#ifndef PORTS_H_
#define PORTS_H_

typedef enum {
	INPUT			= 0,
	OUTPUT			= 1,

	KNOBS			= 2,
	MIDI_IN_CHANNEL		= 0,
	PLAY			= 1,
	MODE			= 2,
	ON_KEY_PRESSED		= 3,
	NR_OF_STEPS		= 4,
	STEPS_PER		= 5,
	BASE			= 6,
	ROOT			= 7,
	SIGNATURE		= 8,
	OCTAVE			= 9,
	SCALE			= 10,
	AUTOPLAY_BPM		= 11,
	AUTOPLAY_BPB		= 12,
	SELECTION_CH		= 13,
	SELECTION_OCTAVE	= 14,
	SELECTION_VELOCITY	= 15,
	SELECTION_DURATION	= 16,

	CH			= 17,
	PITCH			= 0,
	VELOCITY		= 1,
	MIDI_CHANNEL		= 2,
	NOTE_OFFSET		= 3,
	CH_SIZE			= 4,

	KNOBS_SIZE		= CH + 4 * CH_SIZE
} PortIndex;

#endif /* PORTS_H_ */
