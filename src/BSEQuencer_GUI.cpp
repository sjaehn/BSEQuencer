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

#include "BSEQuencer_GUI.hpp"

BSEQuencer_GUI::BSEQuencer_GUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeWindow parentWindow) :
		Window (1040, 840, "B.SEQuencer", parentWindow),
		cursorBits (0), noteBits (0), chBits (0), tempTool (false), tempToolCh (0),
		pluginPath (bundle_path ? std::string (bundle_path) : std::string ("")), controller (NULL), write_function (NULL),
		map (NULL),
		mContainer (0, 0, 1040, 840, "main"),
		padSurface (98, 88, 644, 484, "box"),
		captionSurface (18, 88, 64, 484, "box"),

		modeBox (760, 88, 260, 205, "box"),
		modeBoxLabel (10, 10, 240, 20, "ctlabel", "Play mode"),
		modeLabel (10, 90, 60, 20, "lflabel", "Mode"),
		modeListBox (80, 90, 170, 20, 170, 60, "menu", std::vector<std::string> {"Autoplay", "Host controlled"}, 2.0),
		modeAutoplayBpmLabel (10, 130, 120, 20, "lflabel", "Beats per min"),
		modeAutoplayBpmSlider (120, 120, 130, 25, "slider", 120.0, 1.0, 300.0, 0.0, "%3.1f"),
		modeAutoplayBpbLabel (10, 170, 120, 20, "lflabel", "Beats per bar"),
		modeAutoplayBpbSlider (120, 160, 130, 25, "slider", 4.0, 1.0, 16.0, 1.0, "%2.0f"),
		modeAutoplayBoxScreen (5, 115, 250, 85, "screen"),
		modePlayLabel (10, 50, 205, 20, "lflabel", "Status: playing ..."),
		modePlayButton (210, 40, 40, 40, "box", 1.0),

		toolBox (760, 315, 260, 257, "box"),
		toolBoxLabel (10, 10, 240, 20, "ctlabel", "Toolbox"),
		toolButtonBox (0, 40, 260, 100, "widget"),
		toolButtonBoxCtrlLabel (10, 10, 60, 20, "lflabel", "Controls"),
		toolButtonBoxChLabel (10, 70, 60, 20, "lflabel", "Notes"),
		toolOctaveLabel (30, 225, 60, 20, "lflabel", "Octave"),
		toolOctaveDial (35, 165, 50, 60, "dial", 0.0, -8.0, 8.0, 1.0, "%1.0f"),
		toolVelocityLabel (100, 225, 60, 20, "ctlabel", "Velocity"),
		toolVelocityDial  (105, 165, 50, 60, "dial", 1.0, 0.0, 2.0, 0.0, "%1.2f"),
		toolDurationLabel (170, 225, 60, 20, "ctlabel", "Duration"),
		toolDurationDial (175, 165, 50, 60, "dial", 1.0, 0.0, 1.0, 0.0, "%1.2f"),

		propertiesBox (760, 590, 260, 180, "box"),
		propertiesBoxLabel (10, 10, 240, 20, "ctlabel", "Properties"),
		propertiesNrStepsSlider (10, 40, 80, 25, "slider", 4.0, 1.0, 8.0, 1.0, "%2.0f"),
		propertiesNrStepsLabel (100, 50, 80, 20, "lflabel", "steps per"),
		propertiesBaseListBox (190, 50, 60, 20, 60, 60, "menu", std::vector<std::string> {"beat", "bar"}, 1.0),
		propertiesRootLabel (10, 80, 40, 20, "lflabel", "Root"),
		propertiesRootListBox (120, 80, 60, 20, 0, -160, 60, 160, "menu", {{0, "C"}, {2, "D"}, {4, "E"}, {5, "F"}, {7, "G"}, {9, "A"}, {11, "B"}}, 0.0),
		propertiesSignatureListBox (190, 80, 60, 20, 60, 80, "menu", {{-1, "♭"}, {0, ""}, {1, "♯"}}, 0.0),
		propertiesOctaveLabel (10, 110, 55, 20, "lflabel", "Octave"),
		propertiesOctaveListBox (190, 110, 60, 20, 0, -220, 60, 220, "menu", {{-1, "-1"}, {0, "0"}, {1, "1"}, {2, "2"}, {3, "3"}, {4, "4"}, {5, "5"}, {6, "6"}, {7, "7"}, {8, "8"}}, 4.0),
		propertiesScaleLabel (10, 140, 50, 20, "lflabel", "Scale"),
		propertiesScaleListBox (80, 140, 170, 20, 0, -300, 170, 300, "menu", scaleLabels, 1.0),

		helpLabel (770, 790, 30, 30, "ilabel", "?")

{
	// Init toolbox buttons
	toolButtonBox.addButton (80, 70, 20, 20, {{0.1, 0.1, 0.1, 1.0}, ""});
	for (int i = 1; i < NR_SEQUENCER_CHS + 1; ++i) toolButtonBox.addButton (80 + i * 30, 70, 20, 20, chButtonStyles[i]);
	toolButtonBox.addButton (80 , 10, 20, 20, {{0.1, 0.1, 0.1, 1.0}, ""});
	for (int i = 1; i < NR_CTRL_BUTTONS; ++i) toolButtonBox.addButton (80 + (i % 6) * 30, 10 + ((int) (i / 6)) * 30, 20, 20, ctrlButtonStyles[i]);

	// Init ChBoxes
	for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
	{
		chBoxes[i].box = BWidgets::Widget (98 + i * 163.5, 590, 153.5, 230, "box");
		chBoxes[i].box.rename ("box");
		chBoxes[i].chSymbol = BWidgets::DrawingSurface (7, 7, 26, 26, "button");
		chBoxes[i].chSymbol.rename ("button");
		chBoxes[i].chLabel = BWidgets::Label (40, 10, 40, 20, "ctlabel", "CH " + std::to_string (i + 1));
		chBoxes[i].chLabel.rename ("ctlabel");
		chBoxes[i].portLabel = BWidgets::Label (10, 40, 80, 20, "lflabel", "MIDI port");
		chBoxes[i].portLabel.rename ("lflabel");
		chBoxes[i].portListBox = BWidgets::PopupListBox (93.5, 40, 50, 20, 50, 100, "menu", {"1", "2", "3", "4"}, i);
		chBoxes[i].portListBox.rename ("menu");
		chBoxes[i].channelLabel = BWidgets::Label (10, 70, 80, 20, "lflabel", "MIDI channel");
		chBoxes[i].channelLabel.rename ("lflabel");
		chBoxes[i].channelListBox = BWidgets::PopupListBox (93.5, 70, 50, 20, 50, 120, "menu", {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"}, 1.0);
		chBoxes[i].channelListBox.rename ("menu");
		chBoxes[i].pitchLabel = BWidgets::Label (10, 100, 80, 20, "lflabel", "Input pitch");
		chBoxes[i].pitchLabel.rename ("lflabel");
		chBoxes[i].pitchSwitch = BWidgets::HSwitch (102.5, 102, 32, 16, "slider", 0.0);
		chBoxes[i].pitchSwitch.rename ("ch" + std::to_string (i + 1));
		chBoxes[i].pitchScreen = BWidgets::Widget (10, 100, 133.5, 20, "screen");
		chBoxes[i].pitchScreen.rename ("screen");
		chBoxes[i].pitchScreen.hide ();
		chBoxes[i].velocityDial = BWidgets::DisplayDial (15, 140, 50, 60, "dial", 1.0, 0.0, 2.0, 0.0, "%1.2f");
		chBoxes[i].velocityDial.rename ("ch" + std::to_string (i + 1));
		chBoxes[i].velocityLabel = BWidgets::Label (10, 200, 60, 20, "ctlabel", "Velocity");
		chBoxes[i].velocityLabel.rename ("ctlabel");
		chBoxes[i].noteOffsetDial = BWidgets::DisplayDial (88.5, 140, 50, 60, "dial", 0.0, -127.0, 127.0, 1.0, "%3.0f");
		chBoxes[i].noteOffsetDial.rename ("ch" + std::to_string (i + 1));
		chBoxes[i].noteOffsetLabel = BWidgets::Label (83.5, 200, 60, 20, "ctlabel", "Offset");
		chBoxes[i].noteOffsetLabel.rename ("ctlabel");
	}

	// Link controllerWidgets
	controllerWidgets[PLAY] = (BWidgets::ValueWidget*) &modePlayButton;
	controllerWidgets[MODE] = (BWidgets::ValueWidget*) &modeListBox;
	controllerWidgets[NR_STEPS] = (BWidgets::ValueWidget*) &propertiesNrStepsSlider;
	controllerWidgets[BASE] = (BWidgets::ValueWidget*) &propertiesBaseListBox;
	controllerWidgets[ROOT] = (BWidgets::ValueWidget*) &propertiesRootListBox;
	controllerWidgets[SIGNATURE] = (BWidgets::ValueWidget*) &propertiesSignatureListBox;
	controllerWidgets[OCTAVE] = (BWidgets::ValueWidget*) &propertiesOctaveListBox;
	controllerWidgets[SCALE] = (BWidgets::ValueWidget*) &propertiesScaleListBox;
	controllerWidgets[AUTOPLAY_BPM] = (BWidgets::ValueWidget*) &modeAutoplayBpmSlider;
	controllerWidgets[AUTOPLAY_BPB] = (BWidgets::ValueWidget*) &modeAutoplayBpbSlider;
	controllerWidgets[SELECTION_CH] = (BWidgets::ValueWidget*) &toolButtonBox;
	controllerWidgets[SELECTION_OCTAVE] = (BWidgets::ValueWidget*) &toolOctaveDial;
	controllerWidgets[SELECTION_VELOCITY] = (BWidgets::ValueWidget*) &toolVelocityDial;
	controllerWidgets[SELECTION_DURATION] = (BWidgets::ValueWidget*) &toolDurationDial;

	for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
	{
		controllerWidgets[CH + i * CH_SIZE + PITCH] = (BWidgets::ValueWidget*) &chBoxes[i].pitchSwitch;
		controllerWidgets[CH + i * CH_SIZE + VELOCITY] = (BWidgets::ValueWidget*) &chBoxes[i].velocityDial;
		controllerWidgets[CH + i * CH_SIZE + MIDI_PORT] = (BWidgets::ValueWidget*) &chBoxes[i].portListBox;
		controllerWidgets[CH + i * CH_SIZE + MIDI_CHANNEL] = (BWidgets::ValueWidget*) &chBoxes[i].channelListBox;
		controllerWidgets[CH + i * CH_SIZE + NOTE_OFFSET] = (BWidgets::ValueWidget*) &chBoxes[i].noteOffsetDial;
	}

	// Set callback functions
	for (int i = 0; i < KNOBS_SIZE; ++i) controllerWidgets[i]->setCallbackFunction (BEvents::VALUE_CHANGED_EVENT, valueChangedCallback);
	padSurface.setCallbackFunction (BEvents::BUTTON_PRESS_EVENT, padsPressedCallback);
	padSurface.setCallbackFunction (BEvents::BUTTON_RELEASE_EVENT, padsPressedCallback);
	padSurface.setDragable (true);
	padSurface.setCallbackFunction (BEvents::POINTER_MOTION_WHILE_BUTTON_PRESSED_EVENT, padsPressedCallback);
	helpLabel.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, helpPressedCallback);


	// Apply theme
	widgetBg = BStyles::Fill (pluginPath + BG_FILE);
	mContainer.applyTheme (theme);
	padSurface.applyTheme (theme);
	captionSurface.applyTheme (theme);

	modeBox.applyTheme (theme);
	modeBoxLabel.applyTheme (theme);
	modeBoxLabel.setState (BColors::ACTIVE);
	modeLabel.applyTheme (theme);
	modeListBox.applyTheme (theme);
	modeAutoplayBpmLabel.applyTheme (theme);
	modeAutoplayBpmSlider.applyTheme (theme);
	modeAutoplayBpbLabel.applyTheme (theme);
	modeAutoplayBpbSlider.applyTheme (theme);
	modeAutoplayBoxScreen.applyTheme (theme);
	modePlayLabel.applyTheme (theme);
	modePlayButton.applyTheme (theme);

	toolBox.applyTheme (theme);
	toolBoxLabel.applyTheme (theme);
	toolBoxLabel.setState (BColors::ACTIVE);
	toolButtonBox.applyTheme (theme);
	toolButtonBoxCtrlLabel.applyTheme (theme);
	toolButtonBoxChLabel.applyTheme (theme);
	toolOctaveLabel.applyTheme (theme);
	toolOctaveDial.applyTheme (theme);
	toolVelocityLabel.applyTheme (theme);
	toolVelocityDial.applyTheme (theme);
	toolDurationLabel.applyTheme (theme);
	toolDurationDial.applyTheme (theme);

	propertiesBox.applyTheme (theme);
	propertiesBoxLabel.applyTheme (theme);
	propertiesBoxLabel.setState (BColors::ACTIVE);
	propertiesNrStepsLabel.applyTheme (theme);
	propertiesNrStepsSlider.applyTheme (theme);
	propertiesBaseListBox.applyTheme (theme);
	propertiesRootLabel.applyTheme (theme);
	propertiesRootListBox.applyTheme (theme);
	propertiesSignatureListBox.applyTheme (theme);
	propertiesOctaveLabel.applyTheme (theme);
	propertiesOctaveListBox.applyTheme (theme);
	propertiesScaleLabel.applyTheme (theme);
	propertiesScaleListBox.applyTheme (theme);

	for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
	{
		chBoxes[i].box.applyTheme (theme);
		chBoxes[i].chSymbol.applyTheme (theme);
		drawButton (chBoxes[i].chSymbol.getDrawingSurface(), 0, 0, 20, 20, chButtonStyles[i + 1]);
		chBoxes[i].chLabel.applyTheme (theme);
		chBoxes[i].chLabel.setState (BColors::ACTIVE);
		chBoxes[i].portLabel.applyTheme (theme);
		chBoxes[i].portListBox.applyTheme (theme);
		chBoxes[i].channelLabel.applyTheme (theme);
		chBoxes[i].channelListBox.applyTheme (theme);
		chBoxes[i].pitchLabel.applyTheme (theme);
		chBoxes[i].pitchSwitch.applyTheme (theme);
		chBoxes[i].pitchScreen.applyTheme (theme);
		chBoxes[i].velocityDial.applyTheme (theme);
		chBoxes[i].velocityLabel.applyTheme (theme);
		chBoxes[i].noteOffsetDial.applyTheme (theme);
		chBoxes[i].noteOffsetLabel.applyTheme (theme);
	}

	helpLabel.applyTheme (theme);

	// Pack widgets
	mContainer.applyTheme (theme);
	padSurface.applyTheme (theme);
	captionSurface.applyTheme (theme);

	modeBox.add (modeBoxLabel);
	modeBox.add (modeLabel);
	modeBox.add (modeListBox);
	modeBox.add (modeAutoplayBpmLabel);
	modeBox.add (modeAutoplayBpmSlider);
	modeBox.add (modeAutoplayBpbLabel);
	modeBox.add (modeAutoplayBpbSlider);
	modeBox.add (modeAutoplayBoxScreen);
	modeBox.add (modePlayLabel);
	modeBox.add (modePlayButton);

	toolBox.add (toolBoxLabel);
	toolBox.add (toolButtonBox);

	toolButtonBox.add (toolButtonBoxCtrlLabel);
	toolButtonBox.add (toolButtonBoxChLabel);

	toolBox.add (toolOctaveLabel);
	toolBox.add (toolOctaveDial);
	toolBox.add (toolVelocityLabel);
	toolBox.add (toolVelocityDial);
	toolBox.add (toolDurationLabel);
	toolBox.add (toolDurationDial);

	propertiesBox.add (propertiesBoxLabel);
	propertiesBox.add (propertiesNrStepsLabel);
	propertiesBox.add (propertiesNrStepsSlider);
	propertiesBox.add (propertiesBaseListBox);
	propertiesBox.add (propertiesRootLabel);
	propertiesBox.add (propertiesRootListBox);
	propertiesBox.add (propertiesSignatureListBox);
	propertiesBox.add (propertiesOctaveLabel);
	propertiesBox.add (propertiesOctaveListBox);
	propertiesBox.add (propertiesScaleLabel);
	propertiesBox.add (propertiesScaleListBox);

	for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
	{
		chBoxes[i].box.add (chBoxes[i].chSymbol);
		chBoxes[i].box.add (chBoxes[i].chLabel);
		chBoxes[i].box.add (chBoxes[i].portLabel);
		chBoxes[i].box.add (chBoxes[i].portListBox);
		chBoxes[i].box.add (chBoxes[i].channelLabel);
		chBoxes[i].box.add (chBoxes[i].channelListBox);
		chBoxes[i].box.add (chBoxes[i].pitchLabel);
		chBoxes[i].box.add (chBoxes[i].pitchSwitch);
		chBoxes[i].box.add (chBoxes[i].pitchScreen);
		chBoxes[i].box.add (chBoxes[i].velocityDial);
		chBoxes[i].box.add (chBoxes[i].velocityLabel);
		chBoxes[i].box.add (chBoxes[i].noteOffsetDial);
		chBoxes[i].box.add (chBoxes[i].noteOffsetLabel);
	}

	mContainer.add (padSurface);
	mContainer.add (captionSurface);
	mContainer.add (modeBox);
	mContainer.add (toolBox);
	mContainer.add (propertiesBox);
	mContainer.add (helpLabel);
	for (int i = 0; i < NR_SEQUENCER_CHS; ++i) mContainer.add (chBoxes[i].box);

	add (mContainer);

	//Scan host features for URID map
	LV2_URID_Map* m = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp(features[i]->URI, LV2_URID__map) == 0)
		{
			m = (LV2_URID_Map*) features[i]->data;
		}
	}
	if (!m) throw std::invalid_argument ("Host does not support urid:map");

	//Map URIS
	map = m;
	getURIs (map, &uris);

	// Initialize forge
	lv2_atom_forge_init (&forge,map);
}

void BSEQuencer_GUI::port_event(uint32_t port, uint32_t buffer_size,
	uint32_t format, const void* buffer)
{
	// Notify port
	if ((format == uris.atom_eventTransfer) && (port == NOTIFY))
	{
		const LV2_Atom* atom = (const LV2_Atom*) buffer;
		if ((atom->type == uris.atom_Blank) || (atom->type == uris.atom_Object))
		{
			const LV2_Atom_Object* obj = (const LV2_Atom_Object*) atom;
			if (obj->body.otype == uris.notify_Event)
			{
				LV2_Atom *oPad = NULL, *oCursors = NULL, *oNotes = NULL, *oChs = NULL;
				lv2_atom_object_get(obj, uris.notify_pad, &oPad,
										 uris.notify_cursors, &oCursors,
										 uris.notify_notes, &oNotes,
										 uris.notify_channels, &oChs,
										 NULL);

				// Pad notification
				if (oPad && (oPad->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oPad;
					if (vec->body.child_type == uris.atom_Float)
					{
						uint32_t size = (uint32_t) ((oPad->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (int));
						PadMessage* pMes = (PadMessage*)(&vec->body + 1);
						for (int i = 0; i < size; ++i)
						{
							int step = (int) pMes[i].step;
							int row = (int) pMes[i].row;
							if ((step >= 0) && (step < STEPS) && (row >= 0) && (row < ROWS))
							{
								Pad pd (pMes->ch, pMes->pitchOctave, pMes->velocity, pMes->duration);
								pads[row][step] = pd;
							}
						}

						drawPad ();
					}
				}

				// Cursor notifications
				if (oCursors && (oCursors->type == uris.atom_Int))
				{
					cursorBits = ((LV2_Atom_Int*)oCursors)->body;
					drawPad ();
				}

				// Note notifications
				if (oNotes && (oNotes->type == uris.atom_Int))
				{
					noteBits = ((LV2_Atom_Int*)oNotes)->body;
					drawCaption ();
				}

				// BSEQuencer channel (CH) notifications
				if (oChs && (oChs->type == uris.atom_Int))
				{
					chBits = ((LV2_Atom_Int*)oChs)->body;
					for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
					{
						if ((1 << i) & chBits) chBoxes[i].chLabel.setTextColors (ltColors);
						else chBoxes[i].chLabel.setTextColors (txColors);
					}
				}
			}
		}
	}

	// Scan remaining ports
	else if ((format == 0) && (port >= KNOBS))
	{
		float* pval = (float*) buffer;
		controllerWidgets[port-KNOBS]->setValue (*pval);
		controllers[port-KNOBS] = *pval;
	}

}

void BSEQuencer_GUI::send_ui_on ()
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.ui_on);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, CONTROL, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BSEQuencer_GUI::send_ui_off ()
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.ui_off);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, CONTROL, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BSEQuencer_GUI::send_pad (int row, int step)
{
	Pad* pd = &pads[row][step];
	PadMessage padmsg (step, row, pd->ch, pd->pitchOctave, pd->velocity, pd->duration);

	uint8_t obj_buf[128];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.notify_Event);
	lv2_atom_forge_key(&forge, uris.notify_pad);
	lv2_atom_forge_vector(&forge, sizeof(float), uris.atom_Float, sizeof(PadMessage) / sizeof(float), (void*) &padmsg);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, CONTROL, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BSEQuencer_GUI::valueChangedCallback(BEvents::Event* event)
{
	if ((event) && (event->getWidget ()))
	{
		BWidgets::ValueWidget* widget = (BWidgets::ValueWidget*) event->getWidget ();
		float value = widget->getValue();

		if (widget->getMainWindow())
		{
			BSEQuencer_GUI* ui = (BSEQuencer_GUI*) widget->getMainWindow();
			int widgetNr = -1;

			for (int i = 0; i < KNOBS_SIZE; ++i)
			{
				if (widget == ui->controllerWidgets[i]) widgetNr = i;
			}

			if (widgetNr >= 0)
			{
				ui->controllers[widgetNr] = value;
				ui->write_function(ui->controller, KNOBS + widgetNr, sizeof(float), 0, &ui->controllers[widgetNr]);

				// Playing status changed
				if (widgetNr == PLAY)
				{
					if (value) ui->modePlayLabel.setText ("Status: playing ...");
					else ui->modePlayLabel.setText ("Status: stopped !");
				}

				// Mode changed
				if (widgetNr == MODE)
				{
					if (value == AUTOPLAY)
					{
						ui->modeAutoplayBoxScreen.hide ();
						for (int i = 0; i < NR_SEQUENCER_CHS; ++i) ui->chBoxes[i].pitchScreen.show ();

					}
					else
					{
						ui->modeAutoplayBoxScreen.show ();
						for (int i = 0; i < NR_SEQUENCER_CHS; ++i) ui->chBoxes[i].pitchScreen.hide ();
					}
				}

				// GUI changes
				if ((widgetNr == ROOT) || (widgetNr == SIGNATURE) || (widgetNr == SCALE))
				{
					ui->drawPad ();
					ui->drawCaption ();
				}

				if (widgetNr == NR_STEPS) ui->drawPad ();

			}
		}
	}
}

void BSEQuencer_GUI::helpPressedCallback (BEvents::Event* event) {system(OPEN_CMD " " HELP_URL);}

void BSEQuencer_GUI::padsPressedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()) && (((BWidgets::Widget*)(event->getWidget()))->getMainWindow()) &&
		((event->getEventType () == BEvents::BUTTON_PRESS_EVENT) ||
		 (event->getEventType () == BEvents::BUTTON_RELEASE_EVENT) ||
		 (event->getEventType () == BEvents::POINTER_MOTION_WHILE_BUTTON_PRESSED_EVENT)))
	{
		BWidgets::DrawingSurface* widget = (BWidgets::DrawingSurface*) event->getWidget ();
		BSEQuencer_GUI* ui = (BSEQuencer_GUI*) widget->getMainWindow();
		BEvents::PointerEvent* pointerEvent = (BEvents::PointerEvent*) event;

		int row = 15 - (int) ((pointerEvent->getY () - widget->getYOffset()) / PAD_HEIGHT);
		int step = (pointerEvent->getX () - widget->getXOffset()) / PAD_WIDTH;

		if ((row >= 0) && (row < ROWS) && (step >= 0) && (step < STEPS))
		{
			Pad* pd = &ui->pads[row][step];

			// Left button: apply properties to pad
			if (pointerEvent->getButton() == BEvents::LEFT_BUTTON)
			{
				// CH output set: Limit to note area
				if ((row < ROWS - 1) && (ui->controllerWidgets[SELECTION_CH]->getValue() <= NR_SEQUENCER_CHS))
				{
					if ((event->getEventType () == BEvents::BUTTON_PRESS_EVENT) ||
						(event->getEventType () == BEvents::POINTER_MOTION_WHILE_BUTTON_PRESSED_EVENT))
					{
						Pad props (ui->controllerWidgets[SELECTION_CH]->getValue(),
								   ui->controllerWidgets[SELECTION_OCTAVE]->getValue(),
								   ui->controllerWidgets[SELECTION_VELOCITY]->getValue(),
								   ui->controllerWidgets[SELECTION_DURATION]->getValue());

						// Click on a pad with same settings as in toolbox => temporarily switch to delete
						if ((props == *pd) && (!ui->tempTool) && (event->getEventType () == BEvents::BUTTON_PRESS_EVENT))
						{
							ui->tempTool = true;
							ui->tempToolCh = props.ch;
							props.ch = 0;
							ui->controllerWidgets[SELECTION_CH]->setValue(props.ch);
						}

						// Overwrite if new data
						if (!(props == *pd))
						{
							*pd = props;
							ui->drawPad (row, step);
							ui->send_pad (row, step);
						}
					}
				}

				// CTRL function set: Limit to CTRL area
				else if ((row == ROWS - 1) && (ui->controllerWidgets[SELECTION_CH]->getValue() > NR_SEQUENCER_CHS))
				{
					if ((event->getEventType () == BEvents::BUTTON_PRESS_EVENT) ||
						(event->getEventType () == BEvents::POINTER_MOTION_WHILE_BUTTON_PRESSED_EVENT))
					{
						// Click on a pad with same settings as in toolbox => temporarily switch to delete
						if ((pd->ch == ui->controllerWidgets[SELECTION_CH]->getValue()) && (!ui->tempTool) &&
							(event->getEventType () == BEvents::BUTTON_PRESS_EVENT))
						{
							ui->tempTool = true;
							ui->tempToolCh = ui->controllerWidgets[SELECTION_CH]->getValue();
							ui->controllerWidgets[SELECTION_CH]->setValue(NR_SEQUENCER_CHS + 1);
						}

						// Overwrite if new data
						if (pd->ch != ui->controllerWidgets[SELECTION_CH]->getValue())
						{
							pd->ch = ui->controllerWidgets[SELECTION_CH]->getValue();
							ui->drawPad (row, step);
							ui->send_pad (row, step);
						}
					}
				}
			}

			// Right button: copy pad to properties
			else if ((pointerEvent->getButton() == BEvents::RIGHT_BUTTON) &&
					 ((event->getEventType () == BEvents::BUTTON_PRESS_EVENT) ||
					  (event->getEventType () == BEvents::POINTER_MOTION_WHILE_BUTTON_PRESSED_EVENT)))
			{
				ui->controllerWidgets[SELECTION_CH]->setValue (pd->ch);
				ui->controllerWidgets[SELECTION_OCTAVE]->setValue(pd->pitchOctave);
				ui->controllerWidgets[SELECTION_VELOCITY]->setValue(pd->velocity);
				ui->controllerWidgets[SELECTION_DURATION]->setValue(pd->duration);
			}
		}

		// On BUTTON_RELEASE_EVENT and temporary delete mode: switch back
		if ((ui->tempTool) && (event->getEventType () == BEvents::BUTTON_RELEASE_EVENT) &&(pointerEvent->getButton() == BEvents::LEFT_BUTTON))
		{
			ui->tempTool = false;
			ui->controllerWidgets[SELECTION_CH]->setValue(ui->tempToolCh);
		}
	}
}

void BSEQuencer_GUI::drawCaption ()
{
	cairo_surface_t* surface = captionSurface.getDrawingSurface();
	cairo_surface_clear (surface);
	const double width = captionSurface.getEffectiveWidth ();
	const double height = captionSurface.getEffectiveHeight ();
	cairo_t* cr = cairo_create (surface);
	BColors::Color textcolor = *txColors. getColor(BColors::ACTIVE);
	cairo_set_source_rgba (cr, CAIRO_RGBA (textcolor));

	BScale scale (controllers[ROOT] + controllers[SIGNATURE], scaleNotes[controllers[SCALE] - 1]);
	int size = scale.getSize ();
	char label[8] = "";

	for (int i = 0; i < ROWS; ++i)
	{
		BColors::Color color = BColors::invisible;
		if (i == ROWS - 1) color = ctrlBgColor;
		else if (noteBits & (1 << i)) {color = ink; color.applyBrightness (0.75);}
		drawButton (surface, 0, (ROWS - i - 1) * height / ROWS + 1, width, height / ROWS - 2, {color, ""});

		if (i < ROWS - 1)
		{
			int note = scale.getMIDInote (i);
			if (note != ENOTE)
			{
				scale.getSymbol (label, i);
				if (note >= 12)
				{
					strcat (label, " +");
					char oct[8];
					sprintf (oct,"%i", (int) (note / 12));
					strcat (label, oct);
				}
			}
			else strcpy(label, "ERR");
		}
		else strcpy (label, "Control");

		cairo_text_extents_t ext;
		cairo_text_extents (cr, label, &ext);

		cairo_move_to (cr, width / 2 - ext.width / 2, (15.5 - i) * height / 16 + ext.height / 2);
		cairo_show_text (cr, label);
	}

	cairo_destroy (cr);
	captionSurface.update ();
}

void BSEQuencer_GUI::drawPad ()
{
	cairo_surface_t* surface = padSurface.getDrawingSurface();
	cairo_t* cr = cairo_create (surface);
	for (int i = 0; i < ROWS; ++i)
	{
		for (int j = 0; j < STEPS; ++j) drawPad (cr, i, j);
	}
	cairo_destroy (cr);
	padSurface.update();
}

void BSEQuencer_GUI::drawPad (int row, int step)
{
	cairo_surface_t* surface = padSurface.getDrawingSurface();
	cairo_t* cr = cairo_create (surface);
	drawPad (cr, row, step);
	cairo_destroy (cr);
	padSurface.update();
}

void BSEQuencer_GUI::drawPad (cairo_t* cr, int row, int step)
{
	if ((!cr) || (cairo_status (cr) != CAIRO_STATUS_SUCCESS) || (row < 0) || (row >= ROWS) || (step < 0) || (step >= STEPS)) return;

	// Get size of drawing area
	const double width = padSurface.getEffectiveWidth ();
	const double height = padSurface.getEffectiveHeight ();
	const double w = width / STEPS;
	const double h = height / ROWS;
	const double x = step * w;
	const double y = (ROWS - row - 1) * h;


	// Draw background
	BColors::Color bg =	(row == ROWS - 1 ? ctrlBgColor :
										   (((int)(step / controllerWidgets[NR_STEPS]->getValue ())) % 2) ? oddPadBgColor : evenPadBgColor);
	cairo_set_source_rgba (cr, CAIRO_RGBA (bg));
	cairo_rectangle (cr, x, y, w, h);
	cairo_fill (cr);

	// Draw pad
	int ch = pads[row][step].ch;
	double vel = (pads[row][step].velocity <= 1 ?  pads[row][step].velocity - 1 : (pads[row][step].velocity - 1) * 0.5);

	if ((ch >= 0) && (ch <= NR_SEQUENCER_CHS + NR_CTRL_BUTTONS))
	{
		ButtonStyle padstyle;

		if (ch <= NR_SEQUENCER_CHS)
		{
			padstyle = chButtonStyles[ch];
			if (ch > 0) padstyle.color.applyBrightness(vel);
		}
		else padstyle = ctrlButtonStyles [ch - NR_SEQUENCER_CHS - 1];

		if (cursorBits & (1 << step))
		{
			padstyle.color.setAlpha (1.0);
			padstyle.color.applyBrightness (0.75);
		}

		drawButton (cr, x + 1, y + 1, w - 2, h - 2, padstyle);
	}
}

LV2UI_Handle instantiate (const LV2UI_Descriptor *descriptor,
						  const char *plugin_uri,
						  const char *bundle_path,
						  LV2UI_Write_Function write_function,
						  LV2UI_Controller controller,
						  LV2UI_Widget *widget,
						  const LV2_Feature *const *features)
{
	PuglNativeWindow parentWindow = 0;
	LV2UI_Resize* resize = NULL;

	if (strcmp(plugin_uri, BSEQUENCER_URI) != 0)
	{
		std::cerr << "BSEQuencer.lv2#GUI: GUI does not support plugin with URI " << plugin_uri << std::endl;
		return NULL;
	}

	for (int i = 0; features[i]; ++i)
	{
		if (!strcmp(features[i]->URI, LV2_UI__parent)) parentWindow = (PuglNativeWindow) features[i]->data;
		else if (!strcmp(features[i]->URI, LV2_UI__resize)) resize = (LV2UI_Resize*)features[i]->data;
	}
	if (parentWindow == 0) std::cerr << "BSEQuencer.lv2#GUI: No parent window.\n";

	// New instance
	BSEQuencer_GUI* ui;
	try {ui = new BSEQuencer_GUI (bundle_path, features, parentWindow);}
	catch (std::exception& exc)
	{
		std::cerr << "BSEQuencer.lv2#GUI: Instantiation failed. " << exc.what () << std::endl;
		return NULL;
	}

	ui->controller = controller;
	ui->write_function = write_function;
	if (resize) resize->ui_resize(resize->handle, 1040, 840 );

	*widget = (LV2UI_Widget) puglGetNativeWindow (ui->getPuglView ());
	ui->send_ui_on();

	return (LV2UI_Handle) ui;
}

void cleanup(LV2UI_Handle ui)
{
	BSEQuencer_GUI* self = (BSEQuencer_GUI*) ui;
	delete self;
}

void port_event(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size,
	uint32_t format, const void* buffer)
{
	BSEQuencer_GUI* self = (BSEQuencer_GUI*) ui;
	self->port_event(port_index, buffer_size, format, buffer);
}

static int call_idle (LV2UI_Handle ui)
{
	BSEQuencer_GUI* self = (BSEQuencer_GUI*) ui;
	self->handleEvents ();
	return 0;
}

static const LV2UI_Idle_Interface idle = { call_idle };

static const void* extension_data(const char* uri)
{
	if (!strcmp(uri, LV2_UI__idleInterface)) return &idle;
	else return NULL;
}

const LV2UI_Descriptor guiDescriptor = {
		BSEQUENCER_GUI_URI,
		instantiate,
		cleanup,
		port_event,
		extension_data
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor(uint32_t index)
{
	switch (index) {
	case 0: return &guiDescriptor;
	default:return NULL;
    }
}

/* End of LV2 specific declarations
 *
 * *****************************************************************************
 *
 *
 */


