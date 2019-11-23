/* B.SEQuencer
 * MIDI Step Sequencer LV2 Plugin
 *
 * Copyright (C) 2018, 2019 by Sven Jähnichen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef BSEQUENCER_HPP_
#define BSEQUENCER_HPP_

#define CONTROLLER_CHANGED(con) ((new_controllers[con]) ? (controllers[con] != *(new_controllers[con])) : false)
// #define VALUE_SPEED (controllers[MODE] == AUTOPLAY ? 1 : speed)
#define VALUE_BPM (controllers[MODE] == AUTOPLAY ? controllers[AUTOPLAY_BPM] : bpm)
#define VALUE_BPB (controllers[MODE] == AUTOPLAY ? controllers[AUTOPLAY_BPB] : beatsPerBar)
#define VALUE_BU (controllers[MODE] == AUTOPLAY ? controllers[AUTOPLAY_BU] : beatUnit)
#define STEPS_PER_BEAT (controllers[BASE] == PER_BEAT ? controllers[STEPS_PER] : controllers[STEPS_PER] / VALUE_BPB)
#define FRAMES_PER_BEAT (rate / (VALUE_BPM / 60))

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <array>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/time/time.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/state/state.h>
#include "definitions.h"
#include "ports.h"
#include "urids.h"
#include "BScale.hpp"
#include "ScaleMap.hpp"
#include "Pad.hpp"
#include "PadMessage.hpp"
#include "StaticArrayList.hpp"
#include "MidiStack.hpp"

typedef struct {
	float min;
	float max;
	float step;
} Limit;

typedef struct {
	bool playing;
	int stepOffset;
	int direction;
	Pad pad;
	std::array<bool, MAXSTEPS> jumpOff;
} Output;

typedef struct {
	int note;
	int8_t velocity;
	double startPos;
	int stepNr;
	std::array<Output, MAXSTEPS> output;
} Key;

class BSEQuencer
{
public:
	BSEQuencer (double samplerate, const LV2_Feature* const* features);
	void connect_port(uint32_t port, void *data);
	void run(uint32_t n_samples);
	LV2_State_Status state_save(LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features);
	LV2_State_Status state_restore(LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features);
	void activate ();

private:
	bool makeMidi (const int64_t frames, const uint8_t status, const int key, const int row, uint8_t chbits = ALL_CH);
	void stopMidiOut (const int64_t frames, const uint8_t chbits);
	void stopMidiOut (const int64_t frames, const int key, const uint8_t chbits);
	void stopMidiOut (const int64_t frames, const int key, const int row, const uint8_t chbits);
	void startMidiOut (const int64_t frames, const int key, const uint8_t chbits);
	void startMidiOut (const int64_t frames, const int key, const int row, const uint8_t chbits);
	void cleanupInKeys ();
	double getStep (const int key, const double relpos);
	int getStepOffset (const int key, const int row, const int relStep);
	void runSequencer (const double startpos, const uint32_t start, const uint32_t end);
	float validateValue (float value, const Limit limit);
	Pad validatePad (Pad pad);
	bool padMessageBufferAppendPad (int row, int step, Pad pad);
	void padMessageBufferAllPads ();
	void notifyPadsToGui ();
	void notifyStatusToGui ();
	void notifyScaleMapsToGui ();
	void notifyMidi ();

	// URIs
	BSEQuencerURIs uris;
	LV2_URID_Map* map;
	LV2_URID_Unmap* unmap;

	MidiStack midiStack;

	// DSP <-> GUI communication
	const LV2_Atom_Sequence* inputPort;
	LV2_Atom_Sequence* outputPort;

	LV2_Atom_Forge output_forge;
	LV2_Atom_Forge_Frame output_frame;

	PadMessage padMessageBuffer[MAXSTEPS * ROWS];

	// Controllers
	float* new_controllers [KNOBS_SIZE];
	float controllers [KNOBS_SIZE];
	Limit controllerLimits [KNOBS_SIZE] =
	{
		{0, 16, 1},	// MIDI_IN_CHANNEL
		{0, 1, 1},	// PLAY
		{1, 2, 1},	// MODE
		{8, 32, 8}, 	// NR_OF_STEPS
		{1, 8, 1},	// STEPS_PER
		{1, 2, 1},	// BASE
		{0, 11, 1},	// ROOT
		{-1, 1, 1},	// SIGNATURE
		{-1, 8, 1},	// OCTAVE
		{0, NR_SYSTEM_SCALES + NR_USER_SCALES - 1, 1},	// SCALE
		{1, 300, 0},	// AUTOPLAY_BPM
		{1, 16, 1},	// AUTOPLAY_BPB
		{0, NR_SEQUENCER_CHS + NR_CTRL_BUTTONS + NR_EDIT_BUTTONS, 1},	// SELECTION_CH
		{-8, 8, 1},	// SELECTION_OCTAVE
		{0, 2, 0},	// SELECTION_VELOCITY
		{0, 1, 0},	// SELECTION_DURATION
		{0, 1, 1},	// CH PITCH
		{0, 2, 0},	// CH VELOCITY
		{1, 16, 1},	// CH MIDI_CHANNEL
		{-127, 127, 1},	// CH NOTE_OFFSET
		{0, 1, 1},	// CH PITCH
		{0, 2, 0},	// CH VELOCITY
		{1, 16, 1},	// CH MIDI_CHANNEL
		{-127, 127, 1},	// CH NOTE_OFFSET
		{0, 1, 1},	// CH PITCH
		{0, 2, 0},	// CH VELOCITY
		{1, 16, 1},	// CH MIDI_CHANNEL
		{-127, 127, 1},	// CH NOTE_OFFSET
		{0, 1, 1},	// CH PITCH
		{0, 2, 0},	// CH VELOCITY
		{1, 16, 1},	// CH MIDI_CHANNEL
		{-127, 127, 1}	// CH NOTE_OFFSET
	};

	//Pads
	Pad pads [ROWS] [MAXSTEPS];

	// Host communicated data
	double rate;
	float bpm;
	float beatsPerBar;
	uint32_t outCapacity;

	// Data derived from controllers or host
	double position;

	// Internals
	bool ui_on;
	bool scheduleNotifyPadsToGui;
	bool scheduleNotifyStatusToGui;
	bool scheduleNotifyScaleMapsToGui;
	StaticArrayList<Key, 16> inKeys;
	Key defaultKey;
	BScale scale;

	RTScaleMap rtScaleMaps[NR_SYSTEM_SCALES + NR_USER_SCALES];


};

#endif /* BSEQUENCER_HPP_ */
