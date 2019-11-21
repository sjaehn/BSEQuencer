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

#include "BSEQuencer_GUI.hpp"

BSEQuencer_GUI::BSEQuencer_GUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeWindow parentWindow) :
	Window (1200, 820, "B.SEQuencer", parentWindow, true),
	controller (NULL), write_function (NULL),
	pluginPath (bundle_path ? std::string (bundle_path) : std::string ("")),
	sz (1.0), bgImageSurface (nullptr),
	uris (), forge (), clipBoard (),
	noteBits (0), chBits (0), tempTool (false), tempToolCh (0), wheelScrolled (false),
	mContainer (0, 0, 1200, 820, "main"),
	padSurface (98, 88, 804, 484, "box"),
	padSurfaceFocusText (0, 0, 100, 60, "txtbox", ""),
	captionSurface (18, 88, 64, 484, "box"),

	modeBox (920, 88, 260, 170, "box"),
	modeBoxLabel (10, 10, 240, 20, "ctlabel", "Play mode"),
	modeLabel (10, 80, 60, 20, "lflabel", "Mode"),
	modeListBox (80, 80, 170, 20, 170, 60, "menu", BItems::ItemList ({"Autoplay", "Host controlled"}), 2.0),
	modeAutoplayBpmLabel (10, 115, 120, 20, "lflabel", "Beats per min"),
	modeAutoplayBpmSlider (120, 105, 130, 25, "slider", 120.0, 1.0, 300.0, 0.0, "%3.1f"),
	modeAutoplayBpbLabel (10, 145, 120, 20, "lflabel", "Beats per bar"),
	modeAutoplayBpbSlider (120, 135, 130, 25, "slider", 4.0, 1.0, 16.0, 1.0, "%2.0f"),
	modeMidiInChannelLabel (10, 110, 150, 20 , "lflabel", "MIDI input channel"),
	modeMidiInChannelListBox (180, 110, 70, 20, 70, 200, "menu",
				  BItems::ItemList ({{0, "All"}, {1, "1"}, {2, "2"}, {3, "3"}, {4, "4"}, {5, "5"}, {6, "6"}, {7, "7"}, {8, "8"}, {9, "9"},
						     {10, "10"}, {11, "11"}, {12, "12"}, {13, "13"}, {14, "14"}, {15, "15"}, {16, "16"}})),
	modePlayLabel (10, 50, 205, 20, "lflabel", "Status: playing ..."),
	modePlayButton (220, 40, 30, 30, "box", 1.0),

	toolBox (920, 280, 260, 292, "box"),
	toolBoxLabel (10, 10, 240, 20, "ctlabel", "Toolbox"),
	toolButtonBox (0, 30, 260, 160, "widget"),
	toolWholeStepButton (170, 40, 80, 20, "tgbutton", "Whole step", 0.0),
	toolResetButton (80, 130, 20, 20, "tgbutton"),
	toolUndoButton (110, 130, 20, 20, "tgbutton"),
	toolRedoButton (140, 130, 20, 20, "tgbutton"),
	toolButtonBoxCtrlLabel (10, 10, 60, 20, "lflabel", "Controls"),
	toolButtonBoxChLabel (10, 70, 60, 20, "lflabel", "Channels"),
	toolButtonBoxEditLabel (10, 100, 60, 20, "lflabel", "Edit"),
	toolOctaveLabel (30, 260, 60, 20, "ctlabel", "Octave"),
	toolOctaveDial (35, 200, 50, 60, "dial", 0.0, -8.0, 8.0, 1.0, "%1.0f"),
	toolVelocityLabel (100, 260, 60, 20, "ctlabel", "Velocity"),
	toolVelocityDial  (105, 200, 50, 60, "dial", 1.0, 0.0, 2.0, 0.0, "%1.2f"),
	toolDurationLabel (170, 260, 60, 20, "ctlabel", "Duration"),
	toolDurationDial (175, 200, 50, 60, "dial", 1.0, 0.0, 1.0, 0.0, "%1.2f"),

	propertiesBox (920, 590, 260, 210, "box"),
	propertiesBoxLabel (10, 10, 240, 20, "ctlabel", "Properties"),
	propertiesNrStepsLabel (10, 50, 170, 20, "lflabel", "Total number of steps"),
	propertiesNrStepsListBox (180, 50, 70, 20, 70, 100, "menu",
				  BItems::ItemList ({{8, "8"}, {16, "16"}, {24, "24"}, {32, "32"}}), 16.0),
	propertiesStepsPerLabel (100, 85, 80, 20, "lflabel", "steps per"),
	propertiesStepsPerSlider (10, 75, 80, 25, "slider", 4.0, 1.0, 8.0, 1.0, "%2.0f"),
	propertiesBaseListBox (180, 85, 70, 20, 70, 60, "menu",
			       BItems::ItemList ({"beat", "bar"}), 1.0),
	propertiesRootLabel (10, 115, 40, 20, "lflabel", "Root"),
	propertiesRootListBox (100, 115, 70, 20, 0, -160, 70, 160, "menu",
			       BItems::ItemList ({{0, "C"}, {2, "D"}, {4, "E"}, {5, "F"}, {7, "G"}, {9, "A"}, {11, "B"}}), 0.0),
	propertiesSignatureListBox (180, 115, 70, 20, 70, 80, "menu",
				    BItems::ItemList ({{-1, "b"}, {0, ""}, {1, "#"}}), 0.0),
	propertiesOctaveLabel (10, 145, 55, 20, "lflabel", "Octave"),
	propertiesOctaveListBox (180, 145, 70, 20, 0, -220, 70, 220, "menu",
				 BItems::ItemList ({{-1, "-1"}, {0, "0"}, {1, "1"}, {2, "2"}, {3, "3"}, {4, "4"}, {5, "5"}, {6, "6"}, {7, "7"}, {8, "8"}}), 4.0),
	propertiesScaleLabel (10, 175, 50, 20, "lflabel", "Scale"),
	propertiesScaleEditIcon (70, 175, 20, 20, "widget", (bundle_path ? std::string (bundle_path) + EDIT_SYMBOL : std::string (""))),
	propertiesScaleListBox (),

	helpLabel (1140, 40, 30, 30, "ilabel", "?"),
	scaleEditor (nullptr)
	//(420, 20, 360, 760, "scaleeditor", (bundle_path ? std::string (bundle_path) : std::string ("")), 0, RTScaleMap (), BScale (0,defaultScale))

{
	// Init scale maps
	for (int scaleNr = 0; scaleNr < NR_SYSTEM_SCALES + NR_USER_SCALES; ++scaleNr)
	{
		scaleMaps[scaleNr] = defaultScaleMaps[scaleNr];
	}

	// Init propertiesScaleListBox
	BItems::ItemList scaleItems;
	for (int scaleNr = 0; scaleNr < NR_SYSTEM_SCALES + NR_USER_SCALES; ++scaleNr)
	{
		scaleItems.push_back (BItems::Item (scaleNr, scaleMaps[scaleNr].name));
	}
	propertiesScaleListBox = BWidgets::PopupListBox (100, 175, 150, 20, 0, -400, 150, 400, "menu", scaleItems, 0.0);
	propertiesScaleListBox.rename ("menu");

	// Init toolbox buttons
	toolButtonBox.addButton (80, 70, 20, 20, {{0.0, 0.03, 0.06, 1.0}, NO_CTRL, "No channel"});
	for (int i = 1; i < NR_SEQUENCER_CHS + 1; ++i) toolButtonBox.addButton (80 + i * 30, 70, 20, 20, chButtonStyles[i]);
	toolButtonBox.addButton (80 , 10, 20, 20, {{0.0, 0.03, 0.06, 1.0}, NO_CTRL, "No control"});
	for (int i = 1; i < NR_CTRL_BUTTONS; ++i) toolButtonBox.addButton (80 + (i % 6) * 30, 10 + ((int) (i / 6)) * 30, 20, 20, ctrlButtonStyles[i]);
	for (int i = 0; i < NR_EDIT_BUTTONS; ++i) toolButtonBox.addButton (80 + i * 30, 100, 20, 20, editButtonStyles[i]);

	// Init ChBoxes
	for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
	{
		chBoxes[i].box = BWidgets::Widget (98 + i * 203.5, 590, 193.5, 210, "box");
		chBoxes[i].box.rename ("box");
		chBoxes[i].chSymbol = BWidgets::DrawingSurface (7, 7, 26, 26, "button");
		chBoxes[i].chSymbol.rename ("button");
		chBoxes[i].chLabel = BWidgets::Label (40, 10, 133.5, 20, "ctlabel", "Channel " + std::to_string (i + 1));
		chBoxes[i].chLabel.rename ("ctlabel");
		chBoxes[i].channelLabel = BWidgets::Label (10, 50, 100, 20, "lflabel", "MIDI channel");
		chBoxes[i].channelLabel.rename ("lflabel");
		chBoxes[i].channelListBox = BWidgets::PopupListBox (123.5, 50, 60, 20, 60, 120, "menu",
								    BItems::ItemList({"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"}),
								    i + 1);
		chBoxes[i].channelListBox.rename ("menu");
		chBoxes[i].pitchLabel = BWidgets::Label (10, 80, 80, 20, "lflabel", "Input pitch");
		chBoxes[i].pitchLabel.rename ("lflabel");
		chBoxes[i].pitchSwitch = BWidgets::HSwitch (132.5, 82, 42, 16, "slider", 0.0);
		chBoxes[i].pitchSwitch.rename ("ch" + std::to_string (i + 1));
		chBoxes[i].pitchScreen = BWidgets::Widget (10, 80, 173.5, 20, "screen");
		chBoxes[i].pitchScreen.rename ("screen");
		chBoxes[i].pitchScreen.hide ();
		chBoxes[i].velocityDial = BWidgets::DialValue (25, 120, 50, 60, "dial", 1.0, 0.0, 2.0, 0.0, "%1.2f");
		chBoxes[i].velocityDial.rename ("ch" + std::to_string (i + 1));
		chBoxes[i].velocityLabel = BWidgets::Label (20, 180, 60, 20, "ctlabel", "Velocity");
		chBoxes[i].velocityLabel.rename ("ctlabel");
		chBoxes[i].noteOffsetDial = BWidgets::DialValue (118.5, 120, 50, 60, "dial", 0.0, -127.0, 127.0, 1.0, "%3.0f");
		chBoxes[i].noteOffsetDial.rename ("ch" + std::to_string (i + 1));
		chBoxes[i].noteOffsetLabel = BWidgets::Label (113.5, 180, 60, 20, "ctlabel", "Offset");
		chBoxes[i].noteOffsetLabel.rename ("ctlabel");
	}

	// Link controllerWidgets
	controllerWidgets[MIDI_IN_CHANNEL] = (BWidgets::ValueWidget*) &modeMidiInChannelListBox;
	controllerWidgets[PLAY] = (BWidgets::ValueWidget*) &modePlayButton;
	controllerWidgets[MODE] = (BWidgets::ValueWidget*) &modeListBox;
	controllerWidgets[NR_OF_STEPS] = (BWidgets::ValueWidget*) &propertiesNrStepsListBox;
	controllerWidgets[STEPS_PER] = (BWidgets::ValueWidget*) &propertiesStepsPerSlider;
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
		controllerWidgets[CH + i * CH_SIZE + MIDI_CHANNEL] = (BWidgets::ValueWidget*) &chBoxes[i].channelListBox;
		controllerWidgets[CH + i * CH_SIZE + NOTE_OFFSET] = (BWidgets::ValueWidget*) &chBoxes[i].noteOffsetDial;
	}

	// Init controller values
	for (int i = 0; i < KNOBS_SIZE; ++i) controllers[i] = controllerWidgets[i]->getValue ();

	// Set callback functions
	for (int i = 0; i < KNOBS_SIZE; ++i) controllerWidgets[i]->setCallbackFunction (BEvents::VALUE_CHANGED_EVENT, valueChangedCallback);

	padSurface.setDraggable (true);
	padSurface.setCallbackFunction (BEvents::BUTTON_PRESS_EVENT, padsPressedCallback);
	padSurface.setCallbackFunction (BEvents::BUTTON_RELEASE_EVENT, padsPressedCallback);
	padSurface.setCallbackFunction (BEvents::POINTER_DRAG_EVENT, padsPressedCallback);

	padSurface.setScrollable (true);
	padSurface.setCallbackFunction (BEvents::WHEEL_SCROLL_EVENT, padsScrolledCallback);

	padSurface.setFocusable (true);
	BWidgets::FocusWidget* focus = new BWidgets::FocusWidget (this, "screen");
	if (!focus) throw std::bad_alloc ();
	padSurface.setFocusWidget (focus);
	focus->add (padSurfaceFocusText);
	focus->resize ();
	focus->applyTheme (theme);
	padSurface.setCallbackFunction (BEvents::FOCUS_IN_EVENT, padsFocusedCallback);
	padSurface.setCallbackFunction (BEvents::FOCUS_OUT_EVENT, padsFocusedCallback);
	padSurface.setMergeable (BEvents::POINTER_DRAG_EVENT, false);

	toolResetButton.setCallbackFunction(BEvents::VALUE_CHANGED_EVENT, resetClickedCallback);
	toolUndoButton.setCallbackFunction(BEvents::VALUE_CHANGED_EVENT, undoClickedCallback);
	toolRedoButton.setCallbackFunction(BEvents::VALUE_CHANGED_EVENT, undoClickedCallback);

	helpLabel.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, helpPressedCallback);
	propertiesScaleEditIcon.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, editPressedCallback);

	// Apply theme
	bgImageSurface = cairo_image_surface_create_from_png ((pluginPath + BG_FILE).c_str());
	widgetBg.loadFillFromCairoSurface (bgImageSurface);
	applyTheme (theme);

	toolOctaveDial.setHardChangeable (false);
	toolVelocityDial.setHardChangeable (false);
	toolDurationDial.setHardChangeable (false);

	modeAutoplayBpmLabel.hide ();
	modeAutoplayBpmSlider.hide ();
	modeAutoplayBpbLabel.hide ();
	modeAutoplayBpbSlider.hide ();

	modeBoxLabel.setState (BColors::ACTIVE);
	toolBoxLabel.setState (BColors::ACTIVE);
	propertiesBoxLabel.setState (BColors::ACTIVE);
	propertiesScaleEditIcon.hide ();
	for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
	{
		chBoxes[i].chLabel.setState (BColors::ACTIVE);
		drawButton (chBoxes[i].chSymbol.getDrawingSurface(), 0, 0, 20, 20, chButtonStyles[i + 1].color, chButtonStyles[i + 1].symbol);
		chBoxes[i].noteOffsetDial.setHardChangeable (false);
		chBoxes[i].velocityDial.setHardChangeable (false);
	}


	// Pack widgets

	modeBox.add (modeBoxLabel);
	modeBox.add (modeLabel);
	modeBox.add (modeListBox);
	modeBox.add (modeAutoplayBpmLabel);
	modeBox.add (modeAutoplayBpmSlider);
	modeBox.add (modeAutoplayBpbLabel);
	modeBox.add (modeAutoplayBpbSlider);
	modeBox.add (modeMidiInChannelLabel);
	modeBox.add (modeMidiInChannelListBox);
	modeBox.add (modePlayLabel);
	modeBox.add (modePlayButton);

	toolBox.add (toolBoxLabel);
	toolBox.add (toolButtonBox);

	toolButtonBox.add (toolButtonBoxCtrlLabel);
	toolButtonBox.add (toolButtonBoxChLabel);
	toolButtonBox.add (toolButtonBoxEditLabel);
	toolButtonBox.add (toolWholeStepButton);
	toolButtonBox.add (toolResetButton);
	toolButtonBox.add (toolUndoButton);
	toolButtonBox.add (toolRedoButton);

	toolBox.add (toolOctaveLabel);
	toolBox.add (toolOctaveDial);
	toolBox.add (toolVelocityLabel);
	toolBox.add (toolVelocityDial);
	toolBox.add (toolDurationLabel);
	toolBox.add (toolDurationDial);

	propertiesBox.add (propertiesBoxLabel);
	propertiesBox.add (propertiesNrStepsLabel);
	propertiesBox.add (propertiesNrStepsListBox);
	propertiesBox.add (propertiesStepsPerLabel);
	propertiesBox.add (propertiesStepsPerSlider);
	propertiesBox.add (propertiesBaseListBox);
	propertiesBox.add (propertiesRootLabel);
	propertiesBox.add (propertiesRootListBox);
	propertiesBox.add (propertiesSignatureListBox);
	propertiesBox.add (propertiesOctaveLabel);
	propertiesBox.add (propertiesOctaveListBox);
	propertiesBox.add (propertiesScaleLabel);
	propertiesBox.add (propertiesScaleEditIcon);
	propertiesBox.add (propertiesScaleListBox);

	for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
	{
		chBoxes[i].box.add (chBoxes[i].chSymbol);
		chBoxes[i].box.add (chBoxes[i].chLabel);
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

	drawCaption ();
	drawPad();
	add (mContainer);

	pattern.clear ();

	//Scan host features for URID map
	LV2_URID_Map* map = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp(features[i]->URI, LV2_URID__map) == 0)
		{
			map = (LV2_URID_Map*) features[i]->data;
		}
	}
	if (!map) throw std::invalid_argument ("Host does not support urid:map");

	//Map URIS
	getURIs (map, &uris);

	// Initialize forge
	lv2_atom_forge_init (&forge, map);
}

BSEQuencer_GUI::~BSEQuencer_GUI ()
{
	if (scaleEditor) delete scaleEditor;

	BWidgets::FocusWidget* focus = padSurface.getFocusWidget();
	if (focus) delete focus;

	send_ui_off ();
}

void BSEQuencer_GUI::Pattern::clear ()
{
	Pad pad0 = Pad ();

	changes.oldMessage.clear ();
	changes.newMessage.clear ();
	journal.clear ();

	for (int r = 0; r < ROWS; ++r)
	{
		for (int s = 0; s < MAXSTEPS; ++s)
		{
			setPad (r, s, pad0);
		}
	}

	store ();
}

Pad BSEQuencer_GUI::Pattern::getPad (const size_t row, const size_t step) const
{
	return pads[LIMIT (row, 0, ROWS)][LIMIT (step, 0, MAXSTEPS)];
}
void BSEQuencer_GUI::Pattern::setPad (const size_t row, const size_t step, const Pad& pad)
{
	size_t r = LIMIT (row, 0, ROWS);
	size_t s = LIMIT (step, 0, MAXSTEPS);
	changes.oldMessage.push_back (PadMessage (s, r, pads[r][s]));
	changes.newMessage.push_back (PadMessage (s, r, pad));
	pads[r][s] = pad;
}

std::vector<PadMessage> BSEQuencer_GUI::Pattern::undo ()
{
	if (!changes.newMessage.empty ()) store ();

	std::vector<PadMessage> padMessages = journal.undo ();
	std::reverse (padMessages.begin (), padMessages.end ());
	for (PadMessage const& p : padMessages)
	{
		size_t r = LIMIT (p.row, 0, ROWS);
		size_t s = LIMIT (p.step, 0, MAXSTEPS);
		pads[r][s] = Pad (p);
	}

	return padMessages;
}

std::vector<PadMessage> BSEQuencer_GUI::Pattern::redo ()
{
	if (!changes.newMessage.empty ()) store ();

	std::vector<PadMessage> padMessages = journal.redo ();
	for (PadMessage const& p : padMessages)
	{
		size_t r = LIMIT (p.row, 0, ROWS);
		size_t s = LIMIT (p.step, 0, MAXSTEPS);
		pads[r][s] = Pad (p);
	}

	return padMessages;
}

void BSEQuencer_GUI::Pattern::store ()
{
	if (changes.newMessage.empty ()) return;

	journal.push (changes.oldMessage, changes.newMessage);
	changes.oldMessage.clear ();
	changes.newMessage.clear ();
}

void BSEQuencer_GUI::port_event(uint32_t port, uint32_t buffer_size,
	uint32_t format, const void* buffer)
{
	// Notify port
	if ((format == uris.atom_eventTransfer) && (port == OUTPUT))
	{
		const LV2_Atom* atom = (const LV2_Atom*) buffer;
		if ((atom->type == uris.atom_Blank) || (atom->type == uris.atom_Object))
		{
			const LV2_Atom_Object* obj = (const LV2_Atom_Object*) atom;

			// Pad notification
			if (obj->body.otype == uris.notify_padEvent)
			{
				LV2_Atom *oPad = NULL;
				lv2_atom_object_get(obj, uris.notify_pad, &oPad, NULL);

				if (oPad && (oPad->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oPad;
					if (vec->body.child_type == uris.atom_Float)
					{
						if (wheelScrolled)
						{
							pattern.store ();
							wheelScrolled = false;
						}

						uint32_t size = (uint32_t) ((oPad->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (PadMessage));
						PadMessage* pMes = (PadMessage*)(&vec->body + 1);
						for (unsigned int i = 0; i < size; ++i)

						{
							int step = (int) pMes[i].step;
							int row = (int) pMes[i].row;
							if ((step >= 0) && (step < MAXSTEPS) && (row >= 0) && (row < ROWS))
							{
								pattern.setPad (row, step, Pad (pMes[i]));
							}
						}
						pattern.store ();
						drawPad ();
					}
				}
			}

			// Status notifications
			else if (obj->body.otype == uris.notify_statusEvent)
			{
				LV2_Atom *oCursors = NULL, *oNotes = NULL, *oChs = NULL;
				lv2_atom_object_get
				(
					obj, uris.notify_cursors, &oCursors,
					uris.notify_notes, &oNotes,
					uris.notify_channels, &oChs,
					NULL
				);

				// Cursor notifications
				if (oCursors && (oCursors->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oCursors;
					if (vec->body.child_type == uris.atom_Int)
					{
						int* cb = (int*)(&vec->body + 1);
						if (memcmp (cursorBits, cb, MAXSTEPS * sizeof(int)))
						{
							memcpy (&cursorBits, cb, MAXSTEPS * sizeof(int));
							drawPad ();
						}
					}
				}

				// Note notifications
				if (oNotes && (oNotes->type == uris.atom_Int) && (noteBits != ((uint32_t) ((LV2_Atom_Int*)oNotes)->body)))
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

			// GUI user scales changed notifications
			else if (obj->body.otype == uris.notify_scaleMapsEvent)
			{
				int iD = 0;
				int scaleNr = 0;

				LV2_Atom *oId = NULL, *oName = NULL, *oElements = NULL, *oAltSymbols = NULL, *oScale = NULL;
				lv2_atom_object_get
				(
					obj, uris.notify_scaleID,  &oId,
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
						if (iD == scaleMaps[i].iD)
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
						scaleMaps[scaleNr].name = std::string ((char*) LV2_ATOM_BODY(oName));
						std::string s = (char*) LV2_ATOM_BODY(oName);
						if (propertiesScaleListBox.getItemList())
						{
							BItems::ItemList::iterator it = std::next (propertiesScaleListBox.getItemList()->begin (), scaleNr);
							BWidgets::Label* l = (BWidgets::Label*) (*it).getWidget ();
							if (l) l->setText (s);
							propertiesScaleListBox.update();
						}
					}

					// Elements TODO exceptions
					if (oElements && (oElements->type == uris.atom_Vector))
					{
						const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oElements;
						if (vec->body.child_type == uris.atom_Int)
						{
							memcpy (scaleMaps[scaleNr].elements.data(), &vec->body + 1, 16 * sizeof (int));
						}
					}

					// Alt Symbols TODO exceptions
					if (oAltSymbols && (oAltSymbols->type == uris.atom_Vector))
					{
						const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oAltSymbols;
						if (vec->body.child_type == uris.atom_String)
						{
							char rtAltSymbols[16][16];
							memcpy (rtAltSymbols, (&vec->body + 1), 16 * 16);

							for (size_t r = 0; r < ROWS; ++r)
							{
								scaleMaps[scaleNr].altSymbols[r] = std::string (rtAltSymbols[r]);
							}
						}
					}

					// Scale TODO exceptions
					if (oScale && (oScale->type == uris.atom_Vector))
					{
						const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oScale;
						if (vec->body.child_type == uris.atom_Int)
						{
							BScaleNotes* notes = (BScaleNotes*) (&vec->body + 1);
							scaleMaps[scaleNr].scaleNotes = *notes;
						}
					}

					if (scaleNr == controllers[SCALE]) drawCaption ();

				}
			}
		}
	}

	// Scan remaining ports
	else if ((format == 0) && (port >= KNOBS))
	{
		float* pval = (float*) buffer;
		controllerWidgets[port-KNOBS]->setValue (*pval);
	}

}

void BSEQuencer_GUI::scale ()
{
	hide ();
	//Scale fonts
	ctLabelFont.setFontSize (12 * sz);
	tgLabelFont.setFontSize (12 * sz);
	iLabelFont.setFontSize (18 * sz);
	lfLabelFont.setFontSize (12 * sz);

	//Background
	cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1200 * sz, 820 * sz);
	cairo_t* cr = cairo_create (surface);
	cairo_scale (cr, sz, sz);
	cairo_set_source_surface(cr, bgImageSurface, 0, 0);
	cairo_paint(cr);
	widgetBg.loadFillFromCairoSurface(surface);
	cairo_destroy (cr);
	cairo_surface_destroy (surface);

	//Scale widgets
	RESIZE (mContainer, 0, 0, 1200, 820, sz);
	RESIZE (padSurface, 98, 88, 804, 484, sz);

	//RESIZE (padSurfaceFocusText, 0, 0, 100, 60, sz);
	scaleFocus ();
	RESIZE (captionSurface, 18, 88, 64, 484, sz);

	RESIZE (modeBox, 920, 88, 260, 180, sz);
	RESIZE (modeBoxLabel, 10, 10, 240, 20, sz);
	RESIZE (modeLabel, 10, 80, 60, 20, sz);
	RESIZE (modeListBox, 80, 80, 170, 20, sz);
	modeListBox.resizeListBox(170 * sz, 60 * sz);
	modeListBox.resizeListBoxItems(170 * sz, 20 * sz);
	RESIZE (modeAutoplayBpmLabel, 10, 115, 120, 20, sz);
	RESIZE (modeAutoplayBpmSlider, 120, 105, 130, 25, sz);
	RESIZE (modeAutoplayBpbLabel, 10, 145, 120, 20, sz);
	RESIZE (modeAutoplayBpbSlider, 120, 135, 130, 25, sz);
	RESIZE (modeMidiInChannelLabel, 10, 110, 150, 20, sz);
	RESIZE (modeMidiInChannelListBox, 180, 110, 70, 20, sz);
	modeMidiInChannelListBox.resizeListBox (70 * sz, 200 * sz);
	modeMidiInChannelListBox.resizeListBoxItems (70 * sz, 20 * sz);
	RESIZE (modePlayLabel, 10, 50, 205, 20, sz);
	RESIZE (modePlayButton, 220, 40, 30, 30, sz);

	RESIZE (toolBox, 920, 280, 260, 292, sz);
	RESIZE (toolBoxLabel, 10, 10, 240, 20, sz);
	RESIZE (toolButtonBox, 0, 30, 260, 160, sz);
	RESIZE (toolWholeStepButton, 170, 40, 80, 20, sz);
	RESIZE (toolResetButton, 80, 130, 20, 20, sz);
	RESIZE (toolUndoButton,110, 130, 20, 20, sz);
	RESIZE (toolRedoButton, 140, 130, 20, 20, sz);
	RESIZE (toolButtonBoxCtrlLabel, 10, 10, 60, 20, sz);
	RESIZE (toolButtonBoxChLabel, 10, 70, 60, 20, sz);
	RESIZE (toolButtonBoxEditLabel, 10, 100, 60, 20, sz);
	RESIZE (toolOctaveLabel, 30, 260, 60, 20, sz);
	RESIZE (toolOctaveDial, 35, 200, 50, 60, sz);
	RESIZE (toolVelocityLabel, 100, 260, 60, 20, sz);
	RESIZE (toolVelocityDial, 105, 200, 50, 60, sz);
	RESIZE (toolDurationLabel, 170, 260, 60, 20, sz);
	RESIZE (toolDurationDial, 175, 200, 50, 60, sz);

	RESIZE (propertiesBox, 920, 590, 260, 210, sz);
	RESIZE (propertiesBoxLabel, 10, 10, 240, 20, sz);
	RESIZE (propertiesNrStepsLabel, 10, 50, 170, 20, sz);
	RESIZE (propertiesNrStepsListBox, 180, 50, 70, 20, sz);
	propertiesNrStepsListBox.resizeListBox (70 * sz, 100 * sz);
	propertiesNrStepsListBox.resizeListBoxItems (70 * sz, 20 * sz);
	RESIZE (propertiesStepsPerSlider, 10, 75, 80, 25, sz);
	RESIZE (propertiesStepsPerLabel, 100, 85, 80, 20, sz);
	RESIZE (propertiesBaseListBox, 180, 85, 70, 20, sz);
	propertiesBaseListBox.resizeListBox (70 * sz, 60 * sz);
	propertiesBaseListBox.resizeListBoxItems (70 * sz, 20 * sz);
	RESIZE (propertiesRootLabel, 10, 115, 40, 20, sz);
	RESIZE (propertiesRootListBox, 100, 115, 70, 20, sz);
	propertiesRootListBox.resizeListBox (70 * sz, 160 * sz);
	propertiesRootListBox.moveListBox (0, -160 * sz);
	propertiesRootListBox.resizeListBoxItems (70 * sz, 20 * sz);
	RESIZE (propertiesSignatureListBox, 180, 115, 70, 20, sz);
	propertiesSignatureListBox.resizeListBox (70 * sz, 80 * sz);
	propertiesSignatureListBox.resizeListBoxItems (70 * sz, 20 * sz);
	RESIZE (propertiesOctaveLabel, 10, 145, 55, 20, sz);
	RESIZE (propertiesOctaveListBox, 180, 145, 70, 20, sz);
	propertiesOctaveListBox.resizeListBox (70 * sz, 220 * sz);
	propertiesOctaveListBox.moveListBox (0, -220 * sz);
	propertiesOctaveListBox.resizeListBoxItems (70 * sz, 20 * sz);
	RESIZE (propertiesScaleLabel, 10, 175, 50, 20, sz);
	RESIZE (propertiesScaleEditIcon, 70, 175, 20, 20, sz);
	RESIZE (propertiesScaleListBox, 100, 175, 150, 20, sz);
	propertiesScaleListBox.resizeListBox (150 * sz, 400 * sz);
	propertiesScaleListBox.moveListBox (0, -400 * sz);
	propertiesScaleListBox.resizeListBoxItems (150 * sz, 20 * sz);

	RESIZE (helpLabel, 1140, 40, 30, 30, sz);
	if (scaleEditor) {RESIZE ((*scaleEditor), 420, 20, 360, 760, sz);}

	for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
	{
		RESIZE (chBoxes[i].box, 98 + i * 203.5, 590, 193.5, 210, sz);
		RESIZE (chBoxes[i].chSymbol, 7, 7, 26, 26, sz);
		RESIZE (chBoxes[i].chLabel, 40, 10, 133.5, 20, sz);
		RESIZE (chBoxes[i].channelLabel, 10, 50, 100, 20, sz);
		RESIZE (chBoxes[i].channelListBox, 123.5, 50, 60, 20, sz);
		chBoxes[i].channelListBox.resizeListBox (60 * sz, 120 * sz);
		chBoxes[i].channelListBox.resizeListBoxItems (60 * sz, 20 * sz);
		RESIZE (chBoxes[i].pitchLabel, 10, 80, 80, 20, sz);
		RESIZE (chBoxes[i].pitchSwitch, 132.5, 82, 42, 16, sz);
		RESIZE (chBoxes[i].pitchScreen, 10, 80, 173.5, 20, sz);
		RESIZE (chBoxes[i].velocityDial, 25, 120, 50, 60, sz);
		RESIZE (chBoxes[i].velocityLabel, 20, 180, 60, 20, sz);
		RESIZE (chBoxes[i].noteOffsetDial, 118.5, 120, 50, 60, sz);
		RESIZE (chBoxes[i].noteOffsetLabel, 113.5, 180, 60, 20, sz);

		drawButton
		(
			chBoxes[i].chSymbol.getDrawingSurface(),
			0, 0,
			chBoxes[i].chSymbol.getEffectiveWidth(),
			chBoxes[i].chSymbol.getEffectiveHeight(),
			chButtonStyles[i + 1].color,
			chButtonStyles[i + 1].symbol
		);
	}

	applyTheme (theme);

	drawCaption ();
	drawPad ();
	show ();
}

void BSEQuencer_GUI::scaleFocus ()
{
	cairo_surface_t* surface = padSurface.getDrawingSurface();
	cairo_t* cr = cairo_create (surface);

	padSurfaceFocusText.resize (400,100);	// Maximize size first to omit breaks
	std::vector<std::string> textblock = padSurfaceFocusText.getTextBlock ();
	double blockheight = padSurfaceFocusText.getTextBlockHeight (textblock);
	double blockwidth = 0.0;
	for (std::string textline : textblock)
	{
		cairo_text_extents_t ext = padSurfaceFocusText.getFont ()->getTextExtents (cr, textline);
		if (ext.width > blockwidth) blockwidth = ext.width;
	}
	padSurfaceFocusText.resize (blockwidth + 2 * padSurfaceFocusText.getXOffset (), blockheight + 2 * padSurfaceFocusText.getYOffset ());

	padSurface.getFocusWidget()->resize();

	cairo_destroy (cr);
}

void BSEQuencer_GUI::applyTheme (BStyles::Theme& theme)
{
	mContainer.applyTheme (theme);

	padSurface.applyTheme (theme);
	padSurfaceFocusText.applyTheme (theme);
	captionSurface.applyTheme (theme);

	modeBox.applyTheme (theme);
	modeBoxLabel.applyTheme (theme);
	modeLabel.applyTheme (theme);
	modeListBox.applyTheme (theme);
	modeAutoplayBpmLabel.applyTheme (theme);
	modeAutoplayBpmSlider.applyTheme (theme);
	modeAutoplayBpbLabel.applyTheme (theme);
	modeAutoplayBpbSlider.applyTheme (theme);
	modeMidiInChannelLabel.applyTheme (theme);
	modeMidiInChannelListBox.applyTheme (theme);
	modePlayLabel.applyTheme (theme);
	modePlayButton.applyTheme (theme);

	toolBox.applyTheme (theme);
	toolBoxLabel.applyTheme (theme);
	toolButtonBox.applyTheme (theme);
	toolWholeStepButton.applyTheme (theme);
	toolResetButton.applyTheme (theme);
	toolUndoButton.applyTheme (theme);
	toolRedoButton.applyTheme (theme);
	toolButtonBoxCtrlLabel.applyTheme (theme);
	toolButtonBoxChLabel.applyTheme (theme);
	toolButtonBoxEditLabel.applyTheme (theme);
	toolOctaveLabel.applyTheme (theme);
	toolOctaveDial.applyTheme (theme);
	toolVelocityLabel.applyTheme (theme);
	toolVelocityDial.applyTheme (theme);
	toolDurationLabel.applyTheme (theme);
	toolDurationDial.applyTheme (theme);

	propertiesBox.applyTheme (theme);
	propertiesNrStepsLabel.applyTheme (theme);
	propertiesNrStepsListBox.applyTheme (theme);
	propertiesBoxLabel.applyTheme (theme);
	propertiesStepsPerLabel.applyTheme (theme);
	propertiesStepsPerSlider.applyTheme (theme);
	propertiesBaseListBox.applyTheme (theme);
	propertiesRootLabel.applyTheme (theme);
	propertiesRootListBox.applyTheme (theme);
	propertiesSignatureListBox.applyTheme (theme);
	propertiesOctaveLabel.applyTheme (theme);
	propertiesOctaveListBox.applyTheme (theme);
	propertiesScaleLabel.applyTheme (theme);
	propertiesScaleEditIcon.applyTheme (theme);
	propertiesScaleListBox.applyTheme (theme);

	for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
	{
		chBoxes[i].box.applyTheme (theme);
		chBoxes[i].chSymbol.applyTheme (theme);
		chBoxes[i].chLabel.applyTheme (theme);
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
	//scaleEditor.applyTheme (theme);
}

void BSEQuencer_GUI::onConfigureRequest (BEvents::ExposeEvent* event)
{
	Window::onConfigureRequest (event);

	sz = (width_ / 1200 > height_ / 820 ? height_ / 820 : width_ / 1200);
	scale ();
}

void BSEQuencer_GUI::onCloseRequest (BEvents::WidgetEvent* event)
{
	if (event && (scaleEditor) && (event->getRequestWidget () == scaleEditor))
	{
		int mapNr = scaleEditor->getMapNr();
		scaleMaps[mapNr] = scaleEditor->getScaleMap();

		// Update captions
		if (controllers[SCALE] == mapNr) drawCaption();

		// Update scale listbox
		BItems::ItemList::iterator it = std::next (propertiesScaleListBox.getItemList()->begin (), mapNr);
		BWidgets::Label* l = (BWidgets::Label*) (*it).getWidget ();
		if (l) l->setText (scaleMaps[mapNr].name);
		propertiesScaleListBox.update();

		// Notify plugin
		send_scaleMaps (mapNr);

		// And close / delete
		delete scaleEditor;
		scaleEditor = nullptr;
	}
	else Window::onCloseRequest (event);
}

void BSEQuencer_GUI::send_ui_on ()
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.ui_on);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, INPUT, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BSEQuencer_GUI::send_ui_off ()
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.ui_off);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, INPUT, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BSEQuencer_GUI::send_pad (int row, int step)
{
	PadMessage padmsg (step, row, pattern.getPad (row, step));

	uint8_t obj_buf[128];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.notify_padEvent);
	lv2_atom_forge_key(&forge, uris.notify_pad);
	lv2_atom_forge_vector(&forge, sizeof(float), uris.atom_Float, sizeof(PadMessage) / sizeof(float), (void*) &padmsg);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, INPUT, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BSEQuencer_GUI::send_scaleMaps (int scaleNr)
{
	RTScaleMap rtScaleMap; rtScaleMap = scaleMaps[scaleNr];
	uint8_t obj_buf[2048];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.notify_scaleMapsEvent);
	lv2_atom_forge_key(&forge, uris.notify_scaleID);
	lv2_atom_forge_int(&forge, rtScaleMap.iD);
	lv2_atom_forge_key(&forge, uris.notify_scaleName);
	lv2_atom_forge_string (&forge, rtScaleMap.name, 64);
	lv2_atom_forge_key(&forge, uris.notify_scaleElements);
	lv2_atom_forge_vector(&forge, sizeof (int), uris.atom_Int, 16, (void*) rtScaleMap.elements);
	lv2_atom_forge_key(&forge, uris.notify_scaleAltSymbols);
	lv2_atom_forge_vector(&forge, 16, uris.atom_String, 16, (void*) rtScaleMap.altSymbols);
	lv2_atom_forge_key(&forge, uris.notify_scale);
	BScaleNotes* notes = &rtScaleMap.scaleNotes;
	lv2_atom_forge_vector(&forge, sizeof (int), uris.atom_Int, 12, (void*) notes);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, INPUT, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
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
				if (widget == ui->controllerWidgets[i])
				{
					widgetNr = i;
					break;
				}
			}

			if (widgetNr >= 0)
			{
				//std::cerr << "BSEQuencer.lv2#GUI valueChangedCallback() for widget #" << widgetNr << ": " << value << "\n";
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
						ui->modeAutoplayBpmLabel.show ();
						ui->modeAutoplayBpmSlider.show ();
						ui->modeAutoplayBpbLabel.show ();
						ui->modeAutoplayBpbSlider.show ();
						ui->modeMidiInChannelLabel.hide ();
						ui->modeMidiInChannelListBox.hide ();
						for (int i = 0; i < NR_SEQUENCER_CHS; ++i) ui->chBoxes[i].pitchScreen.show ();
					}

					else
					{
						ui->modeAutoplayBpmLabel.hide ();
						ui->modeAutoplayBpmSlider.hide ();
						ui->modeAutoplayBpbLabel.hide ();
						ui->modeAutoplayBpbSlider.hide ();
						ui->modeMidiInChannelLabel.show ();
						ui->modeMidiInChannelListBox.show ();
						for (int i = 0; i < NR_SEQUENCER_CHS; ++i) ui->chBoxes[i].pitchScreen.hide ();
					}
				}

				// Scale changed
				if (widgetNr == SCALE)
				{
					if (value < NR_SYSTEM_SCALES) ui->propertiesScaleEditIcon.hide();
					else ui->propertiesScaleEditIcon.show();
				}

				// Pad relevant changes
				if ((widgetNr == NR_OF_STEPS) || (widgetNr == STEPS_PER) ||(widgetNr == ROOT) || (widgetNr == SIGNATURE) ||
					(widgetNr == SCALE)) ui->drawPad ();

				// Caption relevant changes
				if ((widgetNr == ROOT) || (widgetNr == SIGNATURE) || (widgetNr == SCALE)) ui->drawCaption ();
			}
		}
	}
}

void BSEQuencer_GUI::helpPressedCallback (BEvents::Event* event) {system(OPEN_CMD " " HELP_URL);}

void BSEQuencer_GUI::editPressedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()) && (event->getWidget()->getMainWindow()))
	{
		BSEQuencer_GUI* ui = (BSEQuencer_GUI*)(((BWidgets::Widget*)(event->getWidget()))->getMainWindow());
		int mapNr = ui->propertiesScaleListBox.getValue();

		if (!ui->scaleEditor)
		{
			ui->scaleEditor = new ScaleEditor
			(
				420 * ui->sz, 20 * ui->sz, 360 * ui->sz, 760 * ui->sz, "scaleeditor",
				ui->pluginPath,
				mapNr,
				ui->scaleMaps[mapNr],
				BScale
				(
					(int (ui->controllers[ROOT] + ui->controllers[SIGNATURE] + 12)) % 12,
					(SignatureIndex) ui->controllers[SIGNATURE],
					ui->scaleMaps[mapNr].scaleNotes
				)
			);
			ui->add (*ui->scaleEditor);
		}
	}
}

void BSEQuencer_GUI::resetClickedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()) && (event->getWidget()->getMainWindow()))
	{
		BWidgets::Button* widget = (BWidgets::Button*)event->getWidget();
		BSEQuencer_GUI* ui = (BSEQuencer_GUI*)(widget->getMainWindow());
		double value = ((BEvents::ValueChangedEvent*) event)->getValue ();

		if ((value == 1) && (widget == (BWidgets::Button*)&ui->toolResetButton))
		{
			if (ui->wheelScrolled)
			{
				ui->pattern.store ();
				ui->wheelScrolled = false;
			}

			Pad p0 = Pad ();
			for (int r = 0; r < ROWS; ++r)
			{
				for (int s = 0; s < MAXSTEPS; ++s)
				{
					ui->pattern.setPad (r, s, p0);
					ui->send_pad (r, s);
				}
			}

			ui->drawPad ();
			ui->pattern.store ();
		}
	}
}

void BSEQuencer_GUI::undoClickedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()) && (event->getWidget()->getMainWindow()))
	{
		BWidgets::Button* widget = (BWidgets::Button*)event->getWidget();
		BSEQuencer_GUI* ui = (BSEQuencer_GUI*)(widget->getMainWindow());
		double value = ((BEvents::ValueChangedEvent*) event)->getValue ();

		if (value == 1)
		{
			if ((widget == (BWidgets::Button*)&ui->toolUndoButton) || (widget == (BWidgets::Button*)&ui->toolRedoButton))
			{
				std::vector<PadMessage> padMessages = (widget == (BWidgets::Button*)&ui->toolUndoButton ? ui->pattern.undo () : ui->pattern.redo ());
				for (PadMessage const& p : padMessages)
				{
					size_t r = LIMIT (p.row, 0, ROWS);
					size_t s = LIMIT (p.step, 0, MAXSTEPS);
					ui->send_pad (r, s);
				}
				ui->drawPad ();
			}
		}
	}
}

void BSEQuencer_GUI::padsPressedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()) && (event->getWidget()->getMainWindow()) &&
		((event->getEventType () == BEvents::BUTTON_PRESS_EVENT) ||
		 (event->getEventType () == BEvents::BUTTON_RELEASE_EVENT) ||
		 (event->getEventType () == BEvents::POINTER_DRAG_EVENT)))
	{
		BWidgets::DrawingSurface* widget = (BWidgets::DrawingSurface*) event->getWidget ();
		BSEQuencer_GUI* ui = (BSEQuencer_GUI*) widget->getMainWindow();
		BEvents::PointerEvent* pointerEvent = (BEvents::PointerEvent*) event;

		if (ui->wheelScrolled)
		{
			ui->pattern.store ();
			ui->wheelScrolled = false;
		}

		// Get size of drawing area
		const double width = ui->padSurface.getEffectiveWidth ();
		const double height = ui->padSurface.getEffectiveHeight ();

		int row = (ROWS - 1) - ((int) ((pointerEvent->getY () - widget->getYOffset()) / (height / ROWS)));
		int step = (pointerEvent->getX () - widget->getXOffset()) / (width / ui->controllerWidgets[NR_OF_STEPS]->getValue ());

		if ((event->getEventType () == BEvents::BUTTON_PRESS_EVENT) || (event->getEventType () == BEvents::POINTER_DRAG_EVENT))
		{

			if ((row >= 0) && (row < ROWS) && (step >= 0) && (step < (int (ui->controllerWidgets[NR_OF_STEPS]->getValue ()))))
			{
				Pad pd = ui->pattern.getPad (row, step);
				//int pdch = ((int)pd->ch) & 0x0F;
				int pdctrl = (int (pd.ch) & 0xF0) / 0x10;

				// Left button: apply properties to pad
				if (pointerEvent->getButton() == BEvents::LEFT_BUTTON)
				{
					if (ui->controllerWidgets[SELECTION_CH]->getValue() <= NR_SEQUENCER_CHS)
					{
						//std::cerr << "BSEQuencer.lv2#GUI: Pad CH at " << row << ", " << step << "\n";
						Pad props
						(
							ui->controllerWidgets[SELECTION_CH]->getValue() + pdctrl * 0x10,
							ui->controllerWidgets[SELECTION_OCTAVE]->getValue(),
							ui->controllerWidgets[SELECTION_VELOCITY]->getValue(),
							ui->controllerWidgets[SELECTION_DURATION]->getValue()
						);

						// Click on a pad with same settings as in toolbox => temporarily switch to delete
						if ((props == pd) && (!ui->tempTool) && (event->getEventType () == BEvents::BUTTON_PRESS_EVENT))
						{
							ui->tempTool = true;
							ui->tempToolCh = ui->controllerWidgets[SELECTION_CH]->getValue();
							props.ch = pdctrl * 0x10;
							ui->controllerWidgets[SELECTION_CH]->setValue (0);
						}

						// Overwrite if new data
						if (!(props == pd))
						{
							ui->pattern.setPad (row, step, props);
							ui->drawPad (row, step);
							ui->send_pad (row, step);
						}
					}

					// CTRL function set
					else if (ui->controllerWidgets[SELECTION_CH]->getValue() <= NR_SEQUENCER_CHS + NR_CTRL_BUTTONS)
					{
						int ctrl = ((int)ui->controllerWidgets[SELECTION_CH]->getValue() - NR_SEQUENCER_CHS - 1) * 0x10;

						// Click on a pad with same settings as in toolbox => temporarily switch to delete
						if (((int (pd.ch) & 0xF0) == ctrl) && (!ui->tempTool) && (event->getEventType () == BEvents::BUTTON_PRESS_EVENT))
						{
							ui->tempTool = true;
							ui->tempToolCh = ui->controllerWidgets[SELECTION_CH]->getValue();
							ui->controllerWidgets[SELECTION_CH]->setValue(NR_SEQUENCER_CHS + 1);
							ctrl = ((int)ui->controllerWidgets[SELECTION_CH]->getValue() - NR_SEQUENCER_CHS - 1) * 0x10;
						}

						// Apply controller data
						// Whole step button pressed ?
						int startrow = row;
						int endrow = row;
						if (ui->toolWholeStepButton.getValue() == 1.0)
						{
							startrow = 0;
							endrow = ROWS - 1;
						}

						for (int irow = startrow; irow <= endrow; ++irow)
						{
							// Overwrite if new data
							if ((int (ui->pattern.getPad (irow, step).ch) & 0xF0) != ctrl)
							{
								Pad iPad = ui->pattern.getPad (irow, step);
								iPad.ch = (int (iPad.ch) & 0x0F) + ctrl;
								ui->pattern.setPad (irow, step, iPad);
								ui->drawPad (irow, step);
								ui->send_pad (irow, step);
							}
						}
					}

					// Edit mode
					else
					{
						int edit = ((int)ui->controllerWidgets[SELECTION_CH]->getValue() - NR_SEQUENCER_CHS - NR_CTRL_BUTTONS) * 0x100;

						if (edit == EDIT_PICK)
						{
							ui->controllerWidgets[SELECTION_CH]->setValue (int (pd.ch) & 0x0F);
							ui->controllerWidgets[SELECTION_OCTAVE]->setValue(pd.pitchOctave);
							ui->controllerWidgets[SELECTION_VELOCITY]->setValue(pd.velocity);
							ui->controllerWidgets[SELECTION_DURATION]->setValue(pd.duration);
						}

						else if ((edit == EDIT_CUT) || (edit == EDIT_COPY) || (edit == EDIT_FLIPX) || (edit = EDIT_FLIPY))
						{
							if (ui->clipBoard.ready)
							{
								ui->clipBoard.origin = std::make_pair (row, step);
								ui->clipBoard.extends = std::make_pair (0, 0);
								ui->clipBoard.ready = false;
								ui->drawPad (row, step);
							}

							else
							{
								std::pair<int, int> newExtends = std::make_pair (row - ui->clipBoard.origin.first, step - ui->clipBoard.origin.second);
								if (newExtends != ui->clipBoard.extends)
								{
									ui->clipBoard.extends = newExtends;
									ui->drawPad ();
								}
							}
						}

						else if (edit == EDIT_PASTE)
						{
							if (!ui->clipBoard.data.empty ())
							{
								for (int r = 0; r < int (ui->clipBoard.data.size ()); ++r)
								{
									for (int s = 0; s < int (ui->clipBoard.data[r].size ()); ++s)
									{
										if
										(
											(row - r >= 0) &&
											(row - r < ROWS) &&
											(step + s >= 0) &&
											(step + s < int (ui->controllerWidgets[NR_OF_STEPS]->getValue ()))
										)
										{
											ui->pattern.setPad (row - r, step + s, ui->clipBoard.data.at(r).at(s));
											ui->drawPad (row - r, step + s);
											ui->send_pad (row - r, step + s);
										}
									}
								}
							}
						}
					}
				}

				// Right button: copy pad to properties
				else if ((pointerEvent->getButton() == BEvents::RIGHT_BUTTON) &&
						 ((event->getEventType () == BEvents::BUTTON_PRESS_EVENT) ||
						  (event->getEventType () == BEvents::POINTER_DRAG_EVENT)))
				{
					ui->controllerWidgets[SELECTION_CH]->setValue (int (pd.ch) & 0x0F);
					ui->controllerWidgets[SELECTION_OCTAVE]->setValue(pd.pitchOctave);
					ui->controllerWidgets[SELECTION_VELOCITY]->setValue(pd.velocity);
					ui->controllerWidgets[SELECTION_DURATION]->setValue(pd.duration);
				}
			}
		}

		else if ((event->getEventType () == BEvents::BUTTON_RELEASE_EVENT) && (pointerEvent->getButton() == BEvents::LEFT_BUTTON))
		{
			// Edit mode
			if (ui->controllerWidgets[SELECTION_CH]->getValue() > NR_SEQUENCER_CHS + NR_CTRL_BUTTONS)
			{
				int edit = ((int)ui->controllerWidgets[SELECTION_CH]->getValue() - NR_SEQUENCER_CHS - NR_CTRL_BUTTONS) * 0x100;

				if ((edit == EDIT_CUT) || (edit == EDIT_COPY) || (edit == EDIT_FLIPX) || (edit == EDIT_FLIPY))
				{
					int clipRMin = ui->clipBoard.origin.first;
					int clipRMax = ui->clipBoard.origin.first + ui->clipBoard.extends.first;
					if (clipRMin > clipRMax) std::swap (clipRMin, clipRMax);
					int clipSMin = ui->clipBoard.origin.second;
					int clipSMax = ui->clipBoard.origin.second + ui->clipBoard.extends.second;
					if (clipSMin > clipSMax) std::swap (clipSMin, clipSMax);

					if (edit == EDIT_FLIPX)
					{
						for (int ds = 0; ds < int ((clipSMax + 1 - clipSMin) / 2); ++ds)
						{
							for (int r = clipRMin; r <= clipRMax; ++r)
							{
								Pad pd = ui->pattern.getPad (r, clipSMin + ds);
								ui->pattern.setPad (r, clipSMin + ds, ui->pattern.getPad (r, clipSMax - ds));
								ui->send_pad (r, clipSMin + ds);
								ui->pattern.setPad (r, clipSMax - ds, pd);
								ui->send_pad (r, clipSMax - ds);
							}
						}
						ui->pattern.store ();
					}

					if (edit == EDIT_FLIPY)
					{
						for (int dr = 0; dr < int ((clipRMax + 1 - clipRMin) / 2); ++dr)
						{
							for (int s = clipSMin; s <= clipSMax; ++s)
							{
								Pad pd = ui->pattern.getPad (clipRMin + dr, s);
								ui->pattern.setPad (clipRMin + dr, s, ui->pattern.getPad (clipRMax -dr, s));
								ui->send_pad (clipRMin + dr, s);
								ui->pattern.setPad (clipRMax - dr, s, pd);
								ui->send_pad (clipRMax - dr, s);
							}
						}
						ui->pattern.store ();
					}

					ui->clipBoard.data.clear ();
					for (int r = clipRMax; r >= clipRMin; --r)
					{
						std::vector<Pad> padRow;
						padRow.clear ();
						for (int s = clipSMin; s <= clipSMax; ++s) padRow.push_back (ui->pattern.getPad (r, s));
						ui->clipBoard.data.push_back (padRow);
					}

					if (edit == EDIT_CUT)
					{
						for (int r = clipRMax; r >= clipRMin; --r)
						{
							for (int s = clipSMin; s <= clipSMax; ++s)
							{
								ui->pattern.setPad (r, s,  Pad ());
								ui->send_pad (r, s);
							}
						}
						ui->pattern.store ();
					}

					ui->clipBoard.ready = true;

					// Hack focusWidget to cut / copy message
					if ((edit == EDIT_CUT) || (edit == EDIT_COPY))
					{
						if (ui->focusWidget)
						{
							ui->clipBoard.time = std::chrono::steady_clock::now() + ui->focusWidget->getFocusInMilliseconds ();
							ui->focusWidget->setFocused (true);
							if (ui->focusWidget->getParent()) ui->focusWidget->getParent()->release (ui->focusWidget);
							ui->add (*ui->focusWidget);
			       				ui->padSurfaceFocusText.setText (edit == EDIT_CUT ? "Cut..." : "Copied...");
			       				ui->scaleFocus ();
							ui->focusWidget->moveTo (widget->getOriginX () + pointerEvent->getX() + 2,
										 widget->getOriginY () + pointerEvent->getY() - ui->focusWidget->getHeight() - 2);
							ui->focusWidget->show ();
						}
					}

					ui->drawPad ();
				}
			}

			// On BUTTON_RELEASE_EVENT and temporary delete mode: switch back
			else if (ui->tempTool)
			{
				ui->tempTool = false;
				ui->controllerWidgets[SELECTION_CH]->setValue(ui->tempToolCh);
			}

			else ui->pattern.store ();
		}
	}
}

void BSEQuencer_GUI::padsScrolledCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()) && (event->getWidget()->getMainWindow()) &&
		((event->getEventType () == BEvents::WHEEL_SCROLL_EVENT)))
	{
		BWidgets::DrawingSurface* widget = (BWidgets::DrawingSurface*) event->getWidget ();
		BSEQuencer_GUI* ui = (BSEQuencer_GUI*) widget->getMainWindow();
		BEvents::WheelEvent* wheelEvent = (BEvents::WheelEvent*) event;

		// Get size of drawing area
		const double width = ui->padSurface.getEffectiveWidth ();
		const double height = ui->padSurface.getEffectiveHeight ();

		int row = (ROWS - 1) - ((int) ((wheelEvent->getY () - widget->getYOffset()) / (height / ROWS)));
		int step = (wheelEvent->getX () - widget->getXOffset()) / (width / ui->controllerWidgets[NR_OF_STEPS]->getValue ());

		if ((row >= 0) && (row < ROWS) && (step >= 0) && (step < ((int)ui->controllerWidgets[NR_OF_STEPS]->getValue ())))
		{
			Pad pd = ui->pattern.getPad (row, step);
			if (int (pd.ch) & 0x0F)
			{
				pd.velocity += 0.01 * wheelEvent->getDeltaY();
				pd.velocity = LIMIT (pd.velocity, 0.0, 2.0);
				ui->pattern.setPad (row, step, pd);
				ui->drawPad (row, step);
				ui->send_pad (row, step);
				ui->wheelScrolled = true;
			}
		}
	}
}

void BSEQuencer_GUI::padsFocusedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()) && (event->getWidget()->getMainWindow()))
	{
		BWidgets::DrawingSurface* widget = (BWidgets::DrawingSurface*) event->getWidget ();
		BSEQuencer_GUI* ui = (BSEQuencer_GUI*) widget->getMainWindow();
		BEvents::FocusEvent* focusEvent = (BEvents::FocusEvent*) event;

		// Get size of drawing area
		const double width = ui->padSurface.getEffectiveWidth ();
		const double height = ui->padSurface.getEffectiveHeight ();

		int row = (ROWS - 1) - ((int) ((focusEvent->getY () - widget->getYOffset()) / (height / ROWS)));
		int step = (focusEvent->getX () - widget->getXOffset()) / (width / ui->controllerWidgets[NR_OF_STEPS]->getValue ());

		if ((row >= 0) && (row < ROWS) && (step >= 0) && (step < ((int)ui->controllerWidgets[NR_OF_STEPS]->getValue ())))
		{
			Pad pd = ui->pattern.getPad (row, step);
			ui->padSurfaceFocusText.setText
			(
				"Channel: " + std::to_string (int (pd.ch) & 0x0f) + "\n" +
				"Octave: " + std::to_string (int (pd.pitchOctave)) + "\n" +
				"Velocity: " + BValues::toBString ("%1.2f", pd.velocity) + "\n" +
				"Duration: " + BValues::toBString ("%1.2f", pd.duration));
			ui->scaleFocus ();
		}
	}

	if (event->getEventType () == BEvents::FOCUS_IN_EVENT) focusInCallback (event);
	else if (event->getEventType () == BEvents::FOCUS_OUT_EVENT) focusOutCallback (event);
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
	cairo_select_font_face (cr, ctLabelFont.getFontFamily ().c_str (), ctLabelFont.getFontSlant (), ctLabelFont.getFontWeight ());

	int scaleNr = controllers[SCALE];
	BScale scale
	(
		((int)(controllers[ROOT] + controllers[SIGNATURE] + 12)) % 12,
		(SignatureIndex) controllers[SIGNATURE],
		scaleMaps[scaleNr].scaleNotes
	);
	std::string label = "";

	for (int i = 0; i < ROWS; ++i)
	{
		BColors::Color color = BColors::invisible;
		if (noteBits & (1 << i)) {color = ink; color.applyBrightness (0.75);}
		drawButton (surface, 0, (ROWS - i - 1) * height / ROWS + 1, width, height / ROWS - 2, color, NO_CTRL);

		ScaleMap* map = &(scaleMaps[scaleNr]);

		if (map->altSymbols[i] != "") label = map->altSymbols[i];
		else
		{
			int element = map->elements[i];
			label = scale.getSymbol (element);
			int note = scale.getMIDInote (element);
			if (note != ENOTE)
			{
				if (note >= 12) label += " +" + std::to_string ((int) (note / 12));
			}
			else label = "ERR";
		}

		cairo_text_extents_t ext;

		double fontsize = ctLabelFont.getFontSize () * sqrt (2);
		do
		{
			fontsize = fontsize / sqrt (2);
			cairo_set_font_size (cr, fontsize);
			cairo_text_extents (cr, label.c_str(), &ext);
		} while ((ext.width > width) && (fontsize >= ctLabelFont.getFontSize () * 0.5));

		cairo_move_to (cr, width / 2 - ext.width / 2, (15.5 - i) * height / 16 + ext.height / 2);
		cairo_show_text (cr, label.c_str());
	}

	cairo_destroy (cr);
	captionSurface.update ();
}

void BSEQuencer_GUI::drawPad ()
{
	cairo_surface_t* surface = padSurface.getDrawingSurface();
	cairo_t* cr = cairo_create (surface);
	for (int row = 0; row < ROWS; ++row)
	{
		for (int step = 0; step < ((int)controllerWidgets[NR_OF_STEPS]->getValue ()); ++step) drawPad (cr, row, step);
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
	if ((!cr) || (cairo_status (cr) != CAIRO_STATUS_SUCCESS) || (row < 0) || (row >= ROWS) || (step < 0) ||
		(step >= ((int)controllerWidgets[NR_OF_STEPS]->getValue ()))) return;

	// Get size of drawing area
	const double width = padSurface.getEffectiveWidth ();
	const double height = padSurface.getEffectiveHeight ();
	const double w = width / controllerWidgets[NR_OF_STEPS]->getValue ();
	const double h = height / ROWS;
	const double x = step * w;
	const double y = (ROWS - row - 1) * h;
	const double xr = round (x);
	const double yr = round (y);
	const double wr = round (x + w) - xr;
	const double hr = round (y + h) - yr;


	// Draw background
	// Odd or even?
	BColors::Color bg = (((int)(step / controllerWidgets[STEPS_PER]->getValue ())) % 2) ? oddPadBgColor : evenPadBgColor;

	// Highlight selection
	int clipRMin = clipBoard.origin.first;
	int clipRMax = clipBoard.origin.first + clipBoard.extends.first;
	if (clipRMin > clipRMax) std::swap (clipRMin, clipRMax);
	int clipSMin = clipBoard.origin.second;
	int clipSMax = clipBoard.origin.second + clipBoard.extends.second;
	if (clipSMin > clipSMax) std::swap (clipSMin, clipSMax);
	if ((!clipBoard.ready) && (row >= clipRMin) && (row <= clipRMax) && (step >= clipSMin) && (step <= clipSMax)) bg.applyBrightness (1.5);

	cairo_set_source_rgba (cr, CAIRO_RGBA (bg));
	cairo_set_line_width (cr, 0.0);
	cairo_rectangle (cr, xr, yr, wr, hr);
	cairo_fill (cr);

	// Draw pad
	Pad pd = pattern.getPad (row, step);
	int ch = (int (pd.ch) & 0x0F);
	int ctrl = (int (pd.ch) & 0xF0) / 0x10;
	double vel = (pd.velocity <= 1 ?  pd.velocity - 1 : (pd.velocity - 1) * 0.5);

	if ((ch >= 0) && (ch <= NR_SEQUENCER_CHS) && (ctrl >= 0) && (ctrl < NR_CTRL_BUTTONS))
	{
		BColors::Color color = chButtonStyles[ch].color;
		if (ch > 0) color.applyBrightness(vel);
		if (cursorBits[step] & (1 << row))
		{
			color.setAlpha (1.0);
			color.applyBrightness (0.75);
		}

		int symbol = ctrlButtonStyles[ctrl].symbol;

		drawButton (cr, xr + 1, yr + 1, wr - 2, hr - 2, color, symbol);
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

	// Reduce min GUI size for small displays
	double sz = 1.0;
	int screenWidth  = getScreenWidth ();
	int screenHeight = getScreenHeight ();
	if ((screenWidth < 840) || (screenHeight < 580)) sz = 0.5;
	else if ((screenWidth < 1240) || (screenHeight < 860)) sz = 0.66;

	/*
	std::cerr << "B.SEQuencer_GUI.lv2 screen size " << screenWidth << " x " << screenHeight <<
			". Set GUI size to " << 1200 * sz << " x " << 820 * sz << ".\n";
	*/

	if (resize) resize->ui_resize(resize->handle, 1200 * sz, 820 * sz);

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
