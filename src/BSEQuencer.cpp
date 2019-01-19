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

#include "BSEQuencer.hpp"

BSEQuencer::BSEQuencer (double samplerate, const LV2_Feature* const* features) :
	map (NULL), notifyPort (NULL), midiIn (NULL), controlPort (NULL),
	rate (samplerate), bar (0), bpm (120.0f), speed (1.0f), position (0.0), beatsPerBar (4.0f), barBeats (4),
	beatUnit (4), key (defaultKey), scale (60, defaultScale)

{
	//Scan host features for URID map
	LV2_URID_Map* m = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp (features[i]->URI, LV2_URID__map) == 0)
		{
			m = (LV2_URID_Map*) features[i]->data;
		}
	}

	if (!m)
	{
		fprintf (stderr, "BSEQuencer.lv2: Host does not support urid:map.\n");
		return;
	}

	//Map URIS
	map = m;
	getURIs (m, &uris);
	if (!map) fprintf(stderr, "BSEQuencer.lv2: Host does not support urid:map.\n");

	// Initialize forge
	lv2_atom_forge_init (&forge, map);

	// Initialize padMessageBuffer
	padMessageBuffer[0] = PadMessage (ENDPADMESSAGE);

	ui_on = false;

}

void BSEQuencer::connect_port (uint32_t port, void *data)
{
	switch (port) {
	case CONTROL:
		controlPort = (LV2_Atom_Sequence*) data;
		break;
	case NOTIFY:
		notifyPort = (LV2_Atom_Sequence*) data;
		break;
	case MIDI_IN:
		midiIn = (LV2_Atom_Sequence*) data;
		break;
	default:
		// Connect MIDI out
		if ((port >= MIDI_OUT_1) && (port < MIDI_OUT_1 + NR_SEQUENCER_CHS)) midiOut[port - MIDI_OUT_1] = (LV2_Atom_Sequence*) data;

		// Connect controllers
		if ((port >= KNOBS) && (port < KNOBS + KNOBS_SIZE)) new_controllers[port - KNOBS] = (float*) data;
	}
}

/* Sends MIDI signal through midiOut port
 * @param ch: BSEQuencer channel
 * @param status: MIDI status byte
 * @param note: MIDI note
 * @param velocity: MIDI velocity
 *
 * TODO outCapacity
 */
void BSEQuencer::appendMidiMsg (const int64_t frames, const uint8_t ch, const uint8_t status, const int note, const uint8_t velocity)
{
	// ch -> MIDI port, MIDI channel
	int port = controllers[CH + (ch - 1) * CH_SIZE + MIDI_PORT] - 1;
	int channel = controllers[CH + (ch - 1) * CH_SIZE + MIDI_CHANNEL] - 1;

	// compose MIDI message block
	struct {
	    LV2_Atom_Event event;
	    uint8_t        msg[3];
	} midiNoteEvent = {};
	midiNoteEvent.event.time.frames = frames;
	midiNoteEvent.event.body.type = uris.midi_Event;
	midiNoteEvent.event.body.size = 3;
	midiNoteEvent.msg[0] = status + channel;
	midiNoteEvent.msg[1] = note;
	midiNoteEvent.msg[2] = velocity;

	// send MIDI message
	lv2_atom_sequence_append_event (midiOut[port], outCapacity[port], &midiNoteEvent.event);
	fprintf (stderr, "appendMidiMsg (frames: %li, port: %i, status: %i, note: %i, velocity: %i) at %f\n", frames, port, channel + status, note, velocity, position);
}

/* Prepares MIDI output of the respective key for sending
 * @param frames: time.frames
 * @param status: MIDI status byte
 * @param key: key number as element of inKeys
 * @param row: pad row
 * @param chbits: BSEQencer channels as bits
 */
bool BSEQuencer::makeMidi (const int64_t frames, const uint8_t status, const int key, const int row, uint8_t chbits)
{
	if ((key >=0) && (key < inKeys.size))
	{
		int inKeyElement = scale.getElement(inKeys[key].note);

		// Allow only valid keys
		if (inKeyElement != ENOTE)
		{
			Pad* pd = &inKeys[key].output.pads[row];
			if (((uint8_t)pd->ch) & chbits)
			{
				uint8_t outCh = pd->ch;
				int outNote = scale.getMIDInote((controllers[CH + (pd->ch - 1) * CH_SIZE + PITCH]) ? inKeyElement + row : row) +
							  pd->pitchOctave * 12 +
							  controllers[CH + (pd->ch - 1) * CH_SIZE + NOTE_OFFSET];
				float outVelocity = ((float)inKeys[key].velocity) * pd->velocity * controllers[CH + (pd->ch - 1) * CH_SIZE + VELOCITY];
				if ((outNote >=0) && (outNote <= 127))
				{
					appendMidiMsg (frames, outCh, status, outNote, LIMIT (outVelocity, 0, 127));
					return (status == LV2_MIDI_MSG_NOTE_ON);
				}
			}
		}
	}
	return false;
}

/*
 * Stops MIDI output and deletes the output playing flag for the respective pads
 */
void BSEQuencer::stopMidiOut (const int64_t frames, const uint8_t chbits)
{
	fprintf (stderr, "stopMidiOut at %f\n", position);
	for (int i = 0; i <= 127; ++i) stopMidiOut (frames, i, chbits);
}
void BSEQuencer::stopMidiOut (const int64_t frames, const int key, const uint8_t chbits)
{
	fprintf (stderr, "stopMidiOut %i at %f\n", key, position);
	for (int i = 0; i < ROWS - 1; ++i) stopMidiOut (frames, key, i, chbits);
}
void BSEQuencer::stopMidiOut (const int64_t frames, const int key, const int row, const uint8_t chbits)
{
	if ((key >= 0) && (key < inKeys.size) && inKeys[key].output.playing[row])
	{
		makeMidi (frames, LV2_MIDI_MSG_NOTE_OFF, key, row, chbits);
		inKeys[key].output.playing[row] = false;
	}
}

/*
 * Starts the MIDI output and sets the output playing flag for the respective pads
 */
void BSEQuencer::startMidiOut (const int64_t frames, const int key, const uint8_t chbits)
{
	fprintf (stderr, "startMidiOut %i at %f\n", key, position);
	for (int i = 0; i < ROWS - 1; ++i) startMidiOut (frames, key, i, chbits);
}
void BSEQuencer::startMidiOut (const int64_t frames, const int key, const int row, const uint8_t chbits)
{
	if ((key >= 0) && (key < inKeys.size) && (chbits & ((uint8_t) inKeys[key].output.pads[row].ch)))
	{
		inKeys[key].output.playing[row] = (inKeys[key].output.playing[row] || makeMidi (frames, LV2_MIDI_MSG_NOTE_ON, key, row, chbits));
	}
}

/*
 * Calculates a new position in steps from a starting position in steps and
 * position in beats relative to the starting position.
 * TODO Control row!!!
 */
double BSEQuencer::getStep (const int key, const double relpos)
{
	double startStep = inKeys[key].stepNr;
	double rawstep = (startStep + STEPS_PER_BEAT * relpos);

	// Return "raw" negative step position for before-start events
	if (rawstep <= 0.0) return rawstep;

	// Calculate exact position for post-start events
	int startStepNr = floor (startStep);
	int endStepNr = floor (rawstep);
	double stepFrac = rawstep - endStepNr;

	// Not stepped => return raw step position
	if (endStepNr == startStepNr) return fmod (rawstep, STEPS);

	// Stepped => iterate through steps
	if (endStepNr > startStepNr)
	{
		int stepNr = startStepNr;
		for (int it = startStep; it < endStepNr; )
		{
			// 1. This step: Calculate the next step to jump to
			if (stepNr < 0)
			{
				++it;
				++stepNr; // Always forward if stepNr negative
			}
			else
			{
				switch ((int) pads[ROWS-1][stepNr].ch - NR_SEQUENCER_CHS - 1)
				{
				case CTRL_SKIP: break;
				case CTRL_STOP: break;

				case CTRL_PLAY_FWD:
					inKeys[key].direction = 1;
					stepNr = (stepNr + STEPS + inKeys[key].direction) % STEPS;
					++it;
					break;

				case CTRL_PLAY_REW:
					inKeys[key].direction = -1;
					stepNr = (stepNr + STEPS + inKeys[key].direction) % STEPS;
					++it;
					break;

				case CTRL_JUMP_FWD:
				{
					if (!inKeys[key].jumpOff[stepNr])
					{
						inKeys[key].jumpOff[stepNr] = true;
						int newStepNr = stepNr;
						for (int i = 1, jumpbackCount = 1; i < STEPS; ++i)
						{
							newStepNr = (stepNr + i) % STEPS;
							if (pads[ROWS-1][newStepNr].ch - NR_SEQUENCER_CHS - 1 == CTRL_JUMP_FWD) ++jumpbackCount;
							if (pads[ROWS-1][newStepNr].ch - NR_SEQUENCER_CHS - 1 == CTRL_ALL_MARK) break;
							if (pads[ROWS-1][newStepNr].ch - NR_SEQUENCER_CHS - 1 == CTRL_MARK)
							{
								--jumpbackCount;
								if (jumpbackCount <= 0) break;
							}
						}
						stepNr = newStepNr;
					}
					else
					{
						inKeys[key].jumpOff[stepNr] = false;
						stepNr = (stepNr + STEPS + inKeys[key].direction) % STEPS;
					}
					++it;
				}

				break;

				case CTRL_JUMP_BACK:
					{
						if (!inKeys[key].jumpOff[stepNr])
						{
							inKeys[key].jumpOff[stepNr] = true;
							int newStepNr = stepNr;
							for (int i = 1, jumpbackCount = 1; i < STEPS; ++i)
							{
								newStepNr = (i <= stepNr ? stepNr - i : stepNr + STEPS - i);
								if (pads[ROWS-1][newStepNr].ch - NR_SEQUENCER_CHS - 1 == CTRL_JUMP_BACK) ++jumpbackCount;
								if (pads[ROWS-1][newStepNr].ch - NR_SEQUENCER_CHS - 1 == CTRL_ALL_MARK) break;
								if (pads[ROWS-1][newStepNr].ch - NR_SEQUENCER_CHS - 1 == CTRL_MARK)
								{
									--jumpbackCount;
									if (jumpbackCount <= 0) break;
								}
							}
							stepNr = newStepNr;
						}
						else
						{
							inKeys[key].jumpOff[stepNr] = false;
							stepNr = (stepNr + STEPS + inKeys[key].direction) % STEPS;
						}
						++it;
					}

					break;

				default:
					{
						++it;
						stepNr = (stepNr + STEPS + inKeys[key].direction) % STEPS;
					}
				}
			}

			// 2. Next step: What to do
			if (stepNr >= 0)
			{
				// CTRL_SKIP
				for (int i = 0;
						(i <= STEPS) && (pads[ROWS-1][stepNr].ch - NR_SEQUENCER_CHS - 1 == CTRL_SKIP);
						++i, stepNr = (stepNr + STEPS + inKeys[key].direction) % STEPS)
				{
					// A whole loop of SKIPs => STOP
					if (i == STEPS) return HALT_STEP;
				}

				// CTRL_STOP
				if (pads[ROWS-1][stepNr].ch - NR_SEQUENCER_CHS - 1 == CTRL_STOP) return HALT_STEP;
			}
		}
		return stepNr + stepFrac;
	}

	// TODO endStepNr < startStepNr (hard jump back)
	return fmod (rawstep, STEPS);
}


/* Core method for handling step sequencer
 * @param startpos: position (beat number) at @param start
 * @param start: start frame
 * @param end: end frame
 */
void BSEQuencer::runSequencer (const double startpos, const uint32_t start, const uint32_t end)
{
	// Playing or halted?
	if ((VALUE_SPEED > 0) && (VALUE_BPM > 0))
	{
		// Remove halted inKeys first
		bool valid;
		do
		{
			valid = true;
			for (Key** it = inKeys.begin (); it < inKeys.end(); ++it)
			{
				if ((**it).stepNr >= STEPS)
				{
					valid = false;
					inKeys.erase (it);
					break;
				}
			}
		} while (! valid);

		double endpos = startpos + (((double)end) - start) / FRAMES_PER_BEAT;

		// Internal keyboard
		for (int key = 0; key < inKeys.size; ++key)
		{
			double nextpos = endpos;
			double lastpos = startpos;

			for (double actpos = startpos; actpos <= endpos; actpos = nextpos)
			{
				int64_t actframes = LIMIT (start + (actpos - startpos) * FRAMES_PER_BEAT, start, end);
				double actstep = getStep (key, actpos - inKeys[key].startPos);
				int actStepNr = LIMIT ((int) floor (actstep), 0, STEPS - 1);
				double actStepFrac = actstep - actStepNr;

				// Break if halted
				if (actstep >= STEPS)
				{
					stopMidiOut (actframes, key, ALL_CH);
					inKeys[key].stepNr = HALT_STEP;
					*new_controllers[PLAY] = 0;
					break;
				}

				// Only present events
				if (actstep >= 0)
				{
					// Just stepped?
					if (inKeys[key].stepNr != actStepNr)
					{
						// Stop all output for the last step
						stopMidiOut (actframes, key, ALL_CH);

						// Update inKeys position data
						inKeys[key].stepNr = actStepNr;
						inKeys[key].startPos = actpos - actStepFrac / STEPS_PER_BEAT;

						//Initialize inKeys output data (copy pads)
						if ((pads[ROWS-1][actStepNr].ch >= NR_SEQUENCER_CHS + 1 + CTRL_PLAY) &&
							(pads[ROWS-1][actStepNr].ch <= NR_SEQUENCER_CHS + 1 + CTRL_JUMP_BACK))
						{
							for (int row = 0; row < ROWS - 1; ++row) inKeys[key].output.pads[row] = pads[row][actStepNr];
						}
						else inKeys[key].output.pads.fill (Pad ());

						// Start new output
						startMidiOut (actframes, key, ALL_CH);
					}

					// Scan pads and calculate note off position
					for (int row = 0; row < ROWS -1; ++row)
					{
						if (inKeys[key].output.playing[row])
						{
							double noteoffpos = inKeys[key].startPos + inKeys[key].output.pads[row].duration / STEPS_PER_BEAT;
							if ((noteoffpos >= lastpos) && (noteoffpos <= actpos))
							{
								int64_t noteoffframes = LIMIT (start + (noteoffpos - startpos) * FRAMES_PER_BEAT, start, end);
								stopMidiOut (noteoffframes, key, row, ALL_CH);
							}
						}
					}
				}

				// Calculate next step position
				if (actpos == endpos) nextpos = endpos + 1; // Why I don't like the break command ;-)
				else
				{
					nextpos = actpos + (1 - actStepFrac) / STEPS_PER_BEAT;
					if (nextpos < actpos + 1 / FRAMES_PER_BEAT) nextpos = actpos + 1 / FRAMES_PER_BEAT;	// At least one frame
					if (nextpos > endpos) nextpos = endpos;
				}
				lastpos = actpos;
			}
		}
	}
}


void BSEQuencer::run (uint32_t n_samples)
{
	int64_t last_t = 0;
	int64_t refFrame = 0;

	// Init MIDI output
	for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
	{
		// Initially midiOut contains a Chunk with size set to capacity
		// Get the capacity
		outCapacity[i] = midiOut[i]->atom.size;

		// Write an empty Sequence header to the outputs
		lv2_atom_sequence_clear(midiOut[i]);
		midiOut[i]->atom.type = midiIn->atom.type;
	}

	if (controlPort)
	{
		LV2_ATOM_SEQUENCE_FOREACH(controlPort, ev)
		{
			if (lv2_atom_forge_is_object_type(&forge, ev->body.type))
			{
				const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;

				// GUI on
				if (obj->body.otype == uris.ui_on)
				{
					ui_on = true;
					notifyPadsToGui ();
					notifyStatusToGui ();
				}

				// GUI off
				else if (obj->body.otype == uris.ui_off) ui_on = false;

				// GUI notifications
				else if (obj->body.otype == uris.notify_Event)
				{
					LV2_Atom *oPd = NULL;
					lv2_atom_object_get (obj, uris.notify_pad,  &oPd,
											  NULL);

					// Pad notification
					if (oPd && (oPd->type == uris.atom_Vector))
					{
						const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oPd;
						if (vec->body.child_type == uris.atom_Float)
						{
							const uint32_t size = (uint32_t) ((oPd->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (PadMessage));
							PadMessage* pMes = (PadMessage*) (&vec->body + 1);

							// Copy PadMessages to pads
							for (int i = 0; i < size; ++i)
							{
								int row = (int) pMes->row;
								int step = (int) pMes->step;
								if ((row >= 0) && (row < ROWS) && (step >= 0) && (step < STEPS))
								{
									Pad pd (pMes->ch, pMes->pitchOctave, pMes->velocity, pMes->duration);
									pads[row][step] = pd;
								}
							}
						}
					}
				}
			}
		}
	}

	// Update global controllers
	// 1. Stop MIDI out if play, mode or root (note/signature/octave) changed
	bool midiStopped[NR_SEQUENCER_CHS] = {false, false, false, false};
	if (CONTROLLER_CHANGED(PLAY) ||
		CONTROLLER_CHANGED(MODE) ||
		CONTROLLER_CHANGED(ROOT) ||
		CONTROLLER_CHANGED(SIGNATURE) ||
		CONTROLLER_CHANGED(OCTAVE))
	{
		fprintf (stderr, "Call stopMidiOut from 'Update global controllers' at %f\n", position);
		stopMidiOut(0, ALL_CH);
		for (int i = 0; i < NR_SEQUENCER_CHS; ++i) midiStopped[i] = true;
	}

	// 2. Stop also MIDI in if play or mode is changed
	if ((CONTROLLER_CHANGED(MODE)) || (CONTROLLER_CHANGED(PLAY)))
	{
		while (!inKeys.empty ()) inKeys.pop_back ();
	}

	// 3. Set new scale if scale or root changed
	if (CONTROLLER_CHANGED(SCALE) ||
		CONTROLLER_CHANGED(ROOT) ||
		CONTROLLER_CHANGED(SIGNATURE) ||
		CONTROLLER_CHANGED(OCTAVE))
	{
		int newScaleNr = LIMIT (*new_controllers[SCALE], 1, scaleNotes.size ());
		scale.setScale(scaleNotes[newScaleNr - 1]);
		scale.setRoot (*new_controllers[ROOT] + *new_controllers[SIGNATURE] + (*new_controllers[OCTAVE] + 1) * 12);
	}

	// 4. Copy controller values
	for (int i = 0; i < CH; ++i) controllers[i] = *new_controllers[i];

	// Update BSEQuencer channel controllers
	for (int i = CH; i < CH + NR_SEQUENCER_CHS * CH_SIZE; ++i)
	{
		// TODO Check for double assignment of port+channel

		int ch = (int)((i - CH) / CH_SIZE) + 1;
		if ((CONTROLLER_CHANGED(i)) && !midiStopped[ch-1])
		{
			fprintf (stderr, "Call stopMidiOut from 'Update BSEQuencer channel controllers' at %f\n", position);
			stopMidiOut (0, 1 << (ch - 1));
			midiStopped[ch-1] = true;
		}
		controllers[i] = *new_controllers[i];
	}

	// Read incoming events
	LV2_ATOM_SEQUENCE_FOREACH(midiIn, ev)
	{
		// Host signal received
		if (ev->body.type == uris.atom_Object || ev->body.type == uris.atom_Blank)
		{
			if (controllers[MODE] == HOST_CONTROLLED)
			{
				const LV2_Atom_Object* obj = (const LV2_Atom_Object*) &ev->body;
				if (obj->body.otype == uris.time_Position)
				{
					bool sheduleStopMidi = false;
					LV2_Atom *oBpm = NULL, *oSpeed = NULL, *oBpb = NULL, *oBu = NULL, *oBbeat = NULL;
					lv2_atom_object_get (obj, uris.time_beatsPerMinute,  &oBpm,
											  uris.time_beatsPerBar,  &oBpb,
											  uris.time_beatUnit,  &oBu,
											  uris.time_speed, &oSpeed,
											  uris.time_barBeat, &oBbeat,
											  NULL);

					// BPM changed?
					if (oBpm && (oBpm->type == uris.atom_Float) && (bpm != ((LV2_Atom_Float*)oBpm)->body))
					{
						bpm = ((LV2_Atom_Float*)oBpm)->body;
						if (controllers[MODE] == HOST_CONTROLLED) sheduleStopMidi = true;
					}

					// Beats per bar changed?
					if (oBpb && (oBpb->type == uris.atom_Float) && (beatsPerBar != ((LV2_Atom_Float*)oBpb)->body) && (((LV2_Atom_Float*)oBpb)->body > 0))
					{
						beatsPerBar = ((LV2_Atom_Float*)oBpb)->body;
						if (controllers[MODE] == HOST_CONTROLLED) sheduleStopMidi = true;
					}

					// BeatUnit changed?
					if (oBu && (oBu->type == uris.atom_Int) && (beatUnit != ((LV2_Atom_Int*)oBu)->body) &&(((LV2_Atom_Int*)oBu)->body > 0))
					{
						beatUnit = ((LV2_Atom_Int*)oBu)->body;
						if (controllers[MODE] == HOST_CONTROLLED) sheduleStopMidi = true;
					}

					// Speed changed? (not implemented yet)
					if (oSpeed && (oSpeed->type == uris.atom_Float) && (speed != ((LV2_Atom_Float*)oSpeed)->body))
					{
						speed = ((LV2_Atom_Float*)oSpeed)->body;
						if (controllers[MODE] == HOST_CONTROLLED) sheduleStopMidi = true;
					}

					// Beat position changed (during playing) ?
					if (oBbeat && (oBbeat->type == uris.atom_Float) && (barBeats != ((LV2_Atom_Float*) oBbeat)->body))
					{
						// No host sync in AUTOPLAY mode
						if (controllers[MODE] == HOST_CONTROLLED)
						{
							barBeats = ((LV2_Atom_Float*) oBbeat)->body;
							position = beatsPerBar * ((double) bar) + barBeats;
							refFrame = ev->time.frames;
						}
					}

					// Stop MIDI output for all BSEQuencer channels
					if (sheduleStopMidi && (controllers[MODE] != AUTOPLAY))
					{
						for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
						{
							if (!midiStopped[i])
							{
								fprintf (stderr, "Call stopMidiOut from 'Stop MIDI output for all BSEQuencer channels' at %f\n", position);
								stopMidiOut (ev->time.frames, 1 << i);
								midiStopped[i] = true;
							}
						}
					}
				}
			}
		}

		// MIDI event received
		else if ((controllers[PLAY]) && (controllers[MODE] == HOST_CONTROLLED) && (ev->body.type == uris.midi_Event))
		{
			const uint8_t* const msg = (const uint8_t*)(ev + 1);
			uint8_t note = msg[1];
			switch (lv2_midi_message_type(msg)) {

			// LV2_MIDI_MSG_NOTE_ON
			case LV2_MIDI_MSG_NOTE_ON:
				{
					bool newNote = true;

					// Scan keys if this is an additional midi message to an already pressed key
					// (e.g., double note on, velocity changed)
					for (Key** it = inKeys.begin(); it < inKeys.end(); ++it)
					{
						if ((**it).note == note)
						{
							newNote = false;
							(**it).velocity = msg[2];
						}
					}

					// Build new key from MIDI data
					if (newNote)
					{
						key = defaultKey; // stepNr = -1; direction = 1; output.pads, output.playing and jumpOff ()-initialized
						key.note = note;
						key.velocity = msg[2];
						key.startPos = position + ((((double)ev->time.frames) - refFrame) / FRAMES_PER_BEAT) - (1 /STEPS_PER_BEAT);
						inKeys.push_back (key);										// TODO Alternatively, push_front (key) ?
					}
				}
				break;

			// LV2_MIDI_MSG_NOTE_OFF
			case LV2_MIDI_MSG_NOTE_OFF:
				{
					for (size_t i = 0; i < inKeys.size; ++i)
					{
						if (inKeys[i].note == note)
						{
							stopMidiOut (ev->time.frames, i, ALL_CH);
							inKeys.erase (&inKeys.iterator[i]);
							break;
						}
					}
				}
				break;

			// All other MIDI signals -> forward
			default:
				//for (int i = 0; i < NR_SEQUENCER_CHS; ++i) lv2_atom_sequence_append_event (midiOut[i], outCapacity[i], ev);
				break;
			}
		}

	    // Update for this iteration
	    if (controllers[PLAY]) runSequencer (position + (((double)last_t) - refFrame) / FRAMES_PER_BEAT, last_t, ev->time.frames);

	    last_t = ev->time.frames;
	}

	// AUTOPLAY pseudo MIDI in
	if ((controllers[PLAY]) && (controllers[MODE] == AUTOPLAY))
	{
		// Exactly one inKey needed for autoplay
		// No inKeys => create an empty preliminary key
		if (inKeys.empty())
		{
			key = defaultKey;
			key.note = -99;
			inKeys.push_back (key);
		}

		// More than one inKey => shrink to one
		while (inKeys.size > 1)
		{
			stopMidiOut (last_t, inKeys.size - 1, ALL_CH);
			inKeys.pop_back ();
		}

		// Check if inKey already plays the root key
		if (inKeys[0].note != controllers[ROOT] + controllers[SIGNATURE] + (controllers[OCTAVE] + 1) * 12)
		{
			stopMidiOut (last_t, 0, ALL_CH);
			inKeys[0] = defaultKey; // stepNr = -1; direction = 1; output.pads, output.playing and jumpOff ()-initialized
			inKeys[0].note = controllers[ROOT] + controllers[SIGNATURE] + (controllers[OCTAVE] + 1) * 12;
			inKeys[0].velocity = 64;
			inKeys[0].startPos = position + ((((double)last_t) - refFrame) / FRAMES_PER_BEAT) - (1 /STEPS_PER_BEAT);
		}
	}

	// Update for the remainder of the cycle
	if ((controllers[PLAY]) && (last_t < n_samples)) runSequencer (position + (((double)last_t) - refFrame) / FRAMES_PER_BEAT, last_t, n_samples);

	//Update position until next time signal from host
	position += (((double)n_samples) - refFrame) / FRAMES_PER_BEAT;

	notifyStatusToGui ();
}

LV2_State_Status BSEQuencer::state_save (LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags,
			const LV2_Feature* const* features)
{
	store(handle, uris.state_pad, (void*) &pads, sizeof(LV2_Atom_Vector_Body) + sizeof(Pad) * STEPS * ROWS, uris.atom_Vector, LV2_STATE_IS_POD);
	return LV2_STATE_SUCCESS;
}

LV2_State_Status BSEQuencer::state_restore (LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags,
			const LV2_Feature* const* features)
{
	// Retrieve data
	size_t   size;
	uint32_t type;
	uint32_t valflags;
	const void* data = retrieve(handle, uris.state_pad, &size, &type, &valflags);

	if (data && (size == sizeof(Pad) * STEPS * ROWS) && (type == uris.atom_Vector))
	{
		// Stop MIDI out
		for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
		{
			outCapacity[i] = midiOut[i]->atom.size;
			lv2_atom_sequence_clear(midiOut[i]);
			midiOut[i]->atom.type = midiIn->atom.type;
		}
		fprintf (stderr, "Call stopMidiOut from 'state_restore' at %f\n", position);
		stopMidiOut (0, ALL_CH);

		// Clear all MIDI in
		while (!inKeys.empty()) inKeys.pop_back();

		// Copy retrieved data
		memcpy (pads, LV2_ATOM_BODY (data), sizeof(Pad) * STEPS * ROWS);

		// Copy all to padMessageBuffer for submission to GUI
		for (int i = 0; i < STEPS; ++i)
		{
			for (int j = 0; j < ROWS; ++j)
			{
				Pad* pd = &pads[j][i];
				padMessageBuffer[i * ROWS + j] = PadMessage (i, j, pd->ch, pd->pitchOctave, pd->velocity, pd->duration);
			}
		}

		// Force GUI notification
		notifyPadsToGui();
	}
	return LV2_STATE_SUCCESS;
}

void BSEQuencer::notifyPadsToGui()
{
	PadMessage endmsg (ENDPADMESSAGE);
	if (!(endmsg == padMessageBuffer[0]))
	{
		// Get padMessageBuffer size
		int end = 0;
		for (int i = 0; (i < ROWS * STEPS) && (!(padMessageBuffer[i] == endmsg)); ++i) end = i;

		// Prepare forge buffer and initialize atom sequence
		const uint32_t space = notifyPort->atom.size;
		if (space > 1024 + sizeof(PadMessage) * end)
		{
			lv2_atom_forge_set_buffer(&forge, (uint8_t*) notifyPort, space);
			lv2_atom_forge_sequence_head(&forge, &notify_frame, 0);

			// Submit data
			LV2_Atom_Forge_Frame frame;
			lv2_atom_forge_frame_time(&forge, 0);
			lv2_atom_forge_object(&forge, &frame, 0, uris.notify_Event);
			lv2_atom_forge_key(&forge, uris.notify_pad);
			lv2_atom_forge_vector(&forge, sizeof(float), uris.atom_Float, sizeof(PadMessage) / sizeof(float) * (end + 1), (void*) padMessageBuffer);
			lv2_atom_forge_pop(&forge, &frame);

			// Close off sequence
			lv2_atom_forge_pop(&forge, &notify_frame);

			// Empty padMessageBuffer
			padMessageBuffer[0] = endmsg;
		}
	}
}

void BSEQuencer::notifyStatusToGui ()
{
	// Get all act. steps for all active midiInKeys -> cursorbits
	// Get all act. played notes for all active midiInKeys -> notebits
	uint32_t cursorbits = 0;
	uint32_t notebits = 0;
	uint32_t chbits = 0;

	int8_t size = scale.getSize ();
	for (size_t i = 0; i < inKeys.size; ++i)
	{
		int8_t element = scale.getElement(inKeys[i].note);
		int stepNr = inKeys[i].stepNr;

		// Only valid keys
		if ((element != ENOTE) && (size != ENOTE))
		{
			// Set notebits
			notebits = notebits | (1 << (element % size));

			if ((stepNr >= 0) && (stepNr < STEPS))
			{

				// Set cursorbits
				cursorbits = cursorbits | (1 << stepNr);

				// Set chbits
				for (int j = 0; j < ROWS - 1; ++j)
				{
					if ((int) inKeys[i].output.pads[j].ch >= 1) chbits = chbits | (1 << ((int) (inKeys[i].output.pads[j].ch - 1)));
				}
			}
		}

	}

	// Prepare forge buffer and initialize atom sequence
	const uint32_t space = notifyPort->atom.size;
	if (space > 1024 + 2 * sizeof(int))
	{
		lv2_atom_forge_set_buffer(&forge, (uint8_t*) notifyPort, space);
		lv2_atom_forge_sequence_head(&forge, &notify_frame, 0);

		// Submit data
		LV2_Atom_Forge_Frame frame;
		lv2_atom_forge_frame_time(&forge, 0);
		lv2_atom_forge_object(&forge, &frame, 0, uris.notify_Event);
		lv2_atom_forge_key(&forge, uris.notify_cursors);
		lv2_atom_forge_int(&forge, cursorbits);
		lv2_atom_forge_key(&forge, uris.notify_notes);
		lv2_atom_forge_int(&forge, notebits);
		lv2_atom_forge_key(&forge, uris.notify_channels);
		lv2_atom_forge_int(&forge, chbits);
		lv2_atom_forge_pop(&forge, &frame);

		// Close off sequence
		lv2_atom_forge_pop(&forge, &notify_frame);
	}
}

/*
 *
 *
 ******************************************************************************
 *  LV2 specific declarations
 */

static LV2_Handle instantiate (const LV2_Descriptor* descriptor, double samplerate, const char* bundle_path, const LV2_Feature* const* features)
{
	// New instance
	BSEQuencer* instance;
	try {instance = new BSEQuencer(samplerate, features);}
	catch (std::exception& exc)
	{
		fprintf (stderr, "BSEQuencer.lv2: Plugin instantiation failed. %s\n", exc.what ());
		return NULL;
	}

	return (LV2_Handle)instance;
}

static void connect_port (LV2_Handle instance, uint32_t port, void *data)
{
	BSEQuencer* inst = (BSEQuencer*) instance;
	inst->connect_port (port, data);
}

static void run (LV2_Handle instance, uint32_t n_samples)
{
	BSEQuencer* inst = (BSEQuencer*) instance;
	inst->run (n_samples);
}

static LV2_State_Status state_save(LV2_Handle instance, LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags,
           const LV2_Feature* const* features)
{
	BSEQuencer* inst = (BSEQuencer*)instance;
	if (!inst) return LV2_STATE_SUCCESS;

	inst->state_save (store, handle, flags, features);
	return LV2_STATE_SUCCESS;
}

static LV2_State_Status state_restore(LV2_Handle instance, LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags,
           const LV2_Feature* const* features)
{
	BSEQuencer* inst = (BSEQuencer*)instance;
	inst->state_restore (retrieve, handle, flags, features);
	return LV2_STATE_SUCCESS;
}

static void cleanup (LV2_Handle instance)
{
	BSEQuencer* inst = (BSEQuencer*) instance;
	delete inst;
}

static const void* extension_data(const char* uri)
{
  static const LV2_State_Interface  state  = {state_save, state_restore};
  if (!strcmp(uri, LV2_STATE__interface)) {
    return &state;
  }
  return NULL;
}

static const LV2_Descriptor descriptor =
{
		BSEQUENCER_URI,
		instantiate,
		connect_port,
		NULL,			// activate,
		run,
		NULL,			// deactivate,
		cleanup,
		extension_data
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor (uint32_t index)
{
	switch (index)
	{
	case 0: return &descriptor;
	default: return NULL;
	}
}

/* End of LV2 specific declarations
 *
 * *****************************************************************************
 *
 *
 */
