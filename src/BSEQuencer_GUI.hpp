#ifndef BSEQUENCER_GUI_HPP_
#define BSEQUENCER_GUI_HPP_

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/time/time.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <iostream>

#include "BScale.hpp"
#include "BWidgets/BWidgets.hpp"
#include "drawbutton.hpp"
#include "PlayStopButton.hpp"
#include "ButtonBox.hpp"
#include "CircledSymbol.hpp"
#include "definitions.h"
#include "ports.h"
#include "urids.h"
#include "Pad.hpp"
#include "PadMessage.hpp"

#define BG_FILE "surface.png"
#define HELP_URL "https://github.com/sjaehn/BSEQuencer/wiki/B.SEQuencer"
#define OPEN_CMD "xdg-open"
class BSEQuencer_GUI : public BWidgets::Window
{
public:
	BSEQuencer_GUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeWindow parentWindow);
	~BSEQuencer_GUI ();
	void port_event (uint32_t port_index, uint32_t buffer_size, uint32_t format, const void *buffer);
	void send_ui_on ();
	void send_ui_off ();
	void send_pad (int row, int step);

	LV2UI_Controller controller;
	LV2UI_Write_Function write_function;

private:
	static void valueChangedCallback(BEvents::Event* event);
	static void helpPressedCallback (BEvents::Event* event);
	static void padsPressedCallback (BEvents::Event* event);
	void drawCaption ();
	void drawPad ();
	void drawPad (int row, int step);
	void drawPad (cairo_t* cr, int row, int step);

	std::string pluginPath;

	BSEQuencerURIs uris;
	LV2_URID_Map* map;
	LV2_Atom_Forge forge;

	// Controllers
	std::array<BWidgets::ValueWidget*, KNOBS_SIZE> controllerWidgets;
	std::array<float, KNOBS_SIZE> controllers;

	//Pads
	Pad pads [ROWS] [MAXSTEPS];

	// Cursors
	uint32_t cursorBits;
	uint32_t noteBits;
	uint32_t chBits;

	// Temporary tools
	double tempToolCh;
	bool tempTool;

	std::array<BScaleNotes, 14> scaleNotes	=
				{{{CROMATICSCALE}, {MAJORSCALE}, {MINORSCALE}, {HARMONICMAJORSCALE}, {HARMONICMINORSCALE}, {MELODICMINORSCALE},
				  {DORIANSCALE}, {PHRYGIANSCALE}, {LYDIANSCALE}, {MIXOLYDIANSCALE}, {LOKRIANSCALE}, {HUNGARIANMINORSCALE},
				  {MAJORPENTATONICSCALE}, {MINORPENTATONICSCALE}}};
	std::vector<BItems::Item> scaleItems =
				{{0, "Chromatic"}, {1, "Major"}, {2, "Minor"}, {3, "Harmonic major"}, {4, "Harmonic minor"}, {5, "Melodic minor"},
				 {6, "Dorian"}, {7, "Phygian"}, {8, "Lydian"}, {9, "Mixolydian"}, {10, "Lokrian"}, {11, "Hungarian minor"},
				 {12, "Major pentatonic"}, {13, "Minor pentatonic"}};


	//Widgets
	BWidgets::Widget mContainer;
	BWidgets::DrawingSurface padSurface;
	BWidgets::DrawingSurface captionSurface;

	BWidgets::Widget modeBox;
	BWidgets::Label modeBoxLabel;
	BWidgets::Label modeLabel;
	BWidgets::PopupListBox modeListBox;
	BWidgets::Label modeAutoplayBpmLabel;
	BWidgets::DisplayHSlider modeAutoplayBpmSlider;
	BWidgets::Label modeAutoplayBpbLabel;
	BWidgets::DisplayHSlider modeAutoplayBpbSlider;
	BWidgets::Label modeMidiInChannelLabel;
	BWidgets::PopupListBox modeMidiInChannelListBox;
	BWidgets::Label modePlayLabel;
	PlayStopButton modePlayButton;

	BWidgets::Widget toolBox;
	BWidgets::Label toolBoxLabel;
	ButtonBox toolButtonBox;
	BWidgets::Label toolButtonBoxCtrlLabel;
	BWidgets::Label toolButtonBoxChLabel;
	BWidgets::Label toolOctaveLabel;
	BWidgets::DisplayDial toolOctaveDial;
	BWidgets::Label toolVelocityLabel;
	BWidgets::DisplayDial toolVelocityDial;
	BWidgets::Label toolDurationLabel;
	BWidgets::DisplayDial toolDurationDial;

	BWidgets::Widget propertiesBox;
	BWidgets::Label propertiesBoxLabel;
	BWidgets::Label propertiesNrStepsLabel;
	BWidgets::PopupListBox propertiesNrStepsListBox;
	BWidgets::Label propertiesStepsPerLabel;
	BWidgets::DisplayHSlider propertiesStepsPerSlider;
	BWidgets::PopupListBox propertiesBaseListBox;
	BWidgets::Label propertiesRootLabel;
	BWidgets::PopupListBox propertiesRootListBox;
	BWidgets::PopupListBox propertiesSignatureListBox;
	BWidgets::Label propertiesOctaveLabel;
	BWidgets::PopupListBox propertiesOctaveListBox;
	BWidgets::Label propertiesScaleLabel;
	BWidgets::PopupListBox propertiesScaleListBox;

	typedef struct {
		BWidgets::Widget box;
		BWidgets::Label chLabel;
		BWidgets::DrawingSurface chSymbol;
		BWidgets::Label pitchLabel;
		BWidgets::HSwitch pitchSwitch;
		BWidgets::Widget pitchScreen;
		BWidgets::Label channelLabel;
		BWidgets::PopupListBox channelListBox;
		BWidgets::Label velocityLabel;
		BWidgets::DisplayDial velocityDial;
		BWidgets::Label noteOffsetLabel;
		BWidgets::DisplayDial noteOffsetDial;
	} ChBox;

	std::array<ChBox, NR_SEQUENCER_CHS> chBoxes;

	CircledSymbol helpLabel;

	// Definition of styles
	BColors::ColorSet fgColors = {{{0.0, 0.25, 0.75, 1.0}, {0.25, 0.75, 0.75, 1.0}, {0.0, 0.0, 0.1, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet fgColors_ch1 = {{{0.0, 0.25, 0.75, 1.0}, {0.25, 0.75, 0.75, 1.0}, {0.0, 0.0, 0.1, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet fgColors_ch2 = {{{0.75, 0.0, 0.75, 1.0}, {0.75, 0.25, 0.75, 1.0}, {0.1, 0.0, 0.1, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet fgColors_ch3 = {{{0.875, 0.4235, 0.0, 1.0}, {0.875, 0.75, 0.25, 1.0}, {0.1, 0.05, 0.0, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet fgColors_ch4 = {{{0.75, 0.75, 0.0, 1.0}, {0.75, 0.75, 0.75, 1.0}, {0.1, 0.1, 0.0, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet txColors = {{{0.167, 0.37, 0.80, 1.0}, {0.33, 0.5, 0.85, 1.0}, {0.0, 0.0, 0.25, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet bgColors = {{{0.15, 0.15, 0.15, 1.0}, {0.3, 0.3, 0.3, 1.0}, {0.05, 0.05, 0.05, 1.0}, {0.0, 0.0, 0.0, 1.0}}};
	BColors::ColorSet ltColors = {{{1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.25, 0.25, 0.25, 1.0}, {0.0, 0.0, 0.0, 1.0}}};
	BColors::Color ink = {0.0, 0.25, 0.5, 1.0};
	BColors::Color light = {1.0, 1.0, 1.0, 1.0};
	BColors::Color ctrlBgColor = {0.0, 0.08, 0.04, 1.0};
	BColors::Color evenPadBgColor = {0.0, 0.03, 0.06, 1.0};
	BColors::Color oddPadBgColor = {0.0, 0.0, 0.0, 1.0};
	std::array<ButtonStyle, NR_SEQUENCER_CHS + 1> chButtonStyles =
		{{{{0.0, 0.0, 0.0, 0.5}, ""}, {{0.0, 0.0, 1.0, 1.0}, ""}, {{1.0, 0.0, 1.0, 1.0}, ""}, {{1.0, 0.5, 0.0, 1.0}, ""}, {{1.0, 1.0, 0.0, 1.0}, ""}}};
	std::array<ButtonStyle, NR_CTRL_BUTTONS> ctrlButtonStyles =
		{{{{0.0, 0.0, 0.0, 0.5}, ""}, {{0.0, 1.0, 0.0, 1.0}, ""}, {{0.0, 1.0, 0.0, 1.0}, "▶"}, {{0.0, 1.0, 0.0, 1.0}, "◀"},
				{{0.0, 1.0, 0.0, 1.0}, "●●"}, {{0.0, 1.0, 0.0, 1.0}, "●"}, {{0.0, 1.0, 0.0, 1.0}, "▶▶●"},  {{0.0, 1.0, 0.0, 1.0}, "●◀◀"},
				{{1.0, 0.0, 0.0, 1.0}, "✖"}, {{1.0, 0.0, 0.0, 1.0}, "◼"}}};
	BStyles::Border border = {{ink, 1.0}, 0.0, 2.0, 0.0};
	BStyles::Fill widgetBg = BStyles::noFill;
	BStyles::Fill screenBg = BStyles::Fill (BColors::Color (0.0, 0.0, 0.0, 0.8));
	BStyles::Fill boxBg = BStyles::Fill (BColors::Color (0.0, 0.0, 0.0, 0.9));
	BStyles::Font ctLabelFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
											   BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
	BStyles::Font iLabelFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 24.0,
											   BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
	BStyles::Font lfLabelFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
											   BStyles::TEXT_ALIGN_LEFT, BStyles::TEXT_VALIGN_MIDDLE);
	BStyles::StyleSet defaultStyles = {"default", {{"background", STYLEPTR (&BStyles::noFill)},
												   {"border", STYLEPTR (&BStyles::noBorder)}}};
	BStyles::StyleSet labelStyles = {"labels", 	  {{"background", STYLEPTR (&BStyles::noFill)},
												   {"border", STYLEPTR (&BStyles::noBorder)},
												   {"textcolors", STYLEPTR (&txColors)},
												   {"font", STYLEPTR (&ctLabelFont)}}};

	BStyles::Theme theme = BStyles::Theme ({
		defaultStyles,
		{"B.SEQuencer", 	{{"background", STYLEPTR (&BStyles::blackFill)},
							 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"main", 			{{"background", STYLEPTR (&widgetBg)},
							 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"widget", 			{{"uses", STYLEPTR (&defaultStyles)}}},
		{"screen", 			{{"background", STYLEPTR (&screenBg)}}},
		{"box", 			{{"background", STYLEPTR (&boxBg)},
							 {"border", STYLEPTR (&border)}}},
		{"button", 			{{"background", STYLEPTR (&BStyles::blackFill)},
							 {"border", STYLEPTR (&border)}}},
		{"dial", 			{{"uses", STYLEPTR (&defaultStyles)},
							 {"fgcolors", STYLEPTR (&fgColors)},
							 {"bgcolors", STYLEPTR (&bgColors)},
							 {"textcolors", STYLEPTR (&fgColors)},
							 {"font", STYLEPTR (&ctLabelFont)}}},
		{"ch1", 			{{"uses", STYLEPTR (&defaultStyles)},
							 {"fgcolors", STYLEPTR (&fgColors_ch1)},
							 {"bgcolors", STYLEPTR (&bgColors)},
							 {"textcolors", STYLEPTR (&fgColors)},
							 {"font", STYLEPTR (&ctLabelFont)}}},
		{"ch2", 			{{"uses", STYLEPTR (&defaultStyles)},
							 {"fgcolors", STYLEPTR (&fgColors_ch2)},
							 {"bgcolors", STYLEPTR (&bgColors)},
							 {"textcolors", STYLEPTR (&fgColors)},
							 {"font", STYLEPTR (&ctLabelFont)}}},
		{"ch3", 			{{"uses", STYLEPTR (&defaultStyles)},
							 {"fgcolors", STYLEPTR (&fgColors_ch3)},
							 {"bgcolors", STYLEPTR (&bgColors)},
							 {"textcolors", STYLEPTR (&fgColors)},
							 {"font", STYLEPTR (&ctLabelFont)}}},
		{"ch4", 			{{"uses", STYLEPTR (&defaultStyles)},
							 {"fgcolors", STYLEPTR (&fgColors_ch4)},
							 {"bgcolors", STYLEPTR (&bgColors)},
							 {"textcolors", STYLEPTR (&fgColors)},
							 {"font", STYLEPTR (&ctLabelFont)}}},
		{"slider",			{{"uses", STYLEPTR (&defaultStyles)},
							 {"fgcolors", STYLEPTR (&fgColors)},
							 {"bgcolors", STYLEPTR (&bgColors)},
							 {"textcolors", STYLEPTR (&fgColors)},
							 {"font", STYLEPTR (&ctLabelFont)}}},
		{"ctlabel",	 		{{"uses", STYLEPTR (&labelStyles)}}},
		{"lflabel",	 		{{"uses", STYLEPTR (&labelStyles)},
							 {"font", STYLEPTR (&lfLabelFont)}}},
		{"ilabel",	 		{{"uses", STYLEPTR (&labelStyles)},
							 {"font", STYLEPTR (&iLabelFont)}}}
	});

};

#endif /* BSEQUENCER_GUI_HPP_ */
