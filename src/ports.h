#ifndef PORTS_H_
#define PORTS_H_

typedef enum {
	INPUT			= 0,
	OUTPUT			= 1,

	KNOBS			= 2,
	MIDI_IN_CHANNEL		= 0,
	PLAY			= 1,
	MODE			= 2,
	NR_OF_STEPS		= 3,
	STEPS_PER		= 4,
	BASE			= 5,
	ROOT			= 6,
	SIGNATURE		= 7,
	OCTAVE			= 8,
	SCALE			= 9,
	AUTOPLAY_BPM		= 10,
	AUTOPLAY_BPB		= 11,
	SELECTION_CH		= 12,
	SELECTION_OCTAVE	= 13,
	SELECTION_VELOCITY	= 14,
	SELECTION_DURATION	= 15,

	CH			= 16,
	PITCH			= 0,
	VELOCITY		= 1,
	MIDI_CHANNEL		= 2,
	NOTE_OFFSET		= 3,
	CH_SIZE			= 4,

	KNOBS_SIZE		= CH + 4 * CH_SIZE
} PortIndex;

#endif /* PORTS_H_ */
