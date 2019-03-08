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

#ifndef SCALEEDITOR_HPP_
#define SCALEEDITOR_HPP_

#include "definitions.h"
#include "BScale.hpp"
#include "BWidgets/BItems.hpp"
#include "BWidgets/Widget.hpp"
#include "BWidgets/ValueWidget.hpp"
#include "BWidgets/HPianoRoll.hpp"
#include "BWidgets/Label.hpp"
#include "BWidgets/PopupListBox.hpp"
#include "BWidgets/TextButton.hpp"
#include "CircledSymbol.hpp"

class ScaleEditor : public BWidgets::ValueWidget
{
public:
	ScaleEditor ();
	ScaleEditor (const double x, const double y, const double width, const double height, const std::string& name, const std::string& pluginPath,
				 const int mapNr, const ScaleMap& scaleMap, const BScale& scale);

	virtual void resize (double width, double height) override;
	void setScaleMap (const ScaleMap& scaleMap);
	ScaleMap getScaleMap () const;
	void setScale (const BScale& scale);
	BScale getScale () const;
	void setMapNr (const int nr);
	int getMapNr () const;

protected:
	void szScaleEditor ();
	void updateAltSymbol (int nr);
	static void symbolDragCallback (BEvents::Event* event);
	static void listboxValueChangedCallback (BEvents::Event* event);
	static void buttonClickCallback (BEvents::Event* event);
	static void closeClickCallback (BEvents::Event* event);
	static void pianoClickCallback (BEvents::Event* event);

	BWidgets::Label nameLabel;
	BWidgets::Label scaleNameLabel;
	BWidgets::Label rowLabel;
	BWidgets::Label symbolLabel;
	BWidgets::Label noteLabel;
	BWidgets::Label altSymbolLabel;
	std::array<BWidgets::Label, ROWS> nrLabel;
	std::array<BWidgets::Widget, ROWS> nrSymbol;
	std::array<BWidgets::PopupListBox, ROWS> nrListbox;
	std::array<BWidgets::Label, ROWS> nrNoteLabel;
	std::array<BWidgets::Label, ROWS> nrAltSymbolLabel;
	BWidgets::Widget drumkitSymbol;
	BWidgets::Widget scaleSymbol;
	BWidgets::Widget dragSymbol;
	BWidgets::HPianoRoll piano;
	BWidgets::TextButton cancelButton;
	BWidgets::TextButton applyButton;
	CircledSymbol closeSymbol;

	int pianoRoot;
	BScale scale;
	ScaleMap scaleMap;
	int mapNr;

	std::vector<BItems::Item> noteSymbols;

	double sz;

	cairo_surface_t* bgSurface;
	cairo_surface_t* drumSurface;
	cairo_surface_t* noteSurface;

	BColors::ColorSet txColors = {{{0.167, 0.37, 0.80, 1.0}, {0.33, 0.5, 0.85, 1.0}, {0.0, 0.0, 0.25, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BStyles::Border border = {{BColors::white, 1.0}, 0.0, 2.0, 0.0};
	BStyles::Fill scaleEditorBg = BStyles::blackFill;
	BStyles::Fill drumBg = BStyles::noFill;
	BStyles::Fill noteBg = BStyles::noFill;
	BStyles::Font ctLabelFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
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
		{"scaleeditor", 	{{"background", STYLEPTR (&scaleEditorBg)},
							 {"border", STYLEPTR (&border)}}},
		{"widget", 			{{"uses", STYLEPTR (&defaultStyles)}}},
		{"drum", 			{{"background", STYLEPTR (&drumBg)},
							 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"note", 			{{"background", STYLEPTR (&noteBg)},
							 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"ctlabel",	 		{{"uses", STYLEPTR (&labelStyles)},
							 {"font", STYLEPTR (&ctLabelFont)}}},
		{"lflabel",	 		{{"uses", STYLEPTR (&labelStyles)},
							 {"font", STYLEPTR (&lfLabelFont)}}},
		{"button", 			{{"font", STYLEPTR (&ctLabelFont)}}},
		{"xsymbol",	 		{{"uses", STYLEPTR (&defaultStyles)},
							 {"textcolors", STYLEPTR (&BColors::whites)},
							 {"font", STYLEPTR (&ctLabelFont)}}},
		{"menu",	 		{{"border", STYLEPTR (&BStyles::greyBorder1pt)},
							 {"background", STYLEPTR (&BStyles::grey20Fill)}}},
		{"menu/item",	 	{{"uses", STYLEPTR (&defaultStyles)},
							 {"textcolors", STYLEPTR (&BColors::whites)},
							 {"font", STYLEPTR (&lfLabelFont)}}},
		{"menu/button",	 	{{"border", STYLEPTR (&BStyles::greyBorder1pt)},
							 {"background", STYLEPTR (&BStyles::grey20Fill)},
							 {"bgcolors", STYLEPTR (&BColors::darks)}}},
		{"menu/listbox",	{{"border", STYLEPTR (&BStyles::greyBorder1pt)},
							 {"background", STYLEPTR (&BStyles::grey20Fill)}}},
		{"menu/listbox/item",	{{"uses", STYLEPTR (&defaultStyles)},
							 {"textcolors", STYLEPTR (&BColors::whites)},
							 {"font", STYLEPTR (&lfLabelFont)}}},
		{"menu/listbox//button",	{{"border", STYLEPTR (&BStyles::greyBorder1pt)},
							 {"background", STYLEPTR (&BStyles::grey20Fill)},
							 {"bgcolors", STYLEPTR (&BColors::darks)}}}
	});

};

ScaleEditor::ScaleEditor() : ScaleEditor (0, 0, 0, 0, "editor", "", 0, ScaleMap (), BScale (0, defaultScale)) {}

ScaleEditor::ScaleEditor(const double x, const double y, const double width, const double height, const std::string& name,
						 const std::string& pluginPath, const int mapNr, const ScaleMap& scaleMap, const BScale& scale) :
		BWidgets::ValueWidget (x, y, width, height, name, 0.0),
		scaleMap (scaleMap), scale (scale), mapNr (mapNr),
		nameLabel (360, 60, 80, 20, "lflabel", "Scale name:"),
		scaleNameLabel (460, 60, 320, 20, "lflabel", std::string (scaleMap.name)),
		rowLabel (20, 60, 80, 20, "lflabel", "Row"),
		symbolLabel (60, 60, 48, 20, "ctlabel", "Mode"),
		noteLabel (128, 60, 80, 20, "ctlabel", "Note"),
		altSymbolLabel (228, 60, 80, 20, "ctlabel", "Symbol"),
		drumkitSymbol (400, 120, 48, 24, "drum"),
		scaleSymbol (460, 120, 48, 24, "note"),
		dragSymbol (460, 120, 48, 24, "note"),
		cancelButton (320, 600, 60, 20, "button", "Cancel"),
		applyButton (420, 600, 60, 20, "button", "Apply"),
		piano (340, 440, 440, 120, "widget", 0, 35), pianoRoot (0),
		closeSymbol (770, 10, 20, 20, "xsymbol", "✕"),
		sz (width / 800 < height / 640 ? width / 800 : height / 640)


{
	setDraggable (true);
	setFocusable (true);	// Only to block underlying pads callback

	bgSurface = cairo_image_surface_create_from_png ((pluginPath + "ScaleEditor.png").c_str());
	drumSurface = cairo_image_surface_create_from_png ((pluginPath + "DrumSymbol.png").c_str());
	noteSurface = cairo_image_surface_create_from_png ((pluginPath + "NoteSymbol.png").c_str());
	scaleEditorBg.loadFillFromCairoSurface(bgSurface);
	drumBg.loadFillFromCairoSurface(drumSurface);
	noteBg.loadFillFromCairoSurface(noteSurface);

	// Create note symbols (for nrListbox)
	BScale sc = BScale(0, {CROMATICSCALE});
	BItems::Item item;
	for (int i = 0; i < 120; ++i)
	{
		char chrNote[8];
		sc.getSymbol(chrNote, i % 12);
		int octave = (i / 12) - 1;
		std::string strNote = std::to_string(i) + " (" + std::string (chrNote) + (octave != 0 ? std::to_string(octave) : "") + ")";
		item.value = i;
		item.string = strNote;
		noteSymbols.push_back (item);
	}

	// Init nr widgets
	for (int i = 0; i < ROWS; ++i)
	{
		nrLabel[i] = BWidgets::Label (20, 540 - i * 30, 30, 24, "lflabel", std::to_string (i + 1));
		nrLabel[i].rename ("lflabel");
		nrLabel[i].applyTheme (theme);
		add (nrLabel[i]);

		nrSymbol[i] = BWidgets::Widget (60, 540 - i * 30, 48, 24, "widget");
		nrSymbol[i].applyTheme (theme);
		add (nrSymbol[i]);

		if (i >= 6) nrListbox[i] = BWidgets::PopupListBox (128, 542 - i * 30, 80, 20, 80, 240, "menu", noteSymbols, 0);
		else nrListbox[i] = BWidgets::PopupListBox (128, 542 - i * 30, 80, 20, 0, -240, 80, 240, "menu", noteSymbols, 0);
		nrListbox[i].setCallbackFunction(BEvents::VALUE_CHANGED_EVENT, listboxValueChangedCallback);
		nrListbox[i].rename ("menu");
		nrListbox[i].applyTheme (theme);
		add (nrListbox[i]);

		nrNoteLabel[i] = BWidgets::Label (128, 540 - i * 30, 80, 24, "ctlabel", "(uses scale)");
		nrNoteLabel[i].rename ("ctlabel");
		nrNoteLabel[i].applyTheme (theme);
		add (nrNoteLabel[i]);
		nrNoteLabel[i].hide ();

		nrAltSymbolLabel[i] = BWidgets::Label (228, 540 - i * 30, 80, 24, "ctlabel", "");
		nrAltSymbolLabel[i].rename ("ctlabel");
		updateAltSymbol (i);
		nrAltSymbolLabel[i].applyTheme (theme);
		add (nrAltSymbolLabel[i]);
	}

	drumkitSymbol.setDraggable (true);
	drumkitSymbol.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, symbolDragCallback);
	drumkitSymbol.setCallbackFunction(BEvents::BUTTON_RELEASE_EVENT, symbolDragCallback);
	drumkitSymbol.setCallbackFunction(BEvents::POINTER_DRAG_EVENT, symbolDragCallback);

	scaleSymbol.setDraggable (true);
	scaleSymbol.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, symbolDragCallback);
	scaleSymbol.setCallbackFunction(BEvents::BUTTON_RELEASE_EVENT, symbolDragCallback);
	scaleSymbol.setCallbackFunction(BEvents::POINTER_DRAG_EVENT, symbolDragCallback);

	cancelButton.setCallbackFunction(BEvents::BUTTON_CLICK_EVENT, buttonClickCallback);
	applyButton.setCallbackFunction(BEvents::BUTTON_CLICK_EVENT, buttonClickCallback);
	closeSymbol.setCallbackFunction(BEvents::BUTTON_CLICK_EVENT, closeClickCallback);

	piano.setKeysToggleable (true);
	piano.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, pianoClickCallback);
	piano.setCallbackFunction(BEvents::BUTTON_RELEASE_EVENT, pianoClickCallback);
	piano.setCallbackFunction(BEvents::POINTER_DRAG_EVENT, pianoClickCallback);

	setScale (scale);
	setScaleMap (scaleMap);

	nameLabel.applyTheme (theme);
	scaleNameLabel.applyTheme (theme);
	rowLabel.applyTheme (theme);
	symbolLabel.applyTheme (theme);
	noteLabel.applyTheme (theme);
	altSymbolLabel.applyTheme (theme);
	drumkitSymbol.applyTheme (theme);
	scaleSymbol.applyTheme (theme);
	cancelButton.applyTheme (theme);
	applyButton.applyTheme (theme);
	piano.applyTheme (theme);
	closeSymbol.applyTheme (theme);
	applyTheme (theme);

	add (nameLabel);
	add (scaleNameLabel);
	add (rowLabel);
	add (symbolLabel);
	add (noteLabel);
	add (altSymbolLabel);
	add (drumkitSymbol);
	add (scaleSymbol);
	add (cancelButton);
	add (applyButton);
	add (piano);
	add (closeSymbol);

	if (sz != 1.0) szScaleEditor();
}

void ScaleEditor::resize (double width, double height)
{
	double wf = width / 800;
	double hf = height / 640;
	double newsz = (wf < hf ? wf : hf);
	if (sz != newsz)
	{
		sz = newsz;
		szScaleEditor ();
	}
}

void ScaleEditor::szScaleEditor ()
{
	// Scale Fonts
	ctLabelFont.setFontSize (12 * sz);
	lfLabelFont.setFontSize (12 * sz);

	// Scale background
	cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 800 * sz, 640 * sz);
	cairo_t* cr = cairo_create (surface);
	cairo_scale (cr, sz, sz);
	cairo_set_source_surface(cr, bgSurface, 0, 0);
	cairo_paint(cr);
	scaleEditorBg.loadFillFromCairoSurface(surface);
	cairo_destroy (cr);
	cairo_surface_destroy (surface);

	// Scale drum image
	surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 48 * sz, 24 * sz);
	cr = cairo_create (surface);
	cairo_scale (cr, sz, sz);
	cairo_set_source_surface(cr, drumSurface, 0, 0);
	cairo_paint(cr);
	drumBg.loadFillFromCairoSurface(surface);
	cairo_destroy (cr);
	cairo_surface_destroy (surface);

	// Scale note image
	surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 48 * sz, 24 * sz);
	cr = cairo_create (surface);
	cairo_scale (cr, sz, sz);
	cairo_set_source_surface(cr, noteSurface, 0, 0);
	cairo_paint(cr);
	noteBg.loadFillFromCairoSurface(surface);
	cairo_destroy (cr);
	cairo_surface_destroy (surface);

	nameLabel.moveTo (360 * sz, 60 * sz); nameLabel.resize (80 * sz, 20 * sz);
	scaleNameLabel.moveTo (460 * sz, 60 * sz); scaleNameLabel.resize(320 * sz, 20 * sz);
	rowLabel.moveTo (20 * sz, 60 * sz); rowLabel.resize (80 * sz, 20 * sz);
	symbolLabel.moveTo (60 * sz, 60 * sz); symbolLabel.resize (48 * sz, 20 * sz);
	noteLabel.moveTo (128 * sz, 60 * sz); noteLabel.resize (80 * sz, 20 * sz);
	altSymbolLabel.moveTo (228 * sz, 60 * sz); altSymbolLabel.resize (80 * sz, 20 * sz);
	drumkitSymbol.moveTo (400 * sz, 120 * sz); drumkitSymbol.resize (48 * sz, 24 * sz);
	scaleSymbol.moveTo (460 * sz, 120 * sz); scaleSymbol.resize (48 * sz, 24 * sz);
	dragSymbol.moveTo (460 * sz, 120 * sz); dragSymbol.resize (48 * sz, 24 * sz);
	cancelButton.moveTo (320 * sz, 600 * sz); cancelButton.resize (60 * sz, 20 * sz);
	applyButton.moveTo (420 * sz, 600 * sz); applyButton.resize (60 * sz, 20 * sz);
	piano.moveTo (340 * sz, 440 * sz); piano.resize (440 * sz, 120 * sz);
	closeSymbol.moveTo (770 * sz, 10 * sz); closeSymbol.resize (20 * sz, 20 * sz);

	for (int i = 0; i < ROWS; ++i)
	{
		nrLabel[i].moveTo (20 * sz, (540 - i * 30) * sz); nrLabel[i].resize (30 * sz, 24 * sz);
		nrSymbol[i].moveTo (60 * sz, (540 - i * 30) * sz); nrSymbol[i].resize (48 * sz, 24 * sz);

		nrListbox[i].moveTo (128 * sz, (542 - i * 30) * sz ); nrListbox[i].resize (80 * sz, 20 * sz);
		nrListbox[i].resizeListBox (80 * sz, 240 * sz);
		if (i < 6) nrListbox[i].moveListBox(0, -240 * sz);

		nrNoteLabel[i].moveTo (128 * sz, (540 - i * 30) * sz); nrNoteLabel[i].resize (80 * sz, 24 * sz);
		nrAltSymbolLabel[i].moveTo (228 * sz, (540 - i * 30) * sz); nrAltSymbolLabel[i].resize (80 * sz, 24 * sz);
	}

	nameLabel.applyTheme (theme);
	scaleNameLabel.applyTheme (theme);
	rowLabel.applyTheme (theme);
	symbolLabel.applyTheme (theme);
	noteLabel.applyTheme (theme);
	altSymbolLabel.applyTheme (theme);
	drumkitSymbol.applyTheme (theme);
	scaleSymbol.applyTheme (theme);
	dragSymbol.applyTheme (theme);
	cancelButton.applyTheme (theme);
	applyButton.applyTheme (theme);
	piano.applyTheme (theme);
	closeSymbol.applyTheme (theme);

	for (int i = 0; i < ROWS; ++i)
	{
		nrLabel[i].applyTheme (theme);
		nrSymbol[i].applyTheme (theme);
		nrListbox[i].applyTheme (theme);
		nrNoteLabel[i].applyTheme (theme);
		nrAltSymbolLabel[i].applyTheme (theme);
	}

	applyTheme (theme);
	Widget::resize (800 * sz, 640 * sz);
}

void ScaleEditor::setScaleMap (const ScaleMap& scaleMap)
{
	this->scaleMap = scaleMap;

	scaleNameLabel.setText(std::string(scaleMap.name));

	// Set nr widgets
	for (int i = 0; i < ROWS; ++i)
	{
		if (scaleMap.elements[i] & 0x0100) nrSymbol[i].rename ("drum");
		else nrSymbol[i].rename ("note");
		nrSymbol[i].applyTheme (theme);

		if (scaleMap.elements[i] & 0x0100)
		{
			nrNoteLabel[i].hide ();
			nrListbox[i].setValue (scaleMap.elements[i] & 0x0FF);
			nrListbox[i].show ();
		}

		else
		{
			nrListbox[i].hide ();
			nrNoteLabel[i].show ();
		}

		updateAltSymbol (i);
	}
}

ScaleMap ScaleEditor::getScaleMap () const {return scaleMap;}

void ScaleEditor::setScale (const BScale& scale)
{
	this->scale = scale;
	for (int i = 0; i < ROWS; ++i) updateAltSymbol (i);

	// Set piano
	int pianoRoot = this->scale.getRoot() % 12;
	if (pianoRoot < 6) pianoRoot +=12;
	std::vector<bool> acKeys;
	std::vector<bool> prKeys;

	for (int i = 0; i < 36; ++i)
	{
		if ((i >= pianoRoot) && (i < pianoRoot + 12)) acKeys.push_back (true);
		else acKeys.push_back (false);

		if (this->scale.getElement (i + this->scale.getRoot()) != ENOTE) prKeys.push_back (true);
		else prKeys.push_back (false);
	}

	piano.activateKeys (acKeys);
	piano.pressKeys (prKeys);
}

BScale ScaleEditor::getScale () const {return scale;}

void ScaleEditor::setMapNr (const int nr) {mapNr = nr;}

int ScaleEditor::getMapNr () const {return mapNr;}

void ScaleEditor::updateAltSymbol (int nr)
{
	std::string symbol;
	if (scaleMap.altSymbols[nr][0] != '\0') symbol = std::string (scaleMap.altSymbols[nr]);
	else if (!(scaleMap.elements[nr] & 0x0100))
	{
		char csymbol[8];
		scale.getSymbol (csymbol, scaleMap.elements[nr]);
		symbol = std::string (csymbol);
	}
	nrAltSymbolLabel[nr].setText (symbol);
}

void ScaleEditor::symbolDragCallback (BEvents::Event* event)
{
	if ((!event) || (!event->getWidget())) return;

	BEvents::PointerEvent* pointerEvent = (BEvents::PointerEvent*) event;
	Widget* symbolWidget = pointerEvent->getWidget();
	if (symbolWidget->getParent())
	{
		ScaleEditor* scaleEditor = (ScaleEditor*)(symbolWidget->getParent());
		BEvents::EventType eventType = pointerEvent->getEventType();
		double sz = scaleEditor->sz;

		// Button pressed
		if (eventType == BEvents::BUTTON_PRESS_EVENT)
		{
			scaleEditor->dragSymbol = *symbolWidget;
			scaleEditor->dragSymbol.rename (symbolWidget->getName());
			scaleEditor->dragSymbol.applyTheme (scaleEditor->theme);
			if (scaleEditor->dragSymbol.getParent()) scaleEditor->dragSymbol.getParent()->release (&(scaleEditor->dragSymbol));
			scaleEditor->add (scaleEditor->dragSymbol);
			scaleEditor->dragSymbol.show();
		}

		// Button released
		else if (eventType == BEvents::BUTTON_RELEASE_EVENT)
		{
			// Get center position
			double x = scaleEditor->dragSymbol.getX() + 24 * sz;
			double y = scaleEditor->dragSymbol.getY() + 12 * sz;


			if ((x >= 60 * sz) && (x < 308 * sz) && (y >= 90 * sz) && (y < 560 * sz))
			{
				int nr = (565 * sz - y) / (30 * sz);
				nr = LIMIT (nr, 0, 15);

				// Drum kit
				if (scaleEditor->dragSymbol.getName() == "drum")
				{
					scaleEditor->scaleMap.elements[nr] = (((int)(scaleEditor->nrListbox[nr].getValue())) | 0x0100);
					strncpy (scaleEditor->scaleMap.altSymbols[nr], scaleEditor->nrListbox[nr].getItem().string.c_str(), 15);
					scaleEditor->nrNoteLabel[nr].hide ();
					scaleEditor->nrListbox[nr].show ();
					scaleEditor->nrSymbol[nr].rename ("drum");
					scaleEditor->nrSymbol[nr].applyTheme (scaleEditor->theme);
				}

				// Scale
				else
				{
					scaleEditor->scaleMap.elements[nr] = 0;	// Will be substituted by auto numbering later
					scaleEditor->nrListbox[nr].hide ();
					scaleEditor->nrNoteLabel[nr].show ();
					scaleEditor->nrSymbol[nr].rename ("note");
					scaleEditor->nrSymbol[nr].applyTheme (scaleEditor->theme);
				}

				// Update auto numbering of scale map elements
				for (int i = 0, count = 0; i < ROWS; ++i)
				{
					if (!(scaleEditor->scaleMap.elements[i] & 0x0100))
					{
						scaleEditor->scaleMap.elements[i] = count;
						scaleEditor->scaleMap.altSymbols[i][0] = '\0';
						++count;
					}
				}

			}

			if (scaleEditor->dragSymbol.getParent()) scaleEditor->dragSymbol.getParent()->release (&(scaleEditor->dragSymbol));
			for (int i = 0; i < ROWS; ++i) scaleEditor->updateAltSymbol (i);
		}

		// Pointer dragged
		else if (eventType == BEvents::POINTER_DRAG_EVENT)
		{
			double x = scaleEditor->dragSymbol.getX() + pointerEvent->getDeltaX();
			double y = scaleEditor->dragSymbol.getY() + pointerEvent->getDeltaY();
			scaleEditor->dragSymbol.moveTo (x, y);
		}
	}
}

void ScaleEditor::listboxValueChangedCallback (BEvents::Event* event)
{
	if ((!event) || (!event->getWidget()) || (!event->getWidget()->getParent())) return;

	BWidgets::PopupListBox* listbox = (BWidgets::PopupListBox*)(event->getWidget());
	ScaleEditor* scaleEditor = (ScaleEditor*)(listbox->getParent());
	int nr = -1;

	for (int i = 0; i < ROWS; ++i)
	{
		if (listbox == &(scaleEditor->nrListbox[i]))
		{
			nr = i;
			break;
		}
	}

	if ((nr >=0) && (scaleEditor->scaleMap.elements[nr] & 0x0100))
	{
		scaleEditor->scaleMap.elements[nr] = (((int)(scaleEditor->nrListbox[nr].getValue())) | 0x0100);
		strncpy (scaleEditor->scaleMap.altSymbols[nr], scaleEditor->nrListbox[nr].getItem().string.c_str(), 15);
		scaleEditor->updateAltSymbol (nr);
	}
}

void ScaleEditor::buttonClickCallback (BEvents::Event* event)
{
	if ((event) && (event->getEventType() == BEvents::BUTTON_CLICK_EVENT) && (event->getWidget()))
	{
		BWidgets::TextButton* button = (BWidgets::TextButton*) event->getWidget();
		if (button->getParent())
		{
			ScaleEditor* scaleEditor = (ScaleEditor*)(button->getParent());

			if (button == &(scaleEditor->cancelButton)) scaleEditor->setValue (-1.0);
			else if (button == &(scaleEditor->applyButton)) scaleEditor->setValue (1.0);
		}
	}
}

void ScaleEditor::closeClickCallback (BEvents::Event* event)
{
	if ((event) && (event->getEventType() == BEvents::BUTTON_CLICK_EVENT) && (event->getWidget()))
	{
		CircledSymbol* button = (CircledSymbol*) event->getWidget();
		if (button->getParent())
		{
			ScaleEditor* scaleEditor = (ScaleEditor*)(button->getParent());
			scaleEditor->setValue (-1.0);
		}
	}
}

void ScaleEditor::pianoClickCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget()))
	{
		BWidgets::HPianoRoll* piano = (BWidgets::HPianoRoll*)(event->getWidget());
		if (piano->getParent())
		{
			ScaleEditor* scaleEditor = (ScaleEditor*)(piano->getParent());
			std::vector<bool> pressedKeys = piano->getPressedKeys();
			BScale* scale = &(scaleEditor->scale);
			int pianoRoot = scale->getRoot() % 12;
			if (pianoRoot < 6) pianoRoot +=12;

			// Build new BScale elementarray from piano
			BScaleNotes newScaleNotes;
			newScaleNotes.fill (ENOTE);
			for (int i = 0, count = 0; i < 12; ++i)
			{
				if (pressedKeys[i + pianoRoot])
				{
					newScaleNotes[count] = i;
					++count;
				}
			}

			// Something changed?
			if (newScaleNotes != scale->getScale())
			{
				scale->setScale (newScaleNotes);
				for (int i = 0; i < ROWS; ++i) scaleEditor->updateAltSymbol (i);
			}
		}
	}
}

#endif /* SCALEEDITOR_HPP_ */
