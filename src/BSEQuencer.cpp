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
	map (NULL), notifyPort (NULL), midiOut (NULL), controlPort (NULL), outCapacity (0),
	rate (samplerate), bar (0), bpm (120.0f), speed (1.0f), position (0.0), beatsPerBar (4.0f), barBeats (4),
	beatUnit (4), key (defaultKey), scale (60, defaultScale), scheduleNotifyPadsToGui (false), scheduleNotifyStatusToGui (false)

{
	//Scan host features for URID map
	LV2_URID_Map* m = NULL;
	LV2_URID_Unmap* u = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp (features[i]->URI, LV2_URID__map) == 0)
		{
			m = (LV2_URID_Map*) features[i]->data;
		}
		else if (strcmp (features[i]->URI, LV2_URID__unmap) == 0)
		{
			u = (LV2_URID_Unmap*) features[i]->data;
		}
	}

	if (!m)
	{
		fprintf (stderr, "BSEQuencer.lv2: Host does not support urid:map.\n");
		return;
	}

	//Map URIS
	map = m;
	unmap = u;
	getURIs (m, &uris);
	if (!map) fprintf(stderr, "BSEQuencer.lv2: Host does not support urid:map.\n");

	// Initialize notify
	lv2_atom_forge_init (&notify_forge, map);

	// Initialize padMessageBuffer
	padMessageBuffer[0] = PadMessage (ENDPADMESSAGE);

	// Init defaultKey
	defaultKey.stepNr = -1;
	for (int i = 0; i < MAXSTEPS; ++i) defaultKey.output[i].direction = 1;


	// Initialize controllers
	// Controllers are zero initialized and will get data from host, only
	// NR_OF_STEPS need to be set to prevent div by zero.
	controllers[NR_OF_STEPS] = MAXSTEPS;

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
	case MIDI_OUT:
		midiOut = (LV2_Atom_Sequence*) data;
		break;
	default:
		// Connect controllers
		if ((port >= KNOBS) && (port < KNOBS + KNOBS_SIZE)) new_controllers[port - KNOBS] = (float*) data;
	}
}

uint32_t BSEQuencer::initMidiOut (LV2_Atom_Sequence* midi)
{
	if (midi)
	{
		uint32_t outCapacity = midi->atom.size;
		lv2_atom_sequence_clear(midi);
		midi->atom.type = uris.atom_Sequence;
		return outCapacity;
	}
	return 0;
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
	if (midiOut) lv2_atom_sequence_append_event (midiOut, outCapacity, &midiNoteEvent.event);
	// fprintf (stderr, "BSEQuencer.lv2: appendMidiMsg (frames: %li, status: %i, note: %i, velocity: %i) at %f\n", frames, channel + status, note, velocity, position);
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
			Pad* pd = &inKeys[key].output[row].pad;
			if (((uint8_t)pd->ch) & chbits & 0x0F)
			{
				uint8_t outCh = pd->ch;
				int outNote = scale.getMIDInote((controllers[CH + ((int)(pd->ch - 1)) * CH_SIZE + PITCH]) ? inKeyElement + row : row) +
							  pd->pitchOctave * 12 +
							  controllers[CH + ((int)(pd->ch - 1)) * CH_SIZE + NOTE_OFFSET];
				float outVelocity = ((float)inKeys[key].velocity) * pd->velocity * controllers[CH + ((int)(pd->ch) - 1) * CH_SIZE + VELOCITY];
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
	for (int i = 0; i <= 127; ++i) stopMidiOut (frames, i, chbits);
}
void BSEQuencer::stopMidiOut (const int64_t frames, const int key, const uint8_t chbits)
{
	for (int i = 0; i < ROWS - 1; ++i) stopMidiOut (frames, key, i, chbits);
}
void BSEQuencer::stopMidiOut (const int64_t frames, const int key, const int row, const uint8_t chbits)
{
	if ((key >= 0) && (key < inKeys.size) && inKeys[key].output[row].playing)
	{
		makeMidi (frames, LV2_MIDI_MSG_NOTE_OFF, key, row, chbits);
		inKeys[key].output[row].playing = false;
	}
}

/*
 * Starts the MIDI output and sets the output playing flag for the respective pads
 */
void BSEQuencer::startMidiOut (const int64_t frames, const int key, const uint8_t chbits)
{
	for (int i = 0; i < ROWS - 1; ++i) startMidiOut (frames, key, i, chbits);
}
void BSEQuencer::startMidiOut (const int64_t frames, const int key, const int row, const uint8_t chbits)
{
	if ((key >= 0) && (key < inKeys.size) && (chbits & ((uint8_t) inKeys[key].output[row].pad.ch) & 0x0F))
	{
		inKeys[key].output[row].playing = (inKeys[key].output[row].playing || makeMidi (frames, LV2_MIDI_MSG_NOTE_ON, key, row, chbits));
	}
}

/*
 * Removes halted inKeys
 */
void BSEQuencer::cleanupInKeys ()
{
	bool valid;
	do
	{
		valid = true;
		for (Key** it = inKeys.begin (); it < inKeys.end(); ++it)
		{
			bool halted = true;
			for (int row = 0; row < ROWS; ++row)
			{
				if ((**it).output[row].stepOffset < MAXSTEPS)
				{
					halted = false;
					break;
				}
			}

			if (halted)
			{
				valid = false;
				int key = it - inKeys.begin();
				stopMidiOut (0, key, ALL_CH);
				inKeys.erase (it);
				break;
			}
		}
	} while (! valid);
}

/*
 * Calculates a new position in steps from a starting position in steps and
 * position in beats relative to the starting position. Controls are not
 * interpreted.
 * @param key		Number of the respective inKey
 * @param relpos	Position relative to the inKey's startPos
 * @return			Returns the distance in steps relative to startPos
 */
double BSEQuencer::getStep (const int key, const double relpos)
{
	double startStep = inKeys[key].stepNr;
	double rawstep = startStep + STEPS_PER_BEAT * relpos;

	// Return "raw" negative step position for before-start events
	if (rawstep <= 0.0) return rawstep;
	else return fmod (rawstep, ((int)controllers[NR_OF_STEPS]));
}

/*
 * Once stepped, this method should be called. This method interprets the
 * controls and returns whether the controls additionally changed the step
 * position.
 * @param key 		Number of the respective inKey
 * @param row		Number of the respective row
 * @return			Returns the change in steps as result of interpretation of
 * 					the controls.
 * 					Returns HALT_STEP if stepping is halted by the HALT
 * 					control.
 */
int BSEQuencer::getStepOffset (const int key, const int row, const int relStep)
{
	if (relStep <= 0) return 0;

	int offset = 0;
	int startStepNr = (inKeys[key].stepNr + inKeys[key].output[row].stepOffset) % ((int)controllers[NR_OF_STEPS]);
	int endStepNr = startStepNr + relStep;

	int stepNr = startStepNr;
	for (int it = startStepNr; it < endStepNr; )
	{

		// 1. This step interpretation: At the end of each step, calculate the next step to jump to
		if (stepNr < 0)
		{
			++it;
			++stepNr; // Always forward if stepNr negative
		}
		else
		{
			int ctrl = ((int) pads[row][stepNr].ch) & 0xF0;

			switch (ctrl)
			{
			case CTRL_SKIP: break;
			case CTRL_STOP: break;

			case CTRL_PLAY_FWD:
				inKeys[key].output[row].direction = 1;
				stepNr = (stepNr + ((int)controllers[NR_OF_STEPS]) + inKeys[key].output[row].direction) % ((int)controllers[NR_OF_STEPS]);
				++it;
				break;

			case CTRL_PLAY_REW:
				inKeys[key].output[row].direction = -1;
				stepNr = (stepNr + ((int)controllers[NR_OF_STEPS]) + inKeys[key].output[row].direction) % ((int)controllers[NR_OF_STEPS]);
				++it;
				break;

			case CTRL_JUMP_FWD:
			{
				if (!inKeys[key].output[row].jumpOff[stepNr])
				{
					inKeys[key].output[row].jumpOff[stepNr] = true;
					int newStepNr = stepNr;
					for (int i = 1, jumpbackCount = 1; i < ((int)controllers[NR_OF_STEPS]); ++i)
					{
						newStepNr = (stepNr + i) % ((int)controllers[NR_OF_STEPS]);
						if ((((int)pads[row][newStepNr].ch) & 0xF0) == CTRL_JUMP_FWD) ++jumpbackCount;
						if ((((int)pads[row][newStepNr].ch) & 0xF0) == CTRL_ALL_MARK) break;
						if ((((int)pads[row][newStepNr].ch) & 0xF0) == CTRL_MARK)
						{
							--jumpbackCount;
							if (jumpbackCount <= 0) break;
						}
					}
					stepNr = newStepNr;
				}
				else
				{
					inKeys[key].output[row].jumpOff[stepNr] = false;
					stepNr = (stepNr + ((int)controllers[NR_OF_STEPS]) + inKeys[key].output[row].direction) % ((int)controllers[NR_OF_STEPS]);
				}
				++it;
			}

			break;

			case CTRL_JUMP_BACK:
				{
					if (!inKeys[key].output[row].jumpOff[stepNr])
					{
						inKeys[key].output[row].jumpOff[stepNr] = true;
						int newStepNr = stepNr;
						for (int i = 1, jumpbackCount = 1; i < ((int)controllers[NR_OF_STEPS]); ++i)
						{
							newStepNr = (i <= stepNr ? stepNr - i : stepNr + ((int)controllers[NR_OF_STEPS]) - i);
							if ((((int)pads[row][newStepNr].ch) & 0xF0) == CTRL_JUMP_BACK) ++jumpbackCount;
							if ((((int)pads[row][newStepNr].ch) & 0xF0) == CTRL_ALL_MARK) break;
							if ((((int)pads[row][newStepNr].ch) & 0xF0) == CTRL_MARK)
							{
								--jumpbackCount;
								if (jumpbackCount <= 0) break;
							}
						}
						stepNr = newStepNr;
					}
					else
					{
						inKeys[key].output[row].jumpOff[stepNr] = false;
						stepNr = (stepNr + ((int)controllers[NR_OF_STEPS]) + inKeys[key].output[row].direction) % ((int)controllers[NR_OF_STEPS]);
					}
					++it;
				}

				break;

			default:
				{
					++it;
					stepNr = (stepNr + ((int)controllers[NR_OF_STEPS]) + inKeys[key].output[row].direction) % ((int)controllers[NR_OF_STEPS]);
				}
			}
		}

		// 2. Next step interpretation: SKIP and HALT controls that need to be
		// handled already at the begin of the next step.
		if (stepNr >= 0)
		{
			// CTRL_SKIP
			for (int i = 0;
					(i <= ((int)controllers[NR_OF_STEPS])) && ((((int) pads[row][stepNr].ch) & 0xF0) == CTRL_SKIP);
					++i, stepNr = (stepNr + ((int)controllers[NR_OF_STEPS]) + inKeys[key].output[row].direction) % ((int)controllers[NR_OF_STEPS]))
			{
				// A whole loop of SKIPs => STOP
				if (i == ((int)controllers[NR_OF_STEPS])) return HALT_STEP;
			}

			// CTRL_STOP
			if ((((int) pads[row][stepNr].ch) & 0xF0) == CTRL_STOP) return HALT_STEP;
		}
	}
	return stepNr - startStepNr - relStep;
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
		cleanupInKeys ();
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
				int actStepNr = LIMIT ((int) floor (actstep), 0, ((int)controllers[NR_OF_STEPS]) - 1);
				double actStepFrac = actstep - actStepNr;

				// Only present events
				if (actstep >= 0)
				{
					// Just stepped?
					if (inKeys[key].stepNr != actStepNr)
					{
						// Stop all output for the last step
						stopMidiOut (actframes, key, ALL_CH);

						// Update all rows, if not halted before
						for (int row = 0; row < ROWS; ++row)
						{
							if (inKeys[key].output[row].stepOffset != HALT_STEP)
							{
								int rawoffset = getStepOffset (key, row, STEPS_PER_BEAT * (actpos - inKeys[key].startPos));
								if (rawoffset == HALT_STEP) inKeys[key].output[row].stepOffset = HALT_STEP;
								else
								{
									// Only positive offset values allowed
									int offset = (rawoffset >= 0 ?
											  	  (inKeys[key].output[row].stepOffset + rawoffset) % ((int)controllers[NR_OF_STEPS]) :
												  (((int)controllers[NR_OF_STEPS]) +
												   (inKeys[key].output[row].stepOffset + rawoffset)) % ((int)controllers[NR_OF_STEPS]));
									inKeys[key].output[row].stepOffset = offset;
									int rowStepNr = (actStepNr + offset) % ((int)controllers[NR_OF_STEPS]);
									inKeys[key].output[row].pad = pads[row][rowStepNr];
								}
							}
						}

						// Update inKeys position data
						inKeys[key].stepNr = actStepNr;
						inKeys[key].startPos = actpos - actStepFrac / STEPS_PER_BEAT;

						// Start new output
						startMidiOut (actframes, key, ALL_CH);
					}

					for (int row = 0; row < ROWS; ++row)
					{
						// Only if pad not halted
						if (inKeys[key].output[row].stepOffset < MAXSTEPS)
						{
							// Scan pads and calculate note off position
							if (inKeys[key].output[row].playing)
							{
								double noteoffpos = inKeys[key].startPos + inKeys[key].output[row].pad.duration / STEPS_PER_BEAT;
								if ((noteoffpos >= lastpos) && (noteoffpos <= actpos))
								{
									int64_t noteoffframes = LIMIT (start + (noteoffpos - startpos) * FRAMES_PER_BEAT, start, end);
									stopMidiOut (noteoffframes, key, row, ALL_CH);
								}
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
	outCapacity = initMidiOut(midiOut);	// TODO outCapacity not used yet

	// Validate controllers
	for (int i = 0; i < KNOBS_SIZE; ++i)
	{
		float val = validateValue (*(new_controllers[i]), controllerLimits[i]);
		if (new_controllers[i] && (val != *(new_controllers[i])))
		{
			fprintf (stderr, "BSEQuencer.lv2: Value out of range in run (): Controller#%i\n", i);
			*(new_controllers[i]) = val;
			// TODO update GUI controller
		}
	}

	// Update global controllers
	// 1. Stop MIDI out if midi_in channel, steps, play, mode or root (note/signature/octave) changed
	bool midiStopped[NR_SEQUENCER_CHS] = {false, false, false, false};
	if (CONTROLLER_CHANGED(MIDI_IN_CHANNEL) ||
		CONTROLLER_CHANGED(NR_OF_STEPS) ||
		CONTROLLER_CHANGED(PLAY) ||
		CONTROLLER_CHANGED(MODE) ||
		CONTROLLER_CHANGED(ROOT) ||
		CONTROLLER_CHANGED(SIGNATURE) ||
		CONTROLLER_CHANGED(OCTAVE))
	{
		//fprintf (stderr, "Call stopMidiOut from 'Update global controllers' at %f\n", position);
		stopMidiOut(0, ALL_CH);
		for (int i = 0; i < NR_SEQUENCER_CHS; ++i) midiStopped[i] = true;
	}

	// 2. Stop also MIDI in if midi_in channel, steps, play or mode is changed
	if (CONTROLLER_CHANGED(MIDI_IN_CHANNEL) || CONTROLLER_CHANGED(NR_OF_STEPS) || (CONTROLLER_CHANGED(MODE)) ||
		(CONTROLLER_CHANGED(PLAY)))
	{
		while (!inKeys.empty ()) inKeys.pop_back ();
	}

	// 3. Set new scale if scale or root changed
	if ((new_controllers[SCALE] && new_controllers[ROOT] && new_controllers[SIGNATURE] && new_controllers[OCTAVE]) &&
		(CONTROLLER_CHANGED(SCALE) ||
		 CONTROLLER_CHANGED(ROOT) ||
		 CONTROLLER_CHANGED(SIGNATURE) ||
		 CONTROLLER_CHANGED(OCTAVE)))
	{
		int newScaleNr = LIMIT (*new_controllers[SCALE], 1, scaleNotes.size ());
		scale.setScale(scaleNotes[newScaleNr]);
		scale.setRoot (*new_controllers[ROOT] + *new_controllers[SIGNATURE] + (*new_controllers[OCTAVE] + 1) * 12);
	}

	// 4. Copy controller values
	for (int i = 0; i < CH; ++i) if (new_controllers[i]) controllers[i] = *new_controllers[i];

	// Update BSEQuencer channel controllers
	for (int i = CH; i < CH + NR_SEQUENCER_CHS * CH_SIZE; ++i)
	{
		int ch = (int)((i - CH) / CH_SIZE) + 1;
		if ((CONTROLLER_CHANGED(i)) && !midiStopped[ch-1])
		{
			//fprintf (stderr, "Call stopMidiOut from 'Update BSEQuencer channel controllers' at %f\n", position);
			stopMidiOut (0, 1 << (ch - 1));
			midiStopped[ch-1] = true;
		}
		if (new_controllers[i]) controllers[i] = *new_controllers[i];
	}

	// Read CONTROL port (notifications from GUI and host)
	if (controlPort)
	{
		LV2_ATOM_SEQUENCE_FOREACH(controlPort, ev)
		{
			if ((ev->body.type == uris.atom_Object) || (ev->body.type == uris.atom_Blank))
			{
				const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;

				// GUI on
				if (obj->body.otype == uris.ui_on)
				{
					ui_on = true;
					//fprintf (stderr, "BSEQuencer.lv2: UI on received.\n");
					padMessageBufferAllPads ();
					scheduleNotifyPadsToGui = true;
					scheduleNotifyStatusToGui = true;
				}

				// GUI off
				else if (obj->body.otype == uris.ui_off)
				{
					//fprintf (stderr, "BSEQuencer.lv2: UI off received.\n");
					ui_on = false;
				}

				// GUI pad notifications
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
								if ((row >= 0) && (row < ROWS) && (step >= 0) && (step < MAXSTEPS))
								{
									Pad pd (pMes->ch, pMes->pitchOctave, pMes->velocity, pMes->duration);
									Pad valPad = validatePad (pd);
									pads[row][step] = valPad;
									if (valPad != pd)
									{
										fprintf (stderr, "BSEQuencer.lv2: Pad out of range in run (): pads[%i][%i].\n", row, step);
										padMessageBufferAppendPad (row, step, valPad);
										scheduleNotifyPadsToGui = true;
									}
								}
							}
						}
					}
				}

				// Host time notifications
				else if (obj->body.otype == uris.time_Position)
				{
					if (controllers[MODE] == HOST_CONTROLLED)
					{
						bool scheduleStopMidi = false;
						LV2_Atom *oBpm = NULL, *oSpeed = NULL, *oBpb = NULL, *oBu = NULL, *oBbeat = NULL, *oBar = NULL;
						lv2_atom_object_get (obj, uris.time_beatsPerMinute,  &oBpm,
												  uris.time_beatsPerBar,  &oBpb,
												  uris.time_beatUnit,  &oBu,
												  uris.time_bar,  &oBar,
												  uris.time_speed, &oSpeed,
												  uris.time_barBeat, &oBbeat,
												  NULL);

						// BPM changed?
						if (oBpm && (oBpm->type == uris.atom_Float) && (bpm != ((LV2_Atom_Float*)oBpm)->body))
						{
							bpm = ((LV2_Atom_Float*)oBpm)->body;
							//fprintf (stderr, "BSEQuencer.lv2: bpm set to %f.\n", bpm);
							if (controllers[MODE] == HOST_CONTROLLED) scheduleStopMidi = true;
						}

						// Beats per bar changed?
						if (oBpb && (oBpb->type == uris.atom_Float) && (beatsPerBar != ((LV2_Atom_Float*)oBpb)->body) && (((LV2_Atom_Float*)oBpb)->body > 0))
						{
							beatsPerBar = ((LV2_Atom_Float*)oBpb)->body;
							//fprintf (stderr, "BSEQuencer.lv2: bpb set to %f.\n", beatsPerBar);
							if (controllers[MODE] == HOST_CONTROLLED) scheduleStopMidi = true;
						}

						// BeatUnit changed?
						if (oBu && (oBu->type == uris.atom_Int) && (beatUnit != ((LV2_Atom_Int*)oBu)->body) &&(((LV2_Atom_Int*)oBu)->body > 0))
						{
							beatUnit = ((LV2_Atom_Int*)oBu)->body;
							//fprintf (stderr, "BSEQuencer.lv2: bu set to %i.\n", beatUnit);
							if (controllers[MODE] == HOST_CONTROLLED) scheduleStopMidi = true;
						}

						// Bar changed?
						if (oBar && (oBar->type == uris.atom_Long) && (bar != ((LV2_Atom_Long*)oBar)->body))
						{
							bar = ((LV2_Atom_Long*)oBar)->body;
							//fprintf (stderr, "BSEQuencer.lv2: bar set to %li.\n", bar);
							if (controllers[MODE] == HOST_CONTROLLED) scheduleStopMidi = true;
						}

						// Speed changed? (not implemented yet)
						if (oSpeed && (oSpeed->type == uris.atom_Float) && (speed != ((LV2_Atom_Float*)oSpeed)->body))
						{
							speed = ((LV2_Atom_Float*)oSpeed)->body;
							//fprintf (stderr, "BSEQuencer.lv2: speed set to %f.\n", speed);
							if (controllers[MODE] == HOST_CONTROLLED) scheduleStopMidi = true;
						}

						// Beat position changed (during playing) ?
						if (oBbeat && (oBbeat->type == uris.atom_Float) && (barBeats != ((LV2_Atom_Float*) oBbeat)->body))
						{
							// No host sync in AUTOPLAY mode
							if (controllers[MODE] == HOST_CONTROLLED)
							{
								barBeats = ((LV2_Atom_Float*) oBbeat)->body;
								position = beatsPerBar * ((double) bar) + barBeats;
								//fprintf (stderr, "BSEQuencer.lv2: barBeats set to %f (position = %f, bar = %li).\n", barBeats, position, bar);
								refFrame = ev->time.frames;
							}
						}

						// Stop MIDI output for all BSEQuencer channels
						if (scheduleStopMidi && (controllers[MODE] != AUTOPLAY))
						{
							for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
							{
								if (!midiStopped[i])
								{
									//fprintf (stderr, "Call stopMidiOut from 'Stop MIDI output for all BSEQuencer channels' at %f\n", position);
									stopMidiOut (ev->time.frames, 1 << i);
									midiStopped[i] = true;
								}
							}
						}
					}
				}

				else fprintf (stderr, "BSEQuencer.lv2: Uninterpreted object in Control port (otype = %i, %s)\n", obj->body.otype,
							  (unmap ? unmap->unmap (unmap->handle, obj->body.otype) : NULL));
			}


			// Read incoming MIDI_IN events
			else if (ev->body.type == uris.midi_Event)
			{
				if ((controllers[PLAY]) && (controllers[MODE] == HOST_CONTROLLED))
				{
					const uint8_t* const msg = (const uint8_t*)(ev + 1);
					uint8_t typ = lv2_midi_message_type(msg);
					uint8_t chn = msg[0] & 0x0F;
					uint8_t note = msg[1];

					if ((controllers[MIDI_IN_CHANNEL] == 0) || (controllers[MIDI_IN_CHANNEL] - 1 == chn))
					{
						switch (typ) {

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
							fprintf (stderr, "BSEQuencer.lv2: Uninterpreted MIDI_in message in run (): #%i (%i, %i).\n", msg[0], msg[1], msg[2]);
							break;
						}
					}
					else fprintf (stderr, "BSEQuencer.lv2: MIDI input channel filter passed MIDI_in message in run (): #%i (%i, %i).\n", msg[0], msg[1], msg[2]);
				}
			}

			else fprintf (stderr, "BSEQuencer.lv2: Uninterpreted event in Control port (type = %i, %s)\n", ev->body.type,
							 (unmap ? unmap->unmap (unmap->handle, ev->body.type) : NULL));


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
	}

	// Update for the remainder of the cycle
	if ((controllers[PLAY]) && (last_t < n_samples)) runSequencer (position + (((double)last_t) - refFrame) / FRAMES_PER_BEAT, last_t, n_samples);

	//Update position until next time signal from host
	position += (((double)n_samples) - refFrame) / FRAMES_PER_BEAT;

	scheduleNotifyStatusToGui = true;

	// Init notify port
	if (notifyPort)
	{
		uint32_t space = notifyPort->atom.size;
		lv2_atom_forge_set_buffer(&notify_forge, (uint8_t*) notifyPort, space);
		lv2_atom_forge_sequence_head(&notify_forge, &notify_frame, 0);

		// Send notifications to GUI
		if (ui_on && scheduleNotifyStatusToGui) space = space - notifyStatusToGui (space);
		if (ui_on && scheduleNotifyPadsToGui) space = space - notifyPadsToGui (space);
		lv2_atom_forge_pop(&notify_forge, &notify_frame);
	}
}

LV2_State_Status BSEQuencer::state_save (LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags,
			const LV2_Feature* const* features)
{
	fprintf (stderr, "BSEQuencer.lv2: state_save ()\n");
	store(handle, uris.state_pad, (void*) &pads, sizeof(LV2_Atom_Vector_Body) + sizeof(Pad) * MAXSTEPS * ROWS, uris.atom_Vector, LV2_STATE_IS_POD);
	return LV2_STATE_SUCCESS;
}

LV2_State_Status BSEQuencer::state_restore (LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags,
			const LV2_Feature* const* features)
{
	fprintf (stderr, "BSEQuencer.lv2: state_restore ()\n");

	// Retrieve data
	size_t   size;
	uint32_t type;
	uint32_t valflags;
	const void* data = retrieve(handle, uris.state_pad, &size, &type, &valflags);

	if (data && (size == sizeof(Pad) * MAXSTEPS * ROWS) && (type == uris.atom_Vector))
	{
		// Stop MIDI out
		outCapacity = initMidiOut (midiOut);
		//fprintf (stderr, "Call stopMidiOut from 'state_restore' at %f\n", position);
		stopMidiOut (0, ALL_CH);

		// Clear all MIDI in
		while (!inKeys.empty()) inKeys.pop_back();

		// Copy retrieved data
		memcpy (pads, LV2_ATOM_BODY (data), sizeof(Pad) * MAXSTEPS * ROWS);

		// Validate all pads
		for (int i = 0; i < ROWS; ++i)
		{
			for (int j = 0; j < MAXSTEPS; ++j)
			{
				Pad valPad = validatePad (pads[i][j]);
				if (valPad != pads[i][j])
				{
					fprintf (stderr, "BSEQuencer.lv2: Pad out of range in state_restore (): pads[%i][%i].\n", i, j);
					pads[i][j] = valPad;
				}
			}
		}

		// Copy all to padMessageBuffer for submission to GUI
		padMessageBufferAllPads ();

		// Force GUI notification
		scheduleNotifyPadsToGui = true;
	}
	return LV2_STATE_SUCCESS;
}

/*
 * Checks if a value is within a limit, and if not, puts the value within
 * this limit.
 * @param value
 * @param limit
 * @return		Value is within the limit
 */
float BSEQuencer::validateValue (float value, const Limit limit)
{
	float ltdValue = ((limit.step != 0) ? (limit.min + round ((value - limit.min) / limit.step) * limit.step) : value);
	return LIMIT (ltdValue, limit.min, limit.max);
}

/*
 * Validates a single pad
 */
Pad BSEQuencer::validatePad (Pad pad)
{

	return Pad(validateValue (pad.ch, {0, 255, 1}),
			   validateValue (pad.pitchOctave, controllerLimits[SELECTION_OCTAVE]),
			   validateValue (pad.velocity, controllerLimits[SELECTION_VELOCITY]),
			   validateValue (pad.duration, controllerLimits[SELECTION_DURATION]));
}

/*
 * Appends a single pad to padMessageBuffer
 */
bool BSEQuencer::padMessageBufferAppendPad (int row, int step, Pad pad)
{
	PadMessage end = PadMessage (ENDPADMESSAGE);
	PadMessage msg = PadMessage (step, row, pad.ch, pad.pitchOctave, pad.velocity, pad.duration);

	for (int i = 0; i < MAXSTEPS * ROWS; ++i)
	{
		if (padMessageBuffer[i] != end)
		{
			padMessageBuffer[i] = msg;
			if (i < MAXSTEPS * ROWS - 1) padMessageBuffer[i + 1] = end;
			return true;
		}
	}
	return false;
}


/*
 * Copies all pads to padMessageBuffer (thus overwrites it!)
 */
void BSEQuencer::padMessageBufferAllPads ()
{
	for (int i = 0; i < MAXSTEPS; ++i)
	{
		for (int j = 0; j < ROWS; ++j)
		{
			Pad* pd = &(pads[j][i]);
			padMessageBuffer[i * ROWS + j] = PadMessage (i, j, pd->ch, pd->pitchOctave, pd->velocity, pd->duration);
		}
	}
}

uint32_t BSEQuencer::notifyPadsToGui(const uint32_t space)
{
	PadMessage endmsg (ENDPADMESSAGE);
	if (!(endmsg == padMessageBuffer[0]))
	{
		// Get padMessageBuffer size
		int end = 0;
		for (int i = 0; (i < ROWS * MAXSTEPS) && (!(padMessageBuffer[i] == endmsg)); ++i) end = i;

		// Prepare forge buffer and initialize atom sequence
		if (space > 1024 + sizeof(PadMessage) * end)							// TODO calculate the right size
		{
			LV2_Atom_Forge_Frame frame;
			lv2_atom_forge_frame_time(&notify_forge, 0);
			lv2_atom_forge_object(&notify_forge, &frame, 0, uris.notify_Event);
			lv2_atom_forge_key(&notify_forge, uris.notify_pad);
			lv2_atom_forge_vector(&notify_forge, sizeof(float), uris.atom_Float, sizeof(PadMessage) / sizeof(float) * (end + 1), (void*) padMessageBuffer);
			lv2_atom_forge_pop(&notify_forge, &frame);

			// Empty padMessageBuffer
			padMessageBuffer[0] = endmsg;

			scheduleNotifyPadsToGui = false;
			return sizeof(PadMessage) * end;									// TODO calculate the right size
		}
	}
	return 0;
}

uint32_t BSEQuencer::notifyStatusToGui (const uint32_t space)
{
	// Get all act. steps for all active midiInKeys -> cursorbits
	// Get all act. played notes for all active midiInKeys -> notebits
	uint32_t cursorbits[MAXSTEPS] = {0};
	uint32_t notebits = 0;
	uint32_t chbits = 0;

	int8_t size = scale.getSize ();
	for (size_t i = 0; i < inKeys.size; ++i)
	{
		int8_t element = scale.getElement(inKeys[i].note);

		// Only valid keys
		if ((element != ENOTE) && (size != ENOTE))
		{
			// Set notebits
			notebits = notebits | (1 << (element % size));

			for (int row = 0; row < ROWS; ++row)
			{
				if ((inKeys[i].stepNr >= 0) && (inKeys[i].output[row].stepOffset != HALT_STEP))
				{
					int stepNr = (inKeys[i].stepNr + inKeys[i].output[row].stepOffset) % ((int)controllers[NR_OF_STEPS]);

					// Set cursorbits
					cursorbits[stepNr] = (cursorbits[stepNr] | (1 << row));

					// Set chbits
					if (((int)inKeys[i].output[row].pad.ch) & 0x0F) chbits = (chbits | (1 << (((int)inKeys[i].output[row].pad.ch - 1) & 0x0F)));
				}
			}
		}
	}

	// Prepare forge buffer and initialize atom sequence
	if (space > 1024 + 3 * sizeof(uint32_t))									// TODO calculate the right size
	{
		LV2_Atom_Forge_Frame frame;
		lv2_atom_forge_frame_time(&notify_forge, 0);
		lv2_atom_forge_object(&notify_forge, &frame, 0, uris.notify_Event);
		lv2_atom_forge_key(&notify_forge, uris.notify_cursors);
		lv2_atom_forge_vector(&notify_forge, sizeof (int), uris.atom_Int, MAXSTEPS, (void*) cursorbits);
		lv2_atom_forge_key(&notify_forge, uris.notify_notes);
		lv2_atom_forge_int(&notify_forge, notebits);
		lv2_atom_forge_key(&notify_forge, uris.notify_channels);
		lv2_atom_forge_int(&notify_forge, chbits);
		lv2_atom_forge_pop(&notify_forge, &frame);

		scheduleNotifyStatusToGui = false;
		return 3 * sizeof(uint32_t);											// TODO calculate the right size
	}
	return 0;
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

/*
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
*/

static void cleanup (LV2_Handle instance)
{
	BSEQuencer* inst = (BSEQuencer*) instance;
	delete inst;
}

/*
static const void* extension_data(const char* uri)
{
  static const LV2_State_Interface  state  = {state_save, state_restore};
  if (!strcmp(uri, LV2_STATE__interface)) {
    return &state;
  }
  return NULL;
}
*/

static const LV2_Descriptor descriptor =
{
		BSEQUENCER_URI,
		instantiate,
		connect_port,
		NULL,			// activate,
		run,
		NULL,			// deactivate,
		cleanup,
		NULL //extension_data
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
