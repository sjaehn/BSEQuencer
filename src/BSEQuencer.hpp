/* B.SEQuencer
 * MIDI Step Sequencer LV2 Plugin
 *
 * Copyright (C) 2018 by Sven Jähnichen
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

#define CONTROLLER_CHANGED(con) (controllers[con] != *(new_controllers[con]))
#define VALUE_SPEED (controllers[MODE] == AUTOPLAY ? 1 : speed)
#define VALUE_BPM (controllers[MODE] == AUTOPLAY ? controllers[AUTOPLAY_BPM] : bpm)
#define VALUE_BPB (controllers[MODE] == AUTOPLAY ? controllers[AUTOPLAY_BPB] : beatsPerBar)
#define VALUE_BU (controllers[MODE] == AUTOPLAY ? controllers[AUTOPLAY_BU] : beatUnit)
#define STEPS_PER_BEAT (controllers[BASE] == PER_BEAT ? controllers[NR_STEPS] : controllers[NR_STEPS] / VALUE_BPB)
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
#include "Pad.hpp"
#include "PadMessage.hpp"
#include "StaticArrayList.hpp"


typedef struct {
	int note;
	int8_t velocity;
	double startPos;
	int stepNr;
	int direction;
	struct {
		std::array<Pad, ROWS> pads;
		std::array<bool, ROWS> playing;											// TODO Really needed ? Can also be done via setting/deleting ch
	} output;
	std::array<bool, STEPS> jumpOff;
} Key;

class BSEQuencer
{
public:
	BSEQuencer (double samplerate, const LV2_Feature* const* features);
	void connect_port(uint32_t port, void *data);
	void run(uint32_t n_samples);
	LV2_State_Status state_save(LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features);
	LV2_State_Status state_restore(LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features);

private:
	void appendMidiMsg (const int64_t frames, const uint8_t ch, const uint8_t status, const int note, const uint8_t velocity);
	void sendMidiOut (const int64_t frames, const uint8_t status, const int key, uint8_t chbits = ALL_CH, uint8_t startrow = 0, uint8_t endrow = ROWS - 2);
	bool makeMidi (const int64_t frames, const uint8_t status, const int key, const int row, uint8_t chbits = ALL_CH);
	void stopMidiOut (const int64_t frames, const uint8_t chbits);
	void stopMidiOut (const int64_t frames, const int key, const uint8_t chbits);
	void stopMidiOut (const int64_t frames, const int key, const int row, const uint8_t chbits);
	void startMidiOut (const int64_t frames, const int key, const uint8_t chbits);
	void startMidiOut (const int64_t frames, const int key, const int row, const uint8_t chbits);
	double getStep (const int key, const double relpos);
	void runSequencer (const double startpos, const uint32_t start, const uint32_t end);
	void notifyPadsToGui ();
	void notifyStatusToGui ();

	// URIs
	BSEQuencerURIs uris;
	LV2_URID_Map* map;

	// DSP <-> GUI communication
	const LV2_Atom_Sequence* controlPort;
	LV2_Atom_Sequence* notifyPort;

	LV2_Atom_Forge forge;
	LV2_Atom_Forge_Frame notify_frame;

	PadMessage padMessageBuffer[STEPS * ROWS];

	// MIDI in sequences
	const LV2_Atom_Sequence* midiIn;
	std::array<LV2_Atom_Sequence*, NR_SEQUENCER_CHS> midiOut;

	// Controllers
	std::array<float*, KNOBS_SIZE> new_controllers;
	std::array<float, KNOBS_SIZE> controllers;

	//Pads
	Pad pads [ROWS] [STEPS];

	// Host communicated data
	double rate;
	long bar;
	float bpm;
	float speed;
	float beatsPerBar;
	float barBeats;
	uint32_t beatUnit;															// TODO Really needed ?
	std::array<uint32_t, NR_SEQUENCER_CHS> outCapacity;

	// Data derived from controllers or host
	double position;

	// Internals
	bool ui_on;
	StaticArrayList<Key, 16> inKeys;
	Key key;
	const Key defaultKey = {0, 0, 0, -1, 1, {{}, {}}, {}};
	BScale scale;

	std::array<BScaleNotes, 14> scaleNotes	=
				{{{CROMATICSCALE}, {MAJORSCALE}, {MINORSCALE}, {HARMONICMAJORSCALE}, {HARMONICMINORSCALE}, {MELODICMINORSCALE},
				  {DORIANSCALE}, {PHRYGIANSCALE}, {LYDIANSCALE}, {MIXOLYDIANSCALE}, {LOKRIANSCALE}, {HUNGARIANMINORSCALE},
				  {MAJORPENTATONICSCALE}, {MINORPENTATONICSCALE}}};


};

#endif /* BSEQUENCER_HPP_ */
