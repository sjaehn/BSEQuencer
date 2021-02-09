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
#include "BUtilities/to_string.hpp"
#include <exception>

BSEQuencer_GUI::BSEQuencer_GUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeView parentWindow) :
	Window (1250, 820, "B.SEQuencer", parentWindow, true, PUGL_MODULE, 0),
	controller (NULL), write_function (NULL),
	pluginPath (bundle_path ? std::string (bundle_path) : std::string ("")),
	sz (1.0), bgImageSurface (nullptr),
	uris (), forge (), clipBoard (),
	cursorBits {0}, noteBits (0), chBits (0),
	tempTool (false), tempToolCh (0), wheelScrolled (false), modifier (MODIFIER_VELOCITY),
	mContainer (0, 0, 1250, 820, "main"),
	padSurface (98, 88, 804, 484, "box"),
	captionSurface (18, 88, 64, 484, "box"),

	modeBox (920, 88, 310, 170, "box"),
	modeBoxLabel (10, 10, 290, 20, "ctlabel", "Play mode"),
	modeLabel (10, 80, 60, 20, "lflabel", "Mode"),
	modeListBox (80, 80, 220, 20, 220, 80, "menu", BItems::ItemList ({{1, "Autoplay"}, {3, "Host-controlled playback"} , {2, "Host & MIDI controlled"}}), 2.0),
	modeAutoplayBpmLabel (10, 115, 120, 20, "lflabel", "Beats per min"),
	modeAutoplayBpmSlider (120, 105, 180, 25, "slider", 120.0, 1.0, 300.0, 0.0, "%3.1f"),
	modeAutoplayBpbLabel (10, 145, 120, 20, "lflabel", "Beats per bar"),
	modeAutoplayBpbSlider (120, 135, 180, 25, "slider", 4.0, 1.0, 16.0, 1.0, "%2.0f"),
	modeMidiInChannelLabel (10, 110, 130, 20 , "lflabel", "MIDI input channel"),
	modeMidiInChannelListBox (180, 110, 120, 20, 120, 200, "menu",
				  BItems::ItemList ({{0, "All"}, {1, "1"}, {2, "2"}, {3, "3"}, {4, "4"}, {5, "5"}, {6, "6"}, {7, "7"}, {8, "8"}, {9, "9"},
						     {10, "10"}, {11, "11"}, {12, "12"}, {13, "13"}, {14, "14"}, {15, "15"}, {16, "16"}})),
	modeOnKeyLabel (10, 140, 100, 20, "lflabel", "On NOTE ON"),
	modeOnKeyListBox (180, 140, 120, 20, 120, 80, "menu", BItems::ItemList ({{0, "Restart"}, {2, "Restart & sync"}, {1, "Continue"}})),
	modePlayLabel (10, 50, 205, 20, "lflabel", "Status: playing ..."),
	modePlayButton (270, 40, 30, 30, "box", 1.0),

	toolBox (920, 280, 310, 292, "box"),
	toolBoxLabel (10, 10, 290, 20, "ctlabel", "Toolbox"),
	toolButtonBox (0, 30, 310, 160, "widget"),
	toolWholeStepButton (210, 40, 80, 20, "tgbutton", "Whole step", 0.0),
	toolResetButton (90, 130, 20, 20, "tgbutton"),
	toolUndoButton (120, 130, 20, 20, "tgbutton"),
	toolRedoButton (150, 130, 20, 20, "tgbutton"),
	toolButtonBoxCtrlLabel (10, 10, 60, 20, "lflabel", "Controls"),
	toolButtonBoxChLabel (10, 70, 60, 20, "lflabel", "Channels"),
	toolButtonBoxEditLabel (10, 100, 40, 20, "lflabel", "Edit"),
	toolGateLabel (10, 260, 30, 20, "ctlabel", "Gate"),
	toolGateSlider (12.5, 200, 25, 60, "dial", 1.0, 0.0, 1.0, 0.0, "%1.2f", BWidgets::LABEL_BOTTOM),
	toolNoteLabel (45, 260, 60, 20, "ctlabel", "Note"),
	toolNoteDial (50, 200, 50, 60, "dial", 0.0, -16.0, 16.0, 1.0, "%1.0f"),
	toolOctaveLabel (110, 260, 60, 20, "ctlabel", "Octave"),
	toolOctaveDial (115, 200, 50, 60, "dial", 0.0, -8.0, 8.0, 1.0, "%1.0f"),
	toolVelocityLabel (175, 260, 60, 20, "ctlabel", "Velocity"),
	toolVelocityDial  (180, 200, 50, 60, "dial", 1.0, 0.0, 2.0, 0.0, "%1.2f"),
	toolDurationLabel (240, 260, 60, 20, "ctlabel", "Duration"),
	toolDurationDial (245, 200, 50, 60, "dial", 1.0, 0.0, 1.0, 0.0, "%1.2f", UNIDIRECTIONAL),

	propertiesBox (920, 590, 310, 210, "box"),
	propertiesBoxLabel (10, 10, 290, 20, "ctlabel", "Properties"),
	propertiesNrStepsLabel (10, 50, 170, 20, "lflabel", "Total number of steps"),
	propertiesNrStepsListBox (210, 50, 90, 20, 90, 100, "menu",
				  BItems::ItemList ({{8, "8"}, {16, "16"}, {24, "24"}, {32, "32"}}), 16.0),
	propertiesStepsPerLabel (110, 85, 80, 20, "lflabel", "steps per"),
	propertiesStepsPerSlider (10, 75, 90, 25, "slider", 4.0, 1.0, 8.0, 1.0, "%2.0f"),
	propertiesBaseListBox (210, 85, 90, 20, 90, 60, "menu",
			       BItems::ItemList ({"beat", "bar"}), 1.0),
	propertiesRootLabel (10, 115, 40, 20, "lflabel", "Root"),
	propertiesRootListBox (110, 115, 90, 20, 0, -160, 90, 160, "menu",
			       BItems::ItemList ({{0, "C"}, {2, "D"}, {4, "E"}, {5, "F"}, {7, "G"}, {9, "A"}, {11, "B"}}), 0.0),
	propertiesSignatureListBox (210, 115, 90, 20, 90, 80, "menu",
				    BItems::ItemList ({{-1, "b"}, {0, ""}, {1, "#"}}), 0.0),
	propertiesOctaveLabel (10, 145, 55, 20, "lflabel", "Octave"),
	propertiesOctaveListBox (210, 145, 90, 20, 0, -220, 90, 220, "menu",
				 BItems::ItemList ({{-1, "-1"}, {0, "0"}, {1, "1"}, {2, "2"}, {3, "3"}, {4, "4"}, {5, "5"}, {6, "6"}, {7, "7"}, {8, "8"}}), 4.0),
	propertiesScaleLabel (10, 175, 50, 20, "lflabel", "Scale"),
	propertiesScaleEditIcon (70, 175, 20, 20, "widget", (bundle_path ? std::string (bundle_path) + EDIT_SYMBOL : std::string (""))),
	propertiesScaleListBox (),

	helpButton (1166, 50, 24, 24, "halobutton", "Help"),
	ytButton (1196, 50, 24, 24, "halobutton", "Feature tour"),
	scaleEditor (nullptr)
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
	propertiesScaleListBox = BWidgets::PopupListBox (100, 175, 200, 20, 0, -420, 200, 420, "menu", scaleItems, 0.0);
	propertiesScaleListBox.rename ("menu");

	// Init toolbox buttons
	toolButtonBox.addButton (90, 70, 20, 20, {{0.0, 0.03, 0.06, 1.0}, NO_CTRL, "No channel"});
	for (int i = 1; i < NR_SEQUENCER_CHS + 1; ++i) toolButtonBox.addButton (90 + i * 30, 70, 20, 20, chButtonStyles[i]);
	toolButtonBox.addButton (90 , 10, 20, 20, {{0.0, 0.03, 0.06, 1.0}, NO_CTRL, "No control"});
	for (int i = 1; i < NR_CTRL_BUTTONS; ++i) toolButtonBox.addButton (90 + (i % 7) * 30, 10 + ((int) (i / 7)) * 30, 20, 20, ctrlButtonStyles[i]);
	for (int i = 0; i < NR_EDIT_BUTTONS; ++i) toolButtonBox.addButton (90 + i * 30, 100, 20, 20, editButtonStyles[i]);

	// Init ChBoxes
	for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
	{
		chBoxes[i].box = BWidgets::Widget (98 + i * 203.5, 590, 193.5, 210, "box");
		chBoxes[i].chSymbol = BWidgets::DrawingSurface (7, 7, 26, 26, "button");
		chBoxes[i].chLabel = BWidgets::Label (40, 10, 133.5, 20, "ctlabel", "Channel " + std::to_string (i + 1));
		chBoxes[i].channelLabel = BWidgets::Label (10, 50, 100, 20, "lflabel", "MIDI channel");
		chBoxes[i].channelListBox = BWidgets::PopupListBox (123.5, 50, 60, 20, 60, 120, "menu",
								    BItems::ItemList({"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"}),
								    i + 1);
		chBoxes[i].pitchLabel = BWidgets::Label (10, 80, 80, 20, "lflabel", "Input pitch");
		chBoxes[i].pitchSwitch = BWidgets::HSwitch (132.5, 82, 42, 16, "ch" + std::to_string (i + 1), 0.0);
		chBoxes[i].pitchScreen = BWidgets::Widget (10, 80, 173.5, 20, "screen");
		chBoxes[i].pitchScreen.hide ();
		chBoxes[i].velocityDial = BWidgets::DialValue (25, 120, 50, 60, "ch" + std::to_string (i + 1), 1.0, 0.0, 2.0, 0.0, "%1.2f");
		chBoxes[i].velocityLabel = BWidgets::Label (20, 180, 60, 20, "ctlabel", "Velocity");
		chBoxes[i].noteOffsetDial = BWidgets::DialValue (118.5, 120, 50, 60, "ch" + std::to_string (i + 1), 0.0, -127.0, 127.0, 1.0, "%1.0f");
		chBoxes[i].noteOffsetLabel = BWidgets::Label (113.5, 180, 60, 20, "ctlabel", "Offset");
	}

	// Link controllerWidgets
	controllerWidgets[MIDI_IN_CHANNEL] = (BWidgets::ValueWidget*) &modeMidiInChannelListBox;
	controllerWidgets[PLAY] = (BWidgets::ValueWidget*) &modePlayButton;
	controllerWidgets[MODE] = (BWidgets::ValueWidget*) &modeListBox;
	controllerWidgets[ON_KEY_PRESSED] = (BWidgets::ValueWidget*) &modeOnKeyListBox;
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
	controllerWidgets[SELECTION_NOTE] = (BWidgets::ValueWidget*) &toolNoteDial;
	controllerWidgets[SELECTION_OCTAVE] = (BWidgets::ValueWidget*) &toolOctaveDial;
	controllerWidgets[SELECTION_VELOCITY] = (BWidgets::ValueWidget*) &toolVelocityDial;
	controllerWidgets[SELECTION_DURATION] = (BWidgets::ValueWidget*) &toolDurationDial;
	controllerWidgets[SELECTION_GATE_RAND] = (BWidgets::ValueWidget*) &toolGateSlider;
	controllerWidgets[SELECTION_NOTE_RAND] = (BWidgets::ValueWidget*) &toolNoteDial.range;
	controllerWidgets[SELECTION_OCTAVE_RAND] = (BWidgets::ValueWidget*) &toolOctaveDial.range;
	controllerWidgets[SELECTION_VELOCITY_RAND] = (BWidgets::ValueWidget*) &toolVelocityDial.range;
	controllerWidgets[SELECTION_DURATION_RAND] = (BWidgets::ValueWidget*) &toolDurationDial.range;

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
	padSurface.setCallbackFunction (BEvents::FOCUS_IN_EVENT, padsFocusedCallback);
	padSurface.setCallbackFunction (BEvents::FOCUS_OUT_EVENT, padsFocusedCallback);
	padSurface.setMergeable (BEvents::POINTER_DRAG_EVENT, false);

	toolResetButton.setCallbackFunction(BEvents::VALUE_CHANGED_EVENT, resetClickedCallback);
	toolUndoButton.setCallbackFunction(BEvents::VALUE_CHANGED_EVENT, undoClickedCallback);
	toolRedoButton.setCallbackFunction(BEvents::VALUE_CHANGED_EVENT, undoClickedCallback);

	helpButton.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, helpPressedCallback);
	ytButton.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, ytPressedCallback);
	propertiesScaleEditIcon.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, editPressedCallback);

	// Apply theme
	bgImageSurface = cairo_image_surface_create_from_png ((pluginPath + BG_FILE).c_str());
	widgetBg.loadFillFromCairoSurface (bgImageSurface);
	applyTheme (theme);

	toolDurationDial.range.setMax (0.0);
	toolGateSlider.setHardChangeable (false);
	toolNoteDial.setHardChangeable (false);
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
	modeBox.add (modeOnKeyLabel);
	modeBox.add (modeOnKeyListBox);
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

	toolBox.add (toolGateLabel);
	toolBox.add (toolGateSlider);
	toolBox.add (toolNoteLabel);
	toolBox.add (toolNoteDial);
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

	mContainer.add (captionSurface);
	mContainer.add (toolBox);
	mContainer.add (modeBox);
	mContainer.add (propertiesBox);
	mContainer.add (helpButton);
	mContainer.add (ytButton);
	mContainer.add (padSurface);
	for (int i = 0; i < NR_SEQUENCER_CHS; ++i) mContainer.add (chBoxes[i].box);

	drawCaption ();
	drawPad();
	add (mContainer);

	getKeyGrabStack()->add (this);

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

	send_ui_off ();
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
	cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1250 * sz, 820 * sz);
	cairo_t* cr = cairo_create (surface);
	cairo_scale (cr, sz, sz);
	cairo_set_source_surface(cr, bgImageSurface, 0, 0);
	cairo_paint(cr);
	widgetBg.loadFillFromCairoSurface(surface);
	cairo_destroy (cr);
	cairo_surface_destroy (surface);

	//Scale widgets
	RESIZE (mContainer, 0, 0, 1250, 820, sz);
	RESIZE (padSurface, 98, 88, 804, 484, sz);

	//RESIZE (padSurfaceFocusText, 0, 0, 100, 60, sz);
	scaleFocus ();
	RESIZE (captionSurface, 18, 88, 64, 484, sz);

	RESIZE (modeBox, 920, 88, 310, 180, sz);
	RESIZE (modeBoxLabel, 10, 10, 290, 20, sz);
	RESIZE (modeLabel, 10, 80, 60, 20, sz);
	RESIZE (modeListBox, 80, 80, 220, 20, sz);
	modeListBox.resizeListBox(BUtilities::Point (220 * sz, 80 * sz));
	modeListBox.resizeListBoxItems(BUtilities::Point (220 * sz, 20 * sz));
	RESIZE (modeAutoplayBpmLabel, 10, 115, 120, 20, sz);
	RESIZE (modeAutoplayBpmSlider, 120, 105, 180, 25, sz);
	RESIZE (modeAutoplayBpbLabel, 10, 145, 120, 20, sz);
	RESIZE (modeAutoplayBpbSlider, 120, 135, 180, 25, sz);
	RESIZE (modeMidiInChannelLabel, 10, 110, 130, 20, sz);
	RESIZE (modeMidiInChannelListBox, 180, 110, 120, 20, sz);
	modeMidiInChannelListBox.resizeListBox (BUtilities::Point (120 * sz, 200 * sz));
	modeMidiInChannelListBox.resizeListBoxItems (BUtilities::Point (120 * sz, 20 * sz));
	RESIZE (modeOnKeyLabel, 10, 140, 100, 20, sz);
	RESIZE (modeOnKeyListBox, 180, 140, 120, 20, sz);
	modeOnKeyListBox.resizeListBox (BUtilities::Point (120 * sz, 80 * sz));
	modeOnKeyListBox.resizeListBoxItems (BUtilities::Point (120 * sz, 20 * sz));
	RESIZE (modePlayLabel, 10, 50, 205, 20, sz);
	RESIZE (modePlayButton, 270, 40, 30, 30, sz);

	RESIZE (toolBox, 920, 280, 310, 292, sz);
	RESIZE (toolBoxLabel, 10, 10, 290, 20, sz);
	RESIZE (toolButtonBox, 0, 30, 310, 160, sz);
	RESIZE (toolWholeStepButton, 210, 40, 80, 20, sz);
	RESIZE (toolResetButton, 90, 130, 20, 20, sz);
	RESIZE (toolUndoButton, 120, 130, 20, 20, sz);
	RESIZE (toolRedoButton, 150, 130, 20, 20, sz);
	RESIZE (toolButtonBoxCtrlLabel, 10, 10, 60, 20, sz);
	RESIZE (toolButtonBoxChLabel, 10, 70, 60, 20, sz);
	RESIZE (toolButtonBoxEditLabel, 10, 100, 60, 20, sz);
	RESIZE (toolGateLabel, 10, 260, 30, 20, sz);
	RESIZE (toolGateSlider, 12.5, 200, 25, 60, sz);
	RESIZE (toolNoteLabel, 45, 260, 60, 20, sz);
	RESIZE (toolNoteDial, 50, 200, 50, 60, sz);
	RESIZE (toolOctaveLabel, 110, 260, 60, 20, sz);
	RESIZE (toolOctaveDial, 115, 200, 50, 60, sz);
	RESIZE (toolVelocityLabel, 175, 260, 60, 20, sz);
	RESIZE (toolVelocityDial, 180, 200, 50, 60, sz);
	RESIZE (toolDurationLabel, 240, 260, 60, 20, sz);
	RESIZE (toolDurationDial, 245, 200, 50, 60, sz);

	RESIZE (propertiesBox, 920, 590, 310, 210, sz);
	RESIZE (propertiesBoxLabel, 10, 10, 290, 20, sz);
	RESIZE (propertiesNrStepsLabel, 10, 50, 170, 20, sz);
	RESIZE (propertiesNrStepsListBox, 210, 50, 90, 20, sz);
	propertiesNrStepsListBox.resizeListBox (BUtilities::Point (90 * sz, 100 * sz));
	propertiesNrStepsListBox.resizeListBoxItems (BUtilities::Point (90 * sz, 20 * sz));
	RESIZE (propertiesStepsPerSlider, 10, 75, 90, 25, sz);
	RESIZE (propertiesStepsPerLabel, 110, 85, 80, 20, sz);
	RESIZE (propertiesBaseListBox, 210, 85, 90, 20, sz);
	propertiesBaseListBox.resizeListBox (BUtilities::Point (90 * sz, 60 * sz));
	propertiesBaseListBox.resizeListBoxItems (BUtilities::Point (90 * sz, 20 * sz));
	RESIZE (propertiesRootLabel, 10, 115, 40, 20, sz);
	RESIZE (propertiesRootListBox, 110, 115, 90, 20, sz);
	propertiesRootListBox.resizeListBox (BUtilities::Point (90 * sz, 160 * sz));
	propertiesRootListBox.moveListBox (BUtilities::Point (0, -160 * sz));
	propertiesRootListBox.resizeListBoxItems (BUtilities::Point (90 * sz, 20 * sz));
	RESIZE (propertiesSignatureListBox, 210, 115, 90, 20, sz);
	propertiesSignatureListBox.resizeListBox (BUtilities::Point (90 * sz, 80 * sz));
	propertiesSignatureListBox.resizeListBoxItems (BUtilities::Point (90 * sz, 20 * sz));
	RESIZE (propertiesOctaveLabel, 10, 145, 55, 20, sz);
	RESIZE (propertiesOctaveListBox, 210, 145, 90, 20, sz);
	propertiesOctaveListBox.resizeListBox (BUtilities::Point (90 * sz, 220 * sz));
	propertiesOctaveListBox.moveListBox (BUtilities::Point (0, -220 * sz));
	propertiesOctaveListBox.resizeListBoxItems (BUtilities::Point (90 * sz, 20 * sz));
	RESIZE (propertiesScaleLabel, 10, 175, 50, 20, sz);
	RESIZE (propertiesScaleEditIcon, 70, 175, 20, 20, sz);
	RESIZE (propertiesScaleListBox, 100, 175, 200, 20, sz);
	propertiesScaleListBox.resizeListBox (BUtilities::Point (200 * sz, 420 * sz));
	propertiesScaleListBox.moveListBox (BUtilities::Point (0, -420 * sz));
	propertiesScaleListBox.resizeListBoxItems (BUtilities::Point (200 * sz, 20 * sz));

	RESIZE (helpButton, 1166, 50, 24, 24, sz);
	RESIZE (ytButton, 1196, 50, 24, 24, sz);
	if (scaleEditor) {RESIZE ((*scaleEditor), 420, 20, 360, 760, sz);}

	for (int i = 0; i < NR_SEQUENCER_CHS; ++i)
	{
		RESIZE (chBoxes[i].box, 98 + i * 203.5, 590, 193.5, 210, sz);
		RESIZE (chBoxes[i].chSymbol, 7, 7, 26, 26, sz);
		RESIZE (chBoxes[i].chLabel, 40, 10, 133.5, 20, sz);
		RESIZE (chBoxes[i].channelLabel, 10, 50, 100, 20, sz);
		RESIZE (chBoxes[i].channelListBox, 123.5, 50, 60, 20, sz);
		chBoxes[i].channelListBox.resizeListBox (BUtilities::Point (60 * sz, 120 * sz));
		chBoxes[i].channelListBox.resizeListBoxItems (BUtilities::Point (60 * sz, 20 * sz));
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

	padSurface.focusText.resize (400,100);	// Maximize size first to omit breaks
	std::vector<std::string> textblock = padSurface.focusText.getTextBlock ();
	double blockheight = padSurface.focusText.getTextBlockHeight (textblock);
	double blockwidth = 0.0;
	for (std::string textline : textblock)
	{
		cairo_text_extents_t ext = padSurface.focusText.getFont ()->getTextExtents (cr, textline);
		if (ext.width > blockwidth) blockwidth = ext.width;
	}
	padSurface.focusText.resize (blockwidth + 2 * padSurface.focusText.getXOffset (), blockheight + 2 * padSurface.focusText.getYOffset ());

	padSurface.focusText.resize();

	cairo_destroy (cr);
}

void BSEQuencer_GUI::applyTheme (BStyles::Theme& theme)
{
	mContainer.applyTheme (theme);

	padSurface.applyTheme (theme);
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
	modeOnKeyLabel.applyTheme (theme);
	modeOnKeyListBox.applyTheme (theme);
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
	toolGateLabel.applyTheme (theme);
	toolGateSlider.applyTheme (theme);
	toolNoteLabel.applyTheme (theme);
	toolNoteDial.applyTheme (theme);
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

	helpButton.applyTheme (theme);
	ytButton.applyTheme (theme);
	//scaleEditor.applyTheme (theme);
}

void BSEQuencer_GUI::onConfigureRequest (BEvents::ExposeEvent* event)
{
	Window::onConfigureRequest (event);

	sz = (getWidth() / 1250 > getHeight() / 820 ? getHeight() / 820 : getWidth() / 1250);
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

void BSEQuencer_GUI::onKeyPressed (BEvents::KeyEvent* event)
{
	if (!event) return;
	if (event->getKey() == BDevices::KEY_SHIFT) modifier = MODIFIER_DURATION;
	else if (event->getKey() == BDevices::KEY_CTRL) modifier = MODIFIER_OCTAVE;
}

void BSEQuencer_GUI::onKeyReleased (BEvents::KeyEvent* event)
{
	if ((event) && ((event->getKey() == BDevices::KEY_SHIFT) || (event->getKey() == BDevices::KEY_CTRL))) modifier = MODIFIER_VELOCITY;
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
						ui->modeOnKeyLabel.hide ();
						ui->modeOnKeyListBox.hide ();
						for (int i = 0; i < NR_SEQUENCER_CHS; ++i) ui->chBoxes[i].pitchScreen.show ();
					}

					else if (value == HOST_CONTROLLED)
					{
						ui->modeAutoplayBpmLabel.hide ();
						ui->modeAutoplayBpmSlider.hide ();
						ui->modeAutoplayBpbLabel.hide ();
						ui->modeAutoplayBpbSlider.hide ();
						ui->modeMidiInChannelLabel.show ();
						ui->modeMidiInChannelListBox.show ();
						ui->modeOnKeyLabel.show ();
						ui->modeOnKeyListBox.show ();
						for (int i = 0; i < NR_SEQUENCER_CHS; ++i) ui->chBoxes[i].pitchScreen.hide ();
					}

					else
					{
						ui->modeAutoplayBpmLabel.hide ();
						ui->modeAutoplayBpmSlider.hide ();
						ui->modeAutoplayBpbLabel.hide ();
						ui->modeAutoplayBpbSlider.hide ();
						ui->modeMidiInChannelLabel.hide ();
						ui->modeMidiInChannelListBox.hide ();
						ui->modeOnKeyLabel.hide ();
						ui->modeOnKeyListBox.hide ();
						for (int i = 0; i < NR_SEQUENCER_CHS; ++i) ui->chBoxes[i].pitchScreen.show ();
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

				// Range dials: also update parents
				if
				(
					(widgetNr == SELECTION_NOTE_RAND) ||
					(widgetNr == SELECTION_OCTAVE_RAND) ||
					(widgetNr == SELECTION_VELOCITY_RAND) ||
					(widgetNr == SELECTION_DURATION_RAND)
				)
				{
					if (widget->getParent()) widget->getParent()->update();
				}
			}
		}
	}
}

void BSEQuencer_GUI::helpPressedCallback (BEvents::Event* event)
{
	if (system(OPEN_CMD " " HELP_URL)) std::cerr << "BSEQuencer.lv2#GUI: Can't open " << HELP_URL << ". You can try to call it maually.";
}

void BSEQuencer_GUI::ytPressedCallback (BEvents::Event* event)
{
	if (system(OPEN_CMD " " YT_URL))  std::cerr << "BSEQuencer.lv2#GUI: Can't open " << YT_URL << ". You can try to call it maually.";
}

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

		int row = (ROWS - 1) - ((int) ((pointerEvent->getPosition ().y - widget->getYOffset()) / (height / ROWS)));
		int step = (pointerEvent->getPosition ().x - widget->getXOffset()) / (width / ui->controllerWidgets[NR_OF_STEPS]->getValue ());

		if ((event->getEventType () == BEvents::BUTTON_PRESS_EVENT) || (event->getEventType () == BEvents::POINTER_DRAG_EVENT))
		{

			if ((row >= 0) && (row < ROWS) && (step >= 0) && (step < (int (ui->controllerWidgets[NR_OF_STEPS]->getValue ()))))
			{
				int start = step;
				while (ui->pattern.padHasAntecessor (row, start)) --start;
				Pad pd = ui->pattern.getPad (row, start);
				int pdctrl = (int (pd.ch) & 0xF0) / 0x10;

				// Left button: apply properties to pad
				if (pointerEvent->getButton() == BDevices::LEFT_BUTTON)
				{
					if (ui->controllerWidgets[SELECTION_CH]->getValue() <= NR_SEQUENCER_CHS)
					{
						Pad props
						(
							ui->controllerWidgets[SELECTION_CH]->getValue() + pdctrl * 0x10,
							ui->controllerWidgets[SELECTION_NOTE]->getValue(),
							ui->controllerWidgets[SELECTION_OCTAVE]->getValue(),
							ui->controllerWidgets[SELECTION_VELOCITY]->getValue(),
							ui->controllerWidgets[SELECTION_DURATION]->getValue() + int (pd.duration - 0.00000001),
							ui->controllerWidgets[SELECTION_GATE_RAND]->getValue(),
							ui->controllerWidgets[SELECTION_NOTE_RAND]->getValue(),
							ui->controllerWidgets[SELECTION_OCTAVE_RAND]->getValue(),
							ui->controllerWidgets[SELECTION_VELOCITY_RAND]->getValue(),
							ui->controllerWidgets[SELECTION_DURATION_RAND]->getValue()
						);

						// Delete ch if duration == 0
						if (props.duration == 0.0f) props.ch = pdctrl * 0x10;

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
							int s = 0;
							float d = props.duration;
							do
							{
								props.duration = ((int (props.ch) & 0x0F) == 0 ? ui->controllerWidgets[SELECTION_DURATION]->getValue() : d);
								ui->pattern.setPad (row, start + s, props);
								ui->drawPad (row, start + s);
								ui->send_pad (row, start + s);
								--d;
								++s;
							} while (d > 0.0);
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
							int istart = step;
							while (ui->pattern.padHasAntecessor (irow, istart)) --istart;

							// Overwrite if new data
							if ((int (ui->pattern.getPad (irow, istart).ch) & 0xF0) != ctrl)
							{
								int istep = istart;
								do
								{
									Pad iPad = ui->pattern.getPad (irow, istep);
									iPad.ch = (int (iPad.ch) & 0x0F) + (istep == istart ? ctrl : 0);
									ui->pattern.setPad (irow, istep, iPad);
									ui->drawPad (irow, istep);
									ui->send_pad (irow, istep);
									++istep;
								} while (ui->pattern.padHasSuccessor (irow, istep - 1));
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

						else if (edit == EDIT_MERGE)
						{
							if (ui->clipBoard.ready)
							{
								ui->clipBoard.origin = std::make_pair (row, start);
								ui->clipBoard.extends = std::make_pair (0, ui->pattern.padGetSize (row, start) - 1);
								ui->clipBoard.ready = false;
								ui->drawPad (row, step);
							}

							else
							{
								std::pair<int, int> newExtends = std::make_pair (0, step - ui->clipBoard.origin.second);
								if (newExtends != ui->clipBoard.extends)
								{
									ui->clipBoard.extends = newExtends;
									ui->drawPad ();
								}
							}
						}

						else if ((edit == EDIT_CUT) || (edit == EDIT_COPY) || (edit == EDIT_FLIPX) || (edit == EDIT_FLIPY))
						{
							if (ui->clipBoard.ready)
							{
								ui->clipBoard.origin = std::make_pair (row, start);
								ui->clipBoard.extends = std::make_pair (0, ui->pattern.padGetSize (row, start) - 1);
								ui->clipBoard.ready = false;
								ui->drawPad (row, step);
							}

							else
							{
								int end = step;
								while (ui->pattern.padHasSuccessor (row, end)) ++end;
								std::pair<int, int> newExtends = std::make_pair (row - ui->clipBoard.origin.first, end - ui->clipBoard.origin.second);
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
									// Clip merged pads
									ui->padClip (row - r, step);
									if (ui->clipBoard.data[r].size () > 1) ui->padClip (row - r, step + ui->clipBoard.data[r].size ());

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
				else if ((pointerEvent->getButton() == BDevices::RIGHT_BUTTON) &&
						 ((event->getEventType () == BEvents::BUTTON_PRESS_EVENT) ||
						  (event->getEventType () == BEvents::POINTER_DRAG_EVENT)))
				{
					ui->controllerWidgets[SELECTION_CH]->setValue (int (pd.ch) & 0x0F);
					ui->controllerWidgets[SELECTION_GATE_RAND]->setValue (pd.randGate);
					ui->controllerWidgets[SELECTION_NOTE]->setValue (pd.pitchNote);
					ui->controllerWidgets[SELECTION_OCTAVE]->setValue (pd.pitchOctave);
					ui->controllerWidgets[SELECTION_VELOCITY]->setValue (pd.velocity);
					ui->controllerWidgets[SELECTION_DURATION]->setValue (LIMIT (pd.duration, 0.0, 1.0));
					ui->controllerWidgets[SELECTION_NOTE_RAND]->setValue (pd.randNote);
					ui->controllerWidgets[SELECTION_OCTAVE_RAND]->setValue (pd.randOctave);
					ui->controllerWidgets[SELECTION_VELOCITY_RAND]->setValue (pd.randVelocity);
					ui->controllerWidgets[SELECTION_DURATION_RAND]->setValue (pd.randDuration);
				}
			}
		}

		else if ((event->getEventType () == BEvents::BUTTON_RELEASE_EVENT) && (pointerEvent->getButton() == BDevices::LEFT_BUTTON))
		{
			// Edit mode
			if (ui->controllerWidgets[SELECTION_CH]->getValue() > NR_SEQUENCER_CHS + NR_CTRL_BUTTONS)
			{
				int edit = ((int)ui->controllerWidgets[SELECTION_CH]->getValue() - NR_SEQUENCER_CHS - NR_CTRL_BUTTONS) * 0x100;

				if (edit == EDIT_MERGE)
				{
					int clipRMin = ui->clipBoard.origin.first;
					int clipRMax = ui->clipBoard.origin.first + ui->clipBoard.extends.first;
					if (clipRMin > clipRMax) std::swap (clipRMin, clipRMax);
					int clipSMin = ui->clipBoard.origin.second;
					int clipSMax = ui->clipBoard.origin.second + ui->clipBoard.extends.second;
					if (clipSMin > clipSMax) std::swap (clipSMin, clipSMax);


					// Split
					if (ui->pattern.padGetSize (clipRMin, clipSMin) > 1)
					{
						// Find start
						int start = clipSMin;
						while (ui->pattern.padHasAntecessor (clipRMin, start)) --start;

						// Find end
						int end = clipSMax;
						while (ui->pattern.padHasSuccessor (clipRMin, end)) ++end;

						// Limit pad duration to 1.0 (= split)
						for (int i = start; i <= end; ++i)
						{
							Pad pd = ui->pattern.getPad (clipRMin, i);
							pd.duration = LIMIT (pd.duration, 0.0, 1.0);
							ui->pattern.setPad (clipRMin, i, pd);
							ui->send_pad (clipRMin, i);
						}
					}

					// Merge
					else
					{
						// Find start
						int start = clipSMin;
						while (ui->pattern.padHasAntecessor (clipRMin, start)) --start;

						// Find end
						int end = clipSMax;
						while (ui->pattern.padHasSuccessor (clipRMin, end)) ++end;

						Pad pd0 = ui->pattern.getPad (clipRMin, start);

						// Count number of mergeable pads
						int size = 0;
						do ++size;
						while
						(
							(start + size - 1 < end) &&
							((int (pd0.ch) & 0x0F) == (int (ui->pattern.getPad (clipRMin, start + size).ch) & 0x0F)) &&
							(ui->pattern.getPad (clipRMin, start + size).duration > 0.0)
						);

						// TODO Set control to first element
						int ctrl = 0;
						for (int i = 0; i < size; ++i)
						{
							Pad pdi = ui->pattern.getPad (clipRMin, start + i);
							if (int (pdi.ch) & 0xF0)
							{
								ctrl = int (pdi.ch) & 0xF0;
								break;
							}
						}
						pd0.ch = int (pd0.ch) | ctrl;

						pd0.duration = ui->pattern.getPad (clipRMin, start + size - 1).duration + size - 1;

						for (int i = 0; i < size; ++i)
						{
							ui->pattern.setPad (clipRMin, start + i, pd0);
							ui->send_pad (clipRMin, start + i);
							--pd0.duration;
							if (i == 0) pd0.ch = int (pd0.ch) & 0x0F;
						}
					}

					ui->clipBoard.ready = true;
					ui->drawPad();
					ui->pattern.store ();
				}

				else if ((edit == EDIT_CUT) || (edit == EDIT_COPY) || (edit == EDIT_FLIPX) || (edit == EDIT_FLIPY))
				{
					int clipRMin = ui->clipBoard.origin.first;
					int clipRMax = ui->clipBoard.origin.first + ui->clipBoard.extends.first;
					if (clipRMin > clipRMax) std::swap (clipRMin, clipRMax);
					int clipSMin = ui->clipBoard.origin.second;
					int clipSMax = ui->clipBoard.origin.second + ui->clipBoard.extends.second;
					if (clipSMin > clipSMax) std::swap (clipSMin, clipSMax);

					// Clip merged Pads
					if ((edit == EDIT_CUT) || (edit == EDIT_FLIPX) || (edit == EDIT_FLIPY))
					{
						for (int r = clipRMin; r < clipRMax; ++r)
						{
							ui->padClip (r, clipSMin);
							if (clipSMin != clipSMax) ui->padClip (r, clipSMax + 1);
						}
					}

					// Copy to clipboard
					ui->clipBoard.data.clear ();
					for (int r = clipRMax; r >= clipRMin; --r)
					{
						std::vector<Pad> padRow;
						padRow.clear ();
						for (int s = clipSMin; s <= clipSMax; ++s)
						{
							Pad pd = ui->pattern.getPad (r, s);

							// Clip oversized merged pads
							if (float(s) + pd.duration > float(clipSMax + 1.0f)) pd.duration = clipSMax + 1 - s;

							padRow.push_back (pd);
						}
						ui->clipBoard.data.push_back (padRow);
					}

					if (edit == EDIT_FLIPX)
					{
						for (int dr = 0; dr < int (ui->clipBoard.data.size()); ++dr)
						{
							for (int ds = 0; ds < int (ui->clipBoard.data[dr].size());)
							{
								Pad pd = ui->clipBoard.data.at (dr).at (ds);
								int pdsize = (pd.duration <= 0.0f ? 1.0 : int (ceil (pd.duration)));

								for (int dds = 0; dds < pdsize; ++dds)
								{
									Pad pds = ui->clipBoard.data.at (dr).at (ds + dds);
									ui->pattern.setPad (clipRMax - dr, clipSMax + 1 + dds - ds - pdsize, pds);
									ui->send_pad (clipRMax - dr, clipSMax + 1 + dds - ds - pdsize);
								}

								ds += pdsize;
							}
						}
						ui->pattern.store ();
					}

					else if (edit == EDIT_FLIPY)
					{
						for (int dr = 0; dr < int (ui->clipBoard.data.size()); ++dr)
						{
							for (int ds = 0; ds < int (ui->clipBoard.data[dr].size()); ++ds)
							{
								Pad pd = ui->clipBoard.data.at (dr).at (ds);
								ui->pattern.setPad (clipRMin + dr, clipSMin + ds, pd);
								ui->send_pad (clipRMin + dr, clipSMin + ds);
							}
						}
						ui->pattern.store ();
					}

					// Cut: Clear pads
					else if (edit == EDIT_CUT)
					{
						for (int r = clipRMax; r >= clipRMin; --r)
						{
							for (int s = clipSMin; s <= clipSMax; ++s)
							{
								ui->pattern.setPad (r, s, Pad ());
								ui->send_pad (r, s);
							}
						}
						ui->pattern.store ();
					}

					ui->clipBoard.ready = true;
					ui->drawPad ();
				}
			}

			// On BUTTON_RELEASE_EVENT and temporary delete mode: switch back
			else if (ui->tempTool)
			{
				ui->tempTool = false;
				ui->controllerWidgets[SELECTION_CH]->setValue (ui->tempToolCh);
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

		int row = (ROWS - 1) - ((int) ((wheelEvent->getPosition ().y - widget->getYOffset()) / (height / ROWS)));
		int step = (wheelEvent->getPosition ().x - widget->getXOffset()) / (width / ui->controllerWidgets[NR_OF_STEPS]->getValue ());

		if ((row >= 0) && (row < ROWS) && (step >= 0) && (step < ((int)ui->controllerWidgets[NR_OF_STEPS]->getValue ())))
		{
			while (ui->pattern.padHasAntecessor (row, step)) --step;
			Pad pd = ui->pattern.getPad (row, step);
			if (int (pd.ch) & 0x0F)
			{
				// SHIFT: Change duration
				if (ui->modifier == MODIFIER_DURATION)
				{
					float d = pd.duration * (1.0f + 0.01f * wheelEvent->getDelta().y);
					if ((d >= 0.0f) && (d <= int (ui->controllerWidgets[NR_OF_STEPS]->getValue() - step)))
					{

						// Delete last step of pad shinked
						if (int (ceil(d)) < int (ceil (pd.duration)))
						{
							Pad pds = ui->pattern.getPad (row, step + int (pd.duration));
							pds.ch = int (pds.ch) & 0xF0;
							pds.duration = 0.0f;
							ui->pattern.setPad (row, step + int (pd.duration), pds);
							ui->drawPad (row, step + int (pd.duration));
							ui->send_pad (row, step + int (pd.duration));
						}

						// Change duration
						do
						{
							pd.duration = d;
							ui->pattern.setPad (row, step, pd);
							ui->drawPad (row, step);
							ui->send_pad (row, step);
							pd.ch = int (pd.ch) & 0x0F;
							++step;
							--d;
						} while (d > 0.0f);
					}
				}

				// CTRL: Change pitch octave
				else if (ui->modifier == MODIFIER_OCTAVE)
				{
					int o = pd.pitchOctave + wheelEvent->getDelta().y;
					o = LIMIT (o, -8, 8);
					do
					{
						Pad pds = ui->pattern.getPad (row, step);
						pds.pitchOctave = o;
						ui->pattern.setPad (row, step, pds);
						ui->drawPad (row, step);
						ui->send_pad (row, step);
						++step;
					} while (ui->pattern.padHasSuccessor (row, step - 1));
				}

				// Otherwise: Change velocity
				else
				{
					float v = pd.velocity * (1.0f + 0.01f * wheelEvent->getDelta().y);
					v = LIMIT (v, 0.0f, 2.0f);
					do
					{
						Pad pds = ui->pattern.getPad (row, step);
						pds.velocity = v;
						ui->pattern.setPad (row, step, pds);
						ui->drawPad (row, step);
						ui->send_pad (row, step);
						++step;
					} while (ui->pattern.padHasSuccessor (row, step - 1));
				}

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

		int row = (ROWS - 1) - ((int) ((focusEvent->getPosition ().y - widget->getYOffset()) / (height / ROWS)));
		int step = (focusEvent->getPosition ().x - widget->getXOffset()) / (width / ui->controllerWidgets[NR_OF_STEPS]->getValue ());
		while (ui->pattern.padHasAntecessor (row, step)) --step;

		if ((row >= 0) && (row < ROWS) && (step >= 0) && (step < ((int)ui->controllerWidgets[NR_OF_STEPS]->getValue ())))
		{
			Pad pd = ui->pattern.getPad (row, step);
			double dm = fmod (pd.duration, 1.0);
			if (dm == 0.0) dm = 1.0;
			double rd = LIMIT (pd.randDuration, - dm, 0.0);
			double pdr = pd.duration * rd / dm;
			ui->padSurface.focusText.setText
			(
				"Channel: " + std::to_string (int (pd.ch) & 0x0f) + "\n" +
				"Gate: " + BUtilities::to_string (pd.randGate, "%1.2f") + "\n" +
				"Note pitch: " + std::to_string (int (pd.pitchNote)) + " ± " + std::to_string (int (abs (pd.randNote))) + "\n" +
				"Octave pitch: " + std::to_string (int (pd.pitchOctave)) + " ± " + std::to_string (int (abs (pd.randOctave))) + "\n" +
				"Velocity: " + BUtilities::to_string (pd.velocity, "%1.2f") + " ± " + BUtilities::to_string (abs (pd.randVelocity), "%1.2f") + "\n" +
				"Duration: " + BUtilities::to_string (pd.duration, "%1.2f") + " (" + BUtilities::to_string (pdr, "%1.2f") + ")");
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
		for (int step = 0; step < ((int)controllerWidgets[NR_OF_STEPS]->getValue ()); ++step)
		{
			drawPad (cr, row, step);
			while (pattern.padHasSuccessor (row, step)) ++step;
		}
	}
	cairo_destroy (cr);
	padSurface.update();
}

void BSEQuencer_GUI::drawPad (const int row, const int step)
{
	cairo_surface_t* surface = padSurface.getDrawingSurface();
	cairo_t* cr = cairo_create (surface);
	drawPad (cr, row, step);
	cairo_destroy (cr);
	padSurface.update();
}

void BSEQuencer_GUI::drawPad (cairo_t* cr, const int row, const int step)
{
	int start = step;
	while (pattern.padHasAntecessor (row, start)) --start;

	Pad pd = pattern.getPad (row, start);

	if ((!cr) || (cairo_status (cr) != CAIRO_STATUS_SUCCESS) || (row < 0) || (row >= ROWS) || (start < 0) ||
		(start >= (int (controllerWidgets[NR_OF_STEPS]->getValue ())))) return;

	// Get size of drawing area
	const double width = padSurface.getEffectiveWidth ();
	const double height = padSurface.getEffectiveHeight ();
	const double w = width / controllerWidgets[NR_OF_STEPS]->getValue ();
	const double h = height / ROWS;
	const double x = start * w;
	const double y = (ROWS - row - 1) * h;
	const double xr = round (x);
	const double yr = round (y);
	int ps = pattern.padGetSize (row, start);
	if (start + ps > int (controllerWidgets[NR_OF_STEPS]->getValue ())) ps = int (controllerWidgets[NR_OF_STEPS]->getValue ()) - start;
	const double wr = round (x + w * ps) - xr;
	const double hr = round (y + h) - yr;


	// Odd or even?
	BColors::Color bg = ((int (start / controllerWidgets[STEPS_PER]->getValue ())) % 2) ? oddPadBgColor : evenPadBgColor;

	// Highlight selection
	int clipRMin = clipBoard.origin.first;
	int clipRMax = clipBoard.origin.first + clipBoard.extends.first;
	if (clipRMin > clipRMax) std::swap (clipRMin, clipRMax);
	int clipSMin = clipBoard.origin.second;
	int clipSMax = clipBoard.origin.second + clipBoard.extends.second;
	if (clipSMin > clipSMax) std::swap (clipSMin, clipSMax);

	// Draw backgroung
	int i = 0;
	do
	{
		if ((start + i >= 0) && (start + i < int (controllerWidgets[NR_OF_STEPS]->getValue ())))
		{
			BColors::Color bgi = bg;
			double xi = round (x + i * w);
			double wi = round (x + (i + 1) * w) - xi;
			if ((!clipBoard.ready) && (row >= clipRMin) && (row <= clipRMax) && (start + i >= clipSMin) && (start + i <= clipSMax)) bgi.applyBrightness (1.5);
			cairo_set_source_rgba (cr, CAIRO_RGBA (bgi));
			cairo_set_line_width (cr, 0.0);
			cairo_rectangle (cr, xi, yr, wi, hr);
			cairo_fill (cr);
		}
		++i;
	} while (pattern.padHasSuccessor (row, start + i - 1));

	// Draw pad
	int ch = padGetChannel (row, start);
	int ctrl = padGetControl (row, start);
	double vel = (pd.velocity <= 1 ?  pd.velocity - 1 : (pd.velocity - 1) * 0.5);
	int oct = pd.pitchOctave;

	if ((ch >= 0) && (ch <= NR_SEQUENCER_CHS) && (ctrl >= 0) && (ctrl < NR_CTRL_BUTTONS))
	{
		BColors::Color color = chButtonStyles[ch].color;
		if (ch > 0) color.applyBrightness(vel);
		int i = 0;
		do
		{
			if (cursorBits[start + i] & (1 << row))
			{
				color.setAlpha (1.0);
				color.applyBrightness (0.75);
				break;
			}
			++i;
		}
		while (pattern.padHasSuccessor (row, start + i - 1));

		int symbol = ctrlButtonStyles[ctrl].symbol;

		drawButton (cr, xr + 1, yr + 1, wr - 2, hr - 2, color, symbol, (pd.duration == 0 ? 1 : pd.duration / ceil (pd.duration)));

		// Displays pitch octave
		if ((pd.duration > 0.0) && (pd.velocity > 0.0))
		{
			cairo_surface_t* surface = padSurface.getDrawingSurface();
			cairo_t* cr = cairo_create (surface);
			double h = 0.75 * lfLabelFont.getFontSize ();

			if (pd.pitchNote > 0)
			{
				cairo_move_to (cr, xr + 1.0 + 3.0 * sz, yr + 1.0 + 2.0 * sz + h);
				cairo_line_to (cr, xr + 1.0 + 3.0 * sz, yr + 1.0 + 2.0 * sz + (1.0 - pd.pitchNote / 16.0) * h);
				cairo_line_to (cr, xr + 1.0 + 2.0 * sz, yr + 1.0 + 3.0 * sz + (1.0 - pd.pitchNote / 16.0) * h);
				cairo_move_to (cr, xr + 1.0 + 3.0 * sz, yr + 1.0 + 2.0 * sz + (1.0 - pd.pitchNote / 16.0) * h);
				cairo_line_to (cr, xr + 1.0 + 4.0 * sz, yr + 1.0 + 3.0 * sz + (1.0 - pd.pitchNote / 16.0) * h);
				cairo_set_line_width (cr, 1.0);
				if ((ch == 4) && (pd.velocity > 0.5)) cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::black));
				else cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::white));
				cairo_stroke (cr);
			}

			if (pd.pitchNote < 0)
			{
				cairo_move_to (cr, xr + 1.0 + 3.0 * sz, yr + 1.0 + 2.0 * sz);
				cairo_line_to (cr, xr + 1.0 + 3.0 * sz, yr + 1.0 + 2.0 * sz + (-pd.pitchNote / 16.0) * h);
				cairo_line_to (cr, xr + 1.0 + 2.0 * sz, yr + 1.0 + 1.0 * sz + (-pd.pitchNote / 16.0) * h);
				cairo_move_to (cr, xr + 1.0 + 3.0 * sz, yr + 1.0 + 2.0 * sz + (-pd.pitchNote / 16.0) * h);
				cairo_line_to (cr, xr + 1.0 + 4.0 * sz, yr + 1.0 + 1.0 * sz + (-pd.pitchNote / 16.0) * h);
				cairo_set_line_width (cr, 1.0);
				if ((ch == 4) && (pd.velocity > 0.5)) cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::black));
				else cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::white));
				cairo_stroke (cr);
			}

			if (oct != 0)
			{
				std::string valstr = (oct <= 0 ? "" : "+") + std::to_string (oct);
				if ((ch == 4) && (pd.velocity > 0.5)) cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::black));
				else cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::white));
				cairo_select_font_face (cr, lfLabelFont.getFontFamily ().c_str (), lfLabelFont.getFontSlant (), lfLabelFont.getFontWeight ());
				cairo_set_font_size (cr, h);
				cairo_move_to (cr, xr + 1.0 + 6.0 * sz, yr + 1.0 + 2.0 * sz + h);
				cairo_show_text (cr, valstr.c_str ());
			}

			cairo_destroy (cr);
		}
	}
}

bool BSEQuencer_GUI::padIsSelected (const int row, const int step)
{
	int s = step;
	while (pattern.padHasAntecessor (row, step)) --s;

	int clipRMin = clipBoard.origin.first;
	int clipRMax = clipBoard.origin.first + clipBoard.extends.first;
	if (clipRMin > clipRMax) std::swap (clipRMin, clipRMax);
	int clipSMin = clipBoard.origin.second;
	int clipSMax = clipBoard.origin.second + clipBoard.extends.second;
	if (clipSMin > clipSMax) std::swap (clipSMin, clipSMax);
	if (clipBoard.ready || (row < clipRMin) || (row > clipRMax)) return false;

	do
	{
		if ((s >= clipSMin) && (s <= clipSMax)) return true;
		++s;
	} while (pattern.padHasSuccessor (row, s - 1));

	return false;
}

int BSEQuencer_GUI::padGetChannel (const int row, const int step)
{
	int start = step;
	while (pattern.padHasAntecessor (row, start)) --start;
	return int (pattern.getPad (row, start).ch) & 0x0F;
}

int BSEQuencer_GUI::padGetControl (const int row, const int step)
{
	int start = step;
	while (pattern.padHasAntecessor (row, start)) --start;
	return (int (pattern.getPad (row, start).ch) & 0xF0) / 0x10;
}

void BSEQuencer_GUI::padClip (const int row, const int step)
{
	if (pattern.padHasAntecessor (row, step))
	{
		int start = step - 1;
		while (pattern.padHasAntecessor (row, start)) --start;
		for (int s = start; s < step; ++s)
		{
			Pad pd = pattern.getPad (row, s);
			pd.duration = step - s;
			pattern.setPad (row, s, pd);
			send_pad (row, s);
		}
		drawPad (row, start);
		drawPad (row, step);
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
	PuglNativeView parentWindow = 0;
	LV2UI_Resize* resize = NULL;

	if (strcmp(plugin_uri, BSEQUENCER_URI) != 0)
	{
		std::cerr << "BSEQuencer.lv2#GUI: GUI does not support plugin with URI " << plugin_uri << std::endl;
		return NULL;
	}

	for (int i = 0; features[i]; ++i)
	{
		if (!strcmp(features[i]->URI, LV2_UI__parent)) parentWindow = (PuglNativeView) features[i]->data;
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
	if ((screenWidth < 870) || (screenHeight < 580)) sz = 0.5;
	else if ((screenWidth < 1290) || (screenHeight < 860)) sz = 0.66;

	/*
	std::cerr << "B.SEQuencer_GUI.lv2 screen size " << screenWidth << " x " << screenHeight <<
			". Set GUI size to " << 1250 * sz << " x " << 820 * sz << ".\n";
	*/

	if (resize) resize->ui_resize(resize->handle, 1250 * sz, 820 * sz);

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

static int call_resize (LV2UI_Handle ui, int width, int height)
{
	BSEQuencer_GUI* self = (BSEQuencer_GUI*) ui;
	BEvents::ExposeEvent* ev = new BEvents::ExposeEvent (self, self, BEvents::CONFIGURE_REQUEST_EVENT, self->getPosition().x, self->getPosition().y, width, height);
	self->addEventToQueue (ev);
	return 0;
}

static const LV2UI_Idle_Interface idle = {.idle = call_idle };
static const LV2UI_Resize resize = {.ui_resize = call_resize} ;

static const void* extension_data(const char* uri)
{
	if (!strcmp(uri, LV2_UI__idleInterface)) return &idle;
	else if(!strcmp(uri, LV2_UI__resize)) return &resize;
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
