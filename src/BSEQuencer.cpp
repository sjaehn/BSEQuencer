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
	map (NULL), unmap (NULL), inputPort (NULL), outputPort (NULL),
	output_forge (), output_frame (),
	new_controllers {nullptr}, controllers {0},
	rate (samplerate), bpm (120.0f), beatsPerBar (4.0f),
	outCapacity (0), position (0.0),
	ui_on (false), scheduleNotifyPadsToGui (false), scheduleNotifyStatusToGui (false),
	scheduleNotifyScaleMapsToGui (true),
	defaultKey (), scale (60, defaultScale)

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
	lv2_atom_forge_init (&output_forge, map);

	// Init scale maps
	for (int scaleNr = 0; scaleNr < NR_SYSTEM_SCALES + NR_USER_SCALES; ++scaleNr)
	{
		rtScaleMaps[scaleNr] = defaultScaleMaps[scaleNr];
	}

	// Initialize padMessageBuffer
	padMessageBuffer[0] = PadMessage (ENDPADMESSAGE);

	// Init defaultKey
	defaultKey.stepNr = -1;
	for (int i = 0; i < MAXSTEPS; ++i)
	{
		defaultKey.output[i].direction = 1;
		defaultKey.output[i].stepOffset = 0;
	}


	// Initialize controllers
	// Controllers are zero initialized and will get data from host, only
	// NR_OF_STEPS need to be set to prevent div by zero.
	controllers[NR_OF_STEPS] = MAXSTEPS;

	ui_on = false;

}

void BSEQuencer::connect_port (uint32_t port, void *data)
{
	switch (port) {
	case INPUT:
		inputPort = (LV2_Atom_Sequence*) data;
		break;
	case OUTPUT:
		outputPort = (LV2_Atom_Sequence*) data;
		break;
	default:
		// Connect controllers
		if ((port >= KNOBS) && (port < KNOBS + KNOBS_SIZE)) new_controllers[port - KNOBS] = (float*) data;
	}
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
	if ((key >= 0) && (key < int (inKeys.size)))
	{
		int inKeyElement = scale.getElement(inKeys[key].note);

		// Allow only valid keys
		if (inKeyElement != ENOTE)
		{
			Pad* pd = &inKeys[key].output[row].pad;
			uint8_t outCh = (((uint8_t)pd->ch) & 0x0F);
			if ((1 << (outCh - 1)) & chbits)
			{
				int scaleNr = controllers[SCALE];
				int outNote;

				// Drumkit: absolute MIDI notes, not input pitched
				if (rtScaleMaps[scaleNr].elements[row] &0x100) outNote = rtScaleMaps[scaleNr].elements[row] & 0x0FF;

				// Scale: relative Notes obtained from actual scale, input pitched
				else
				{
					int pitch = ((controllers[CH + (outCh - 1) * CH_SIZE + PITCH]) ? inKeyElement : 0);
					outNote = scale.getMIDInote((rtScaleMaps[scaleNr].elements[row] & 0x0FF) + pitch);
				}

				// Apply octave shift, note offset
				outNote += pd->pitchOctave * 12 + controllers[CH + (outCh - 1) * CH_SIZE + NOTE_OFFSET];

				float outVelocity = ((float)inKeys[key].velocity) * pd->velocity * controllers[CH + (outCh - 1) * CH_SIZE + VELOCITY];
				if ((outNote >=0) && (outNote <= 127))
				{
					midiStack.append (frames, outCh, status, outNote, LIMIT (outVelocity, 0, 127));
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
	for (int i = 0; i < ROWS; ++i) stopMidiOut (frames, key, i, chbits);
}
void BSEQuencer::stopMidiOut (const int64_t frames, const int key, const int row, const uint8_t chbits)
{
	if ((key >= 0) && (key < ((int) inKeys.size)) && inKeys[key].output[row].playing)
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
	for (int i = 0; i < ROWS; ++i) startMidiOut (frames, key, i, chbits);
}
void BSEQuencer::startMidiOut (const int64_t frames, const int key, const int row, const uint8_t chbits)
{
	if ((key >= 0) && (key < ((int) inKeys.size)) && (chbits & ((uint8_t) inKeys[key].output[row].pad.ch) & 0x0F))
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
	else return fmod (rawstep, int (controllers[NR_OF_STEPS]));
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
	if (end < start) return;

	// Playing or halted?
	if (VALUE_BPM > 0)
	{
		cleanupInKeys ();
		double endpos = startpos + double (end - start) / FRAMES_PER_BEAT;

		// Internal keyboard
		for (int key = 0; key < int (inKeys.size); ++key)
		{
			double nextpos = endpos;
			double lastpos = startpos;

			for (double actpos = startpos; actpos <= endpos; actpos = nextpos)
			{
				int64_t actframes = LIMIT (start + (actpos - startpos) * FRAMES_PER_BEAT, start, end);
				double actstep = getStep (key, actpos - inKeys[key].startPos);
				int actStepNr = LIMIT (int (floor (actstep)), 0, int (controllers[NR_OF_STEPS]) - 1);
				double actStepFrac = actstep - actStepNr;

				// Only present events
				if (actstep >= 0)
				{
					// Just stepped?
					if (inKeys[key].stepNr != actStepNr)
					{

						// fprintf (stderr, "BSEQuencer.lv2: Just stepped (key = %i, actpos = %f, actStepFrac = %f)\n", key, actpos, actStepFrac);

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

	if ((!inputPort) || (!outputPort)) return;

	midiStack.clear ();

	// Init notify port
	uint32_t space = outputPort->atom.size;
	lv2_atom_forge_set_buffer(&output_forge, (uint8_t*) outputPort, space);
	lv2_atom_forge_sequence_head(&output_forge, &output_frame, 0);

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
		CONTROLLER_CHANGED(SCALE) ||
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
		int newScaleNr = LIMIT (*new_controllers[SCALE], 0, NR_SYSTEM_SCALES + NR_USER_SCALES - 1);
		scale.setScale(rtScaleMaps[newScaleNr].scaleNotes);
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
	LV2_ATOM_SEQUENCE_FOREACH(inputPort, ev)
	{
		int64_t act_t = (ev->time.frames <= n_samples ? ev->time.frames : n_samples);

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

			// GUI pad changed notifications
			else if (obj->body.otype == uris.notify_padEvent)
			{
				LV2_Atom *oPd = NULL;
				lv2_atom_object_get (obj, uris.notify_pad,  &oPd, NULL);

				// Pad notification
				if (oPd && (oPd->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oPd;
					if (vec->body.child_type == uris.atom_Float)
					{
						const uint32_t size = (uint32_t) ((oPd->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (PadMessage));
						PadMessage* pMes = (PadMessage*) (&vec->body + 1);

						// Copy PadMessages to pads
						for (unsigned int i = 0; i < size; ++i)
						{
							int row = (int) pMes[i].row;
							int step = (int) pMes[i].step;
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

			// GUI user scales changed notifications
			else if (obj->body.otype == uris.notify_scaleMapsEvent)
			{
				int iD = 0;
				int scaleNr = 0;

				LV2_Atom *oId = NULL, *oName = NULL, *oElements = NULL, *oAltSymbols = NULL, *oScale = NULL;
				lv2_atom_object_get
				(
					obj,
					uris.notify_scaleID,  &oId,
					uris.notify_scaleName, &oName,
					uris.notify_scaleElements, &oElements,
					uris.notify_scaleAltSymbols, &oAltSymbols,
					uris.notify_scale, &oScale,
					NULL
				);

				if (oId && (oId->type == uris.atom_Int))
				{
					iD = ((LV2_Atom_Int*)oId)->body;
					for (int i = 0; i < NR_SYSTEM_SCALES + NR_USER_SCALES; ++i)
					{
						if (iD == rtScaleMaps[i].iD)
						{
							scaleNr = i;
							break;
						}
					}
				}

				if ((scaleNr >= NR_SYSTEM_SCALES) && (scaleNr < NR_SYSTEM_SCALES + NR_USER_SCALES))
				{
					// Name
					if (oName && (oName->type == uris.atom_String))
					{
						strncpy (rtScaleMaps[scaleNr].name, (char*) LV2_ATOM_BODY(oName), 64);
					}

					// Elements TODO safety
					if (oElements && (oElements->type == uris.atom_Vector))
					{
						const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oElements;
						if (vec->body.child_type == uris.atom_Int)
						{
							memcpy (rtScaleMaps[scaleNr].elements, &vec->body + 1, 16 * sizeof (int));
						}
					}

					// Alt Symbols TODO safety
					if (oAltSymbols && (oAltSymbols->type == uris.atom_Vector))
					{
						const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oAltSymbols;
						if (vec->body.child_type == uris.atom_String)
						{
							memcpy (rtScaleMaps[scaleNr].altSymbols, &vec->body + 1, 16 * 16);
						}
					}

					// Scale TODO safety
					if (oScale && (oScale->type == uris.atom_Vector))
					{
						const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oScale;
						if (vec->body.child_type == uris.atom_Int)
						{
							BScaleNotes* notes = (BScaleNotes*) (&vec->body + 1);
							rtScaleMaps[scaleNr].scaleNotes = *notes;
						}
					}

					// Playing scale changed => update and stop output
					if (scaleNr == controllers[SCALE])
					{
						scale.setScale(rtScaleMaps[scaleNr].scaleNotes);
						for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
						{
							if (!midiStopped[i]) stopMidiOut(0, 1 << i);
							midiStopped[i] = true;
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
					LV2_Atom *oBpm = NULL, *oBpb = NULL;
					lv2_atom_object_get
					(
						obj,
						uris.time_beatsPerMinute,  &oBpm,
						uris.time_beatsPerBar,  &oBpb,
						NULL
					);

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

					// Stop MIDI output for all BSEQuencer channels
					if (scheduleStopMidi && (controllers[MODE] != AUTOPLAY))
					{
						for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
						{
							if (!midiStopped[i])
							{
								// fprintf (stderr, "Call stopMidiOut from 'Stop MIDI output for all BSEQuencer channels' at %f\n", position);
								stopMidiOut (act_t, 1 << i);
								midiStopped[i] = true;
							}
						}
					}
				}
			}

			//else fprintf (stderr, "BSEQuencer.lv2: Ignored object in Control port (otype = %i, %s)\n", obj->body.otype,
			//			  (unmap ? unmap->unmap (unmap->handle, obj->body.otype) : NULL));
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
								Key key = defaultKey; // stepNr = -1; direction = 1; output.pads, output.playing and jumpOff ()-initialized
								key.note = note;
								key.velocity = msg[2];
								key.startPos = position + double (act_t) / FRAMES_PER_BEAT - (1 / STEPS_PER_BEAT);
								inKeys.push_back (key);
								// fprintf (stderr, "BSEQuencer.lv2: Key on (frames: %li, note: %i, velocity: %i) at %f\n", act_t, key.note, key.velocity, key.startPos);
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
									// fprintf (stderr, "BSEQuencer.lv2: Key off (frames: %li, note: %i, velocity: %i) at %f + frames\n", act_t, note, msg[2], position);

									stopMidiOut (act_t, i, ALL_CH);
									inKeys.erase (&inKeys.iterator[i]);
									break;
								}
							}
						}
						break;

					// LV2_MIDI_MSG_CONTROLLER
					case LV2_MIDI_MSG_CONTROLLER:
						{
							switch (note)
							{

							// LV2_MIDI_CTL_SUSTAIN: Forward to all outputs
							case LV2_MIDI_CTL_SUSTAIN:
								for (int ch = 1; ch <= NR_SEQUENCER_CHS; ++ch)
								{
									midiStack.append (act_t, ch, LV2_MIDI_MSG_CONTROLLER, LV2_MIDI_CTL_SUSTAIN, msg[2]);
								}
								break;


							// LV2_MIDI_CTL_ALL_SOUNDS_OFF: Stop all outputs
							case LV2_MIDI_CTL_ALL_SOUNDS_OFF:
								for (size_t i = 0; i < inKeys.size; ++i) stopMidiOut (act_t, i, ALL_CH);
								break;

							// LV2_MIDI_CTL_ALL_NOTES_OFF: Stop all outputs and delete all keys
							// As B.SEQuencer doesn't interpret LV2_MIDI_CTL_SUSTAIN itself, the
							// result is the same as in LV2_MIDI_CTL_ALL_SOUNDS_OFF
							case LV2_MIDI_CTL_ALL_NOTES_OFF:
								while (!inKeys.empty())
								{
									stopMidiOut (act_t, inKeys.size - 1, ALL_CH);
									inKeys.pop_back();
								}
								break;

							// All other MIDI signals
							default:
								//fprintf (stderr, "BSEQuencer.lv2: Ignored MIDI_in message in run (): #%i (%i, %i).\n", msg[0], msg[1], msg[2]);
								break;
							}

						}
						break;


					// All other MIDI signals
					default:
						//fprintf (stderr, "BSEQuencer.lv2: Ignored MIDI_in message in run (): #%i (%i, %i).\n", msg[0], msg[1], msg[2]);
						break;
					}
				}
				//else fprintf (stderr, "BSEQuencer.lv2: MIDI input channel filter passed MIDI_in message in run (): #%i (%i, %i).\n", msg[0], msg[1], msg[2]);
			}
		}

		//else fprintf (stderr, "BSEQuencer.lv2: Uninterpreted event in Control port (type = %i, %s)\n", ev->body.type,
		//				 (unmap ? unmap->unmap (unmap->handle, ev->body.type) : NULL));


		// Update for this iteration
		if (controllers[PLAY]) runSequencer (position + double (last_t) / FRAMES_PER_BEAT, last_t, act_t);
		last_t = act_t;
	}

	// AUTOPLAY pseudo MIDI in
	if ((controllers[PLAY]) && (controllers[MODE] == AUTOPLAY))
	{
		// Exactly one inKey needed for autoplay
		// No inKeys => create an empty preliminary key
		if (inKeys.empty())
		{
			Key key = defaultKey;
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
			inKeys[0].startPos = position + double (last_t) / FRAMES_PER_BEAT - (1 / STEPS_PER_BEAT);
		}
	}

	// Update for the remainder of the cycle
	if ((controllers[PLAY]) && (last_t < n_samples)) runSequencer (position + double (last_t) / FRAMES_PER_BEAT, last_t, n_samples);

	//Update position until next time signal from host
	position += double (n_samples) / FRAMES_PER_BEAT;

	scheduleNotifyStatusToGui = true;

	// Send notifications to GUI
	if (ui_on && scheduleNotifyStatusToGui) notifyStatusToGui ();
	if (ui_on && scheduleNotifyPadsToGui) notifyPadsToGui ();
	if (ui_on && scheduleNotifyScaleMapsToGui) notifyScaleMapsToGui ();
	notifyMidi ();
	lv2_atom_forge_pop(&output_forge, &output_frame);
}

LV2_State_Status BSEQuencer::state_save (LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags,
			const LV2_Feature* const* features)
{
	// Store pads
	char padDataString[0x8010] = "Matrix data:\n";

	for (int step = 0; step < MAXSTEPS; ++step)
	{
		for (int row = 0; row < ROWS; ++row)
		{
			char valueString[64];
			int id = step * ROWS + row;
			Pad* pd = &pads[row][step];
			snprintf (valueString, 62, "id:%d; ch:%d; oc:%d; ve:%1.2f; du:%1.2f", id, (int) pd->ch, (int) pd->pitchOctave, pd->velocity, pd->duration);
			if ((step < MAXSTEPS - 1) || (row < ROWS)) strcat (valueString, ";\n");
			else strcat(valueString, "\n");
			strcat (padDataString, valueString);
		}
	}
	store (handle, uris.state_pad, padDataString, strlen (padDataString) + 1, uris.atom_String, LV2_STATE_IS_POD);

	// Store user scales
	std::string mapDataString = "Scale data:\n";

	for (int nr = NR_SYSTEM_SCALES; nr < NR_SYSTEM_SCALES + NR_USER_SCALES; ++nr)
	{
		mapDataString += "id:" + std::to_string (rtScaleMaps[nr].iD) + ";\n";

		std::string namestr = std::string (rtScaleMaps[nr].name);
		while (namestr.find ("\"") != std::string::npos) namestr.replace (namestr.find ("\""), 1, "&quot;");
		mapDataString += "nm:\"" + namestr + "\";\n";

		mapDataString += "el:";
		for (int row = 0; row < ROWS; ++row) mapDataString += std::to_string (rtScaleMaps[nr].elements[row]) + ";";
		mapDataString += "\n";

		mapDataString += "as:";
		for (int row = 0; row < ROWS; ++row)
		{
			std::string altstr = std::string (rtScaleMaps[nr].altSymbols[row]);
			while (altstr.find ("\"") != std::string::npos) altstr.replace (namestr.find ("\""), 1, "&quot;");
			mapDataString += "\"" + altstr + "\";";
		}
		mapDataString += "\n";

		mapDataString += "sc:";
		for (int el = 0; el < 12; ++el) mapDataString += std::to_string (rtScaleMaps[nr].scaleNotes[el]) + ";";
		mapDataString += "\n";
	}
	store (handle, uris.state_scales, mapDataString.c_str(), mapDataString.size() + 1, uris.atom_String, LV2_STATE_IS_POD);

	//fprintf (stderr, "BSEQuencer.lv2: State saved.\n");
	return LV2_STATE_SUCCESS;
}

LV2_State_Status BSEQuencer::state_restore (LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags,
			const LV2_Feature* const* features)
{
	//fprintf (stderr, "BSEQuencer.lv2: state_restore ()\n");

	// Retrieve pad data
	size_t   size;
	uint32_t type;
	uint32_t valflags;
	const void* padData = retrieve(handle, uris.state_pad, &size, &type, &valflags);

	if (padData && (type == uris.atom_String))
	{
		// Stop MIDI out
		stopMidiOut (0, ALL_CH);

		// Clear all MIDI in
		while (!inKeys.empty()) inKeys.pop_back();

		// Restore pads
		// Parse retrieved data
		std::string padDataString = (char*) padData;
		const std::string keywords[5] = {"id:", "ch:", "oc:", "ve:", "du:"};
		while (!padDataString.empty())
		{
			// Look for next "id:"
			size_t strPos = padDataString.find ("id:");
			size_t nextPos = 0;
			if (strPos == std::string::npos) break;	// No "id:" found => end
			if (strPos + 3 > padDataString.length()) break;	// Nothing more after id => end
			padDataString.erase (0, strPos + 3);
			int id;
			try {id = std::stof (padDataString, &nextPos);}
			catch  (const std::exception& e)
			{
				fprintf (stderr, "BSEQuencer.lv2: Restore pad state incomplete. Can't parse ID from \"%s...\"", padDataString.substr (0, 63).c_str());
				break;
			}

			if (nextPos > 0) padDataString.erase (0, nextPos);
			if ((id < 0) || (id >= MAXSTEPS * ROWS))
			{
				fprintf (stderr, "BSEQuencer.lv2: Restore pad state incomplete. Invalid matrix data block loaded with ID %i. Try to use the data before this id.\n", id);
				break;
			}
			int row = id % ROWS;
			int step = id / ROWS;

			// Look for pad data
			for (int i = 1; i < 5; ++i)
			{
				strPos = padDataString.find (keywords[i]);
				if (strPos == std::string::npos) continue;	// Keyword not found => next keyword
				if (strPos + 3 >= padDataString.length())	// Nothing more after keyword => end
				{
					padDataString ="";
					break;
				}
				if (strPos > 0) padDataString.erase (0, strPos + 3);
				float val;
				try {val = std::stof (padDataString, &nextPos);}
				catch  (const std::exception& e)
				{
					fprintf (stderr, "BSEQuencer.lv2: Restore padstate incomplete. Can't parse %s from \"%s...\"",
							 keywords[i].substr(0,2).c_str(), padDataString.substr (0, 63).c_str());
					break;
				}

				if (nextPos > 0) padDataString.erase (0, nextPos);
				switch (i) {
				case 1: pads[row][step].ch = val;
						break;
				case 2: pads[row][step].pitchOctave = val;
						break;
				case 3:	pads[row][step].velocity = val;
						break;
				case 4:	pads[row][step].duration = val;
						break;
				default:break;
				}
			}
		}


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

	// Restore user scales
	const void* scaleData = retrieve(handle, uris.state_scales, &size, &type, &valflags);

	if (scaleData && (type == uris.atom_String))
	{
		std::string scaleDataString = (char*) scaleData;
		const std::string keywords[5] = {"id:", "nm:", "el:", "as:", "sc:"};
		while (!scaleDataString.empty())
		{
			// Look for next "id:"
			size_t strPos = scaleDataString.find ("id:");
			size_t nextPos = 0;
			if (strPos == std::string::npos) break;	// No "id:" found => end
			if (strPos + 3 > scaleDataString.length()) break;	// Nothing more after id => end
			scaleDataString.erase (0, strPos + 3);

			int id = -1;
			int scaleNr = -1;

			try {id = std::stof (scaleDataString, &nextPos);}
			catch  (const std::exception& e)
			{
				fprintf (stderr, "BSEQuencer.lv2: Restore scale map state incomplete. Can't parse ID from \"%s...\"", scaleDataString.substr (0, 63).c_str());
				break;
			}

			if (nextPos > 0) scaleDataString.erase (0, nextPos);
			if ((id < 0) || (id >= NR_SYSTEM_SCALES + NR_USER_SCALES))
			{
				fprintf (stderr, "BSEQuencer.lv2: Restore scale map state incomplete. Invalid scale data block loaded with ID %i. Try to use the data before this id.\n", id);
				break;
			}

			// iD to index
			for (int i = 0; i < NR_SYSTEM_SCALES + NR_USER_SCALES; ++i)
			{
				if (id == rtScaleMaps[i].iD)
				{
					scaleNr = i;
					break;
				}
			}
			if (scaleNr < 0)
			{
				fprintf (stderr, "BSEQuencer.lv2: Restore scale map state incomplete. Invalid scale data block loaded with ID %i. Try to use the data before this id.\n", id);
				break;
			}

			// Look for scale data
			for (int i = 1; i < 5; ++i)
			{
				strPos = scaleDataString.find (keywords[i]);
				if (strPos == std::string::npos) continue;	// Keyword not found => next keyword
				if (strPos + 3 >= scaleDataString.length())	// Nothing more after keyword => end
				{
					scaleDataString ="";
					break;
				}
				if (strPos > 0) scaleDataString.erase (0, strPos + 3);
				switch (i) {
				case 1: {
							nextPos = scaleDataString.find ("\"");
							scaleDataString.erase (0, nextPos + 1);
							nextPos = scaleDataString.find ("\"");
							std::string namestr = scaleDataString.substr (0, nextPos);
							while (namestr.find ("&quot") != std::string::npos) namestr.replace (namestr.find ("&quot;"), 1, "\"");
							strncpy (rtScaleMaps[scaleNr].name, namestr.c_str(), 63);
							scaleDataString.erase (0, nextPos + 1);
						}
						break;
				case 2:	{
							int el;

							for (int i = 0; i < ROWS; ++i)
							{
								try
								{
									el = std::stoi (scaleDataString, &nextPos);
								}
								catch (const std::exception& e)
								{
									fprintf (stderr, "BSEQuencer.lv2: Restore scale map state incomplete. Incomplete scale data block loaded with ID %i.\n", id);
									break;
								}

								rtScaleMaps[scaleNr].elements[i] = el;
								scaleDataString.erase (0, nextPos);
								nextPos = scaleDataString.find (";");
								scaleDataString.erase (0, nextPos + 1);
							}

						}
						break;
				case 3: {
							for (int i = 0; i < ROWS; ++i)
							{
								nextPos = scaleDataString.find ("\"");

								// Check if a string is following or a keyword
								size_t keyPos = std::string::npos;
								for (int j = 0; j < 5; ++j)
								{
									size_t pos = scaleDataString.find (keywords[j]);
									if (pos < keyPos) keyPos = pos;
								}

								// No string => break
								if (keyPos < nextPos)
								{
									fprintf (stderr, "BSEQuencer.lv2: Restore scale map state incomplete. Incomplete scale data block loaded with ID %i.\n", id);
									break;
								}

								// Copy string contents
								scaleDataString.erase (0, nextPos + 1);
								nextPos = scaleDataString.find ("\"");
								std::string altstr = scaleDataString.substr (0, nextPos);
								while (altstr.find ("&quot") != std::string::npos) altstr.replace (altstr.find ("&quot;"), 1, "\"");
								strncpy (rtScaleMaps[scaleNr].altSymbols[i], altstr.c_str(), 15);
								scaleDataString.erase (0, nextPos + 1);
							}
						}
						break;
				case 4:	{
							int sc;

							for (int i = 0; i < 12; ++i)
							{
								try
								{
									sc = std::stoi (scaleDataString, &nextPos);
								}
								catch (const std::exception& e)
								{
									fprintf (stderr, "BSEQuencer.lv2: Restore scale map state incomplete. Incomplete scale data block loaded with ID %i.\n", id);
									break;
								}

								rtScaleMaps[scaleNr].scaleNotes[i] = sc;
								scaleDataString.erase (0, nextPos);
								nextPos = scaleDataString.find (";");
								scaleDataString.erase (0, nextPos + 1);
							}
						}
						break;
				default:break;
				}
			}
		}
	}

	// Force GUI notification
	scheduleNotifyStatusToGui = true;

	return LV2_STATE_SUCCESS;
}

void BSEQuencer::activate ()
{
	inKeys.clear ();
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

void BSEQuencer::notifyPadsToGui ()
{
	PadMessage endmsg (ENDPADMESSAGE);
	if (!(endmsg == padMessageBuffer[0]))
	{
		// Get padMessageBuffer size
		int end = 0;
		for (int i = 0; (i < ROWS * MAXSTEPS) && (!(padMessageBuffer[i] == endmsg)); ++i) end = i;

		// Prepare forge buffer and initialize atom sequence

		LV2_Atom_Forge_Frame frame;
		lv2_atom_forge_frame_time(&output_forge, 0);
		lv2_atom_forge_object(&output_forge, &frame, 0, uris.notify_padEvent);
		lv2_atom_forge_key(&output_forge, uris.notify_pad);
		lv2_atom_forge_vector(&output_forge, sizeof(float), uris.atom_Float, sizeof(PadMessage) / sizeof(float) * (end + 1), (void*) padMessageBuffer);
		lv2_atom_forge_pop(&output_forge, &frame);

		// Empty padMessageBuffer
		padMessageBuffer[0] = endmsg;

		scheduleNotifyPadsToGui = false;
	}
}

void BSEQuencer::notifyStatusToGui ()
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

	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time(&output_forge, 0);
	lv2_atom_forge_object(&output_forge, &frame, 0, uris.notify_statusEvent);
	lv2_atom_forge_key(&output_forge, uris.notify_cursors);
	lv2_atom_forge_vector(&output_forge, sizeof (int), uris.atom_Int, MAXSTEPS, (void*) cursorbits);
	lv2_atom_forge_key(&output_forge, uris.notify_notes);
	lv2_atom_forge_int(&output_forge, notebits);
	lv2_atom_forge_key(&output_forge, uris.notify_channels);
	lv2_atom_forge_int(&output_forge, chbits);
	lv2_atom_forge_pop(&output_forge, &frame);

	scheduleNotifyStatusToGui = false;
}

void BSEQuencer::notifyScaleMapsToGui ()
{
	for (int i = NR_SYSTEM_SCALES; i < NR_SYSTEM_SCALES + NR_USER_SCALES; ++i)
	{
		LV2_Atom_Forge_Frame frame;
		lv2_atom_forge_frame_time(&output_forge, 0);
		lv2_atom_forge_object(&output_forge, &frame, 0, uris.notify_scaleMapsEvent);
		lv2_atom_forge_key(&output_forge, uris.notify_scaleID);
		lv2_atom_forge_int(&output_forge, rtScaleMaps[i].iD);
		lv2_atom_forge_key(&output_forge, uris.notify_scaleName);
		lv2_atom_forge_string (&output_forge, rtScaleMaps[i].name, 64);
		lv2_atom_forge_key(&output_forge, uris.notify_scaleElements);
		lv2_atom_forge_vector(&output_forge, sizeof (int), uris.atom_Int, 16, (void*) rtScaleMaps[i].elements);
		lv2_atom_forge_key(&output_forge, uris.notify_scaleAltSymbols);
		lv2_atom_forge_vector(&output_forge, 16, uris.atom_String, 16, (void*) rtScaleMaps[i].altSymbols);
		lv2_atom_forge_key(&output_forge, uris.notify_scale);
		BScaleNotes* notes = &rtScaleMaps[i].scaleNotes;
		lv2_atom_forge_vector(&output_forge, sizeof (int), uris.atom_Int, 12, (void*) notes);
		lv2_atom_forge_pop(&output_forge, &frame);

		scheduleNotifyScaleMapsToGui = false;
	}
}

void BSEQuencer::notifyMidi ()
{
	for (size_t i = 0; i < midiStack.size (); ++i)
	{
		MidiData& midiData = midiStack[i];
		// ch -> MIDI channel
		int channel = controllers[CH + (midiData.ch - 1) * CH_SIZE + MIDI_CHANNEL] - 1;

		// compose MIDI message block
		LV2_Atom midiatom;
		midiatom.type = uris.midi_Event;
		midiatom.size = 3;

		uint8_t msg[3];
		msg[0] = midiData.status + channel;
		msg[1] = midiData.note;
		msg[2] = midiData.velocity;

		// send MIDI message
		if (!lv2_atom_forge_frame_time (&output_forge, midiData.frames)) return;
		if (!lv2_atom_forge_raw (&output_forge, &midiatom, sizeof (LV2_Atom))) return;
		if (!lv2_atom_forge_raw (&output_forge, &msg, 3)) return;
		lv2_atom_forge_pad (&output_forge, sizeof (LV2_Atom) + 3);
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

static void activate (LV2_Handle instance)
{
	BSEQuencer* inst = (BSEQuencer*)instance;
	inst->activate ();
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
		activate,
		run,
		NULL,	// deactivate
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
