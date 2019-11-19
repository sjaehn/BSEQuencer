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

#include <utility>
#include "definitions.h"
#include "BScale.hpp"
#include "BWidgets/Widget.hpp"
#include "BWidgets/ValueWidget.hpp"
#include "BWidgets/HPianoRoll.hpp"
#include "BWidgets/Label.hpp"
#include "BWidgets/PopupListBox.hpp"
#include "BWidgets/TextButton.hpp"
#include "BWidgets/ImageIcon.hpp"

class ScaleEditor : public BWidgets::ValueWidget
{
public:
	ScaleEditor ();
	ScaleEditor (const double x, const double y, const double width, const double height,
		     const std::string& name, const std::string& pluginPath,
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
	static void symbolListboxValueChangedCallback (BEvents::Event* event);
	static void noteListboxValueChangedCallback (BEvents::Event* event);
	static void buttonClickCallback (BEvents::Event* event);
	static void labelClickCallback (BEvents::Event* event);
	static void labelMessageCallback (BEvents::Event* event);
	static void pianoClickCallback (BEvents::Event* event);

	BWidgets::Label nameLabel;
	BWidgets::Label scaleNameLabel;
	BWidgets::Label rowLabel;
	BWidgets::Label symbolLabel;
	BWidgets::Label noteLabel;
	BWidgets::Label altSymbolLabel;
	std::array<BWidgets::ImageIcon, ROWS> drumSymbol;
	std::array<BWidgets::ImageIcon, ROWS>noteSymbol;
	std::array<BWidgets::Label, ROWS> nrLabel;
	std::array<BWidgets::PopupListBox, ROWS> nrSymbolListbox;
	std::array<BWidgets::PopupListBox, ROWS> nrNoteListbox;
	std::array<BWidgets::Label, ROWS> nrNoteLabel;
	std::array<BWidgets::Label, ROWS> nrAltSymbolLabel;
	BWidgets::HPianoRoll piano;
	BWidgets::TextButton cancelButton;
	BWidgets::TextButton applyButton;

	int pianoRoot;
	BScale scale;
	ScaleMap scaleMap;
	int mapNr;

	double sz;

	cairo_surface_t* bgSurface;

	BColors::ColorSet txColors = {{{0.167, 0.37, 0.80, 1.0}, {0.33, 0.5, 0.85, 1.0}, {0.0, 0.0, 0.25, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet bgColors = {{{0.15, 0.15, 0.15, 1.0}, {0.3, 0.3, 0.3, 1.0}, {0.05, 0.05, 0.05, 1.0}, {0.0, 0.0, 0.0, 1.0}}};
	BStyles::Border border = {{BColors::white, 1.0}, 0.0, 2.0, 0.0};
	BStyles::Border menuBorder = {{BColors::darkgrey, 1.0}, 0.0, 0.0, 0.0};
	BStyles::Border labelborder = {BStyles::noLine, 4.0, 0.0, 0.0};
	BStyles::Fill scaleEditorBg = BStyles::blackFill;
	BStyles::Fill menuBg = BStyles::Fill (BColors::Color (0.0, 0.0, 0.05, 1.0));
	BStyles::Font ctLabelFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
						   BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
	BStyles::Font lfLabelFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
						   BStyles::TEXT_ALIGN_LEFT, BStyles::TEXT_VALIGN_MIDDLE);
	BStyles::StyleSet defaultStyles = {"default", {{"background", STYLEPTR (&BStyles::noFill)},
					  {"border", STYLEPTR (&BStyles::noBorder)}}};
	BStyles::StyleSet labelStyles = {"labels", {{"background", STYLEPTR (&BStyles::noFill)},
						   {"border", STYLEPTR (&BStyles::noBorder)},
						   {"textcolors", STYLEPTR (&txColors)},
						   {"font", STYLEPTR (&ctLabelFont)}}};

	BStyles::Theme theme = BStyles::Theme ({
		defaultStyles,
		{"scaleeditor", 	{{"background", STYLEPTR (&scaleEditorBg)},
					{"border", STYLEPTR (&border)}}},
		{"widget", 		{{"uses", STYLEPTR (&defaultStyles)}}},
		{"ctlabel",	 	{{"uses", STYLEPTR (&labelStyles)},
					 {"font", STYLEPTR (&ctLabelFont)}}},
		{"lflabel",	 	{{"uses", STYLEPTR (&labelStyles)},
					 {"font", STYLEPTR (&lfLabelFont)}}},
		{"button", 		{{"font", STYLEPTR (&ctLabelFont)}}},
		{"xsymbol",	 	{{"uses", STYLEPTR (&defaultStyles)},
					 {"fgcolors", STYLEPTR (&BColors::whites)},
					 {"font", STYLEPTR (&ctLabelFont)}}},
		{"menu",	 	{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&menuBg)}}},
		{"menu/item",	 	{{"uses", STYLEPTR (&defaultStyles)},
					 {"border", STYLEPTR (&labelborder)},
					 {"textcolors", STYLEPTR (&BColors::whites)},
					 {"font", STYLEPTR (&lfLabelFont)}}},
		{"menu/button",	 	{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&menuBg)},
					 {"bgcolors", STYLEPTR (&bgColors)}}},
		{"menu/listbox",	{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&menuBg)}}},
		{"menu/listbox/item",	{{"uses", STYLEPTR (&defaultStyles)},
					 {"border", STYLEPTR (&labelborder)},
					 {"textcolors", STYLEPTR (&BColors::whites)},
					 {"font", STYLEPTR (&lfLabelFont)}}},
		{"menu/listbox//button",{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&menuBg)},
					 {"bgcolors", STYLEPTR (&bgColors)}}}
	});

};

ScaleEditor::ScaleEditor() : ScaleEditor (0, 0, 0, 0, "editor", "", 0, ScaleMap (), BScale (0, defaultScale)) {}

ScaleEditor::ScaleEditor(const double x, const double y, const double width, const double height, const std::string& name,
						 const std::string& pluginPath, const int mapNr, const ScaleMap& scaleMap, const BScale& scale) :
		BWidgets::ValueWidget (x, y, width, height, name, 0.0),
		nameLabel (20, 60, 80, 20, "lflabel", "Scale name:"),
		scaleNameLabel (120, 60, 320, 20, "lflabel", std::string (scaleMap.name)),
		rowLabel (20, 100, 80, 20, "lflabel", "Row"),
		symbolLabel (70, 100, 48, 20, "ctlabel", "Mode"),
		noteLabel (148, 100, 80, 20, "ctlabel", "Note"),
		altSymbolLabel (248, 100, 80, 20, "ctlabel", "Symbol"),
		piano (50, 620, 260, 60, "widget", 0, 35),
		cancelButton (60, 720, 60, 20, "button", "Cancel"),
		applyButton (240, 720, 60, 20, "button", "Apply"),
		pianoRoot (0), scale (scale), scaleMap (scaleMap), mapNr (mapNr),
		sz (width / 360 < height / 760 ? width / 360 : height / 760)


{
	setDraggable (true);
	setFocusable (true);	// Only to block underlying pads callback

	bgSurface = cairo_image_surface_create_from_png ((pluginPath + "ScaleEditor.png").c_str());
	scaleEditorBg.loadFillFromCairoSurface(bgSurface);

	scaleNameLabel.setEditable (true);
	scaleNameLabel.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, labelClickCallback);
	scaleNameLabel.setCallbackFunction(BEvents::MESSAGE_EVENT, labelMessageCallback);

	// Create note symbols (for nrListbox)
	BScale sc = BScale(0, {CROMATICSCALE});
	std::list<BItems::Item> noteNameItems;
	for (int i = 0; i < 120; ++i)
	{
		int octave = (i / 12) - 1;
		std::string strNote = std::to_string(i) + " (" + sc.getSymbol (i) + (octave != 0 ? std::to_string(octave) : "") + ")";
		noteNameItems.push_back (BItems::Item (i, strNote));
	}

	// Init nr widgets
	for (int i = 0; i < ROWS; ++i)
	{

		drumSymbol[i] = BWidgets::ImageIcon (0, 0, 48, 24, "widget", pluginPath + "DrumSymbol.png");
		drumSymbol[i].rename ("widget");
		drumSymbol[i].applyTheme (theme);
		noteSymbol[i] = BWidgets::ImageIcon (0, 0, 48, 24, "widget", pluginPath + "NoteSymbol.png");
		noteSymbol[i].rename ("widget");
		noteSymbol[i].applyTheme (theme);

		BItems::ItemList il;
		il.push_back (BItems::Item (0, &noteSymbol[i]));
		il.push_back (BItems::Item (1, &drumSymbol[i]));

		nrSymbolListbox[i] =  BWidgets::PopupListBox (60, 580 - i * 30, 68, 24, 68, 68, "menu", il, 0);
		nrSymbolListbox[i].setCallbackFunction(BEvents::VALUE_CHANGED_EVENT, symbolListboxValueChangedCallback);
		nrSymbolListbox[i].rename ("menu");
		nrSymbolListbox[i].applyTheme (theme);
		add (nrSymbolListbox[i]);

		nrLabel[i] = BWidgets::Label (20, 580 - i * 30, 30, 24, "lflabel", std::to_string (i + 1));
		nrLabel[i].rename ("lflabel");
		nrLabel[i].applyTheme (theme);
		add (nrLabel[i]);

		if (i >= 6) nrNoteListbox[i] = BWidgets::PopupListBox (148, 580 - i * 30, 80, 24, 80, 240, "menu", noteNameItems, 0);
		else nrNoteListbox[i] = BWidgets::PopupListBox (148, 580 - i * 30, 80, 24, 0, -240, 80, 240, "menu", noteNameItems, 0);
		nrNoteListbox[i].setCallbackFunction(BEvents::VALUE_CHANGED_EVENT, noteListboxValueChangedCallback);
		nrNoteListbox[i].rename ("menu");
		nrNoteListbox[i].applyTheme (theme);
		add (nrNoteListbox[i]);

		nrNoteLabel[i] = BWidgets::Label (148, 580 - i * 30, 80, 24, "ctlabel", "(uses scale)");
		nrNoteLabel[i].rename ("ctlabel");
		nrNoteLabel[i].applyTheme (theme);
		add (nrNoteLabel[i]);
		nrNoteLabel[i].hide ();

		nrAltSymbolLabel[i] = BWidgets::Label (248, 580 - i * 30, 80, 24, "ctlabel", "");
		nrAltSymbolLabel[i].rename ("ctlabel");
		nrAltSymbolLabel[i].setEditable (true);
		nrAltSymbolLabel[i].setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, labelClickCallback);
		nrAltSymbolLabel[i].setCallbackFunction(BEvents::MESSAGE_EVENT, labelMessageCallback);
		updateAltSymbol (i);
		nrAltSymbolLabel[i].applyTheme (theme);
		add (nrAltSymbolLabel[i]);
	}

	cancelButton.setCallbackFunction(BEvents::BUTTON_CLICK_EVENT, buttonClickCallback);
	applyButton.setCallbackFunction(BEvents::BUTTON_CLICK_EVENT, buttonClickCallback);

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
	cancelButton.applyTheme (theme);
	applyButton.applyTheme (theme);
	piano.applyTheme (theme);
	applyTheme (theme);

	add (nameLabel);
	add (scaleNameLabel);
	add (rowLabel);
	add (symbolLabel);
	add (noteLabel);
	add (altSymbolLabel);
	add (cancelButton);
	add (applyButton);
	add (piano);

	if (sz != 1.0) szScaleEditor();
}

void ScaleEditor::resize (double width, double height)
{
	double wf = width / 360;
	double hf = height / 760;
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
	cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 360 * sz, 760 * sz);
	cairo_t* cr = cairo_create (surface);
	cairo_scale (cr, sz, sz);
	cairo_set_source_surface(cr, bgSurface, 0, 0);
	cairo_paint(cr);
	scaleEditorBg.loadFillFromCairoSurface(surface);
	cairo_destroy (cr);
	cairo_surface_destroy (surface);

	nameLabel.moveTo (20 * sz, 60 * sz); nameLabel.resize (80 * sz, 20 * sz);
	scaleNameLabel.moveTo (120 * sz, 60 * sz); scaleNameLabel.resize(320 * sz, 20 * sz);
	rowLabel.moveTo (20 * sz, 100 * sz); rowLabel.resize (80 * sz, 20 * sz);
	symbolLabel.moveTo (70 * sz, 100 * sz); symbolLabel.resize (48 * sz, 20 * sz);
	noteLabel.moveTo (148 * sz, 100 * sz); noteLabel.resize (80 * sz, 20 * sz);
	altSymbolLabel.moveTo (248 * sz, 100 * sz); altSymbolLabel.resize (80 * sz, 20 * sz);
	cancelButton.moveTo (60 * sz, 720 * sz); cancelButton.resize (60 * sz, 20 * sz);
	applyButton.moveTo (240 * sz, 720 * sz); applyButton.resize (60 * sz, 20 * sz);
	piano.moveTo (50 * sz, 620 * sz); piano.resize (260 * sz, 60 * sz);

	for (int i = 0; i < ROWS; ++i)
	{
		nrLabel[i].moveTo (20 * sz, (580 - i * 30) * sz); nrLabel[i].resize (30 * sz, 24 * sz);
		nrSymbolListbox[i].moveTo (60 * sz, (580 - i * 30) * sz); nrSymbolListbox[i].resize (68 * sz, 24 * sz);
		nrSymbolListbox[i].resizeListBox (68 * sz, 68 * sz);

		nrNoteListbox[i].moveTo (148 * sz, (580 - i * 30) * sz ); nrNoteListbox[i].resize (80 * sz, 24 * sz);
		nrNoteListbox[i].resizeListBox (80 * sz, 240 * sz);
		if (i < 6) nrNoteListbox[i].moveListBox(0, -240 * sz);
		nrNoteListbox[i].resizeListBoxItems (80 * sz, 24 * sz);

		nrNoteLabel[i].moveTo (148 * sz, (580 - i * 30) * sz); nrNoteLabel[i].resize (80 * sz, 24 * sz);
		nrAltSymbolLabel[i].moveTo (248 * sz, (580 - i * 30) * sz); nrAltSymbolLabel[i].resize (80 * sz, 24 * sz);
	}

	nameLabel.applyTheme (theme);
	scaleNameLabel.applyTheme (theme);
	rowLabel.applyTheme (theme);
	symbolLabel.applyTheme (theme);
	noteLabel.applyTheme (theme);
	altSymbolLabel.applyTheme (theme);
	cancelButton.applyTheme (theme);
	applyButton.applyTheme (theme);
	piano.applyTheme (theme);

	for (int i = 0; i < ROWS; ++i)
	{
		nrLabel[i].applyTheme (theme);
		nrSymbolListbox[i].applyTheme (theme);
		nrNoteListbox[i].applyTheme (theme);
		nrNoteLabel[i].applyTheme (theme);
		nrAltSymbolLabel[i].applyTheme (theme);
	}

	applyTheme (theme);
	Widget::resize (360 * sz, 760 * sz);
}

void ScaleEditor::setScaleMap (const ScaleMap& scaleMap)
{
	this->scaleMap = scaleMap;

	scaleNameLabel.setText(std::string(scaleMap.name));

	// Set nr widgets
	for (int i = 0; i < ROWS; ++i)
	{
		if (scaleMap.elements[i] & 0x0100)
		{
			nrSymbolListbox[i].setValue (1);
			nrNoteLabel[i].hide ();
			nrNoteListbox[i].setValue (scaleMap.elements[i] & 0x0FF);
			nrNoteListbox[i].show ();
		}

		else
		{
			nrSymbolListbox[i].setValue (0);
			nrNoteListbox[i].hide ();
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

		if (this->scale.getElement (i/* + this->scale.getRoot()*/) != ENOTE) prKeys.push_back (true);
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
	else if (!(scaleMap.elements[nr] & 0x0100)) symbol = scale.getSymbol (scaleMap.elements[nr]);
	nrAltSymbolLabel[nr].setText (symbol);
}

void ScaleEditor::symbolListboxValueChangedCallback (BEvents::Event* event)
{
	if ((!event) || (!event->getWidget()) || (!event->getWidget()->getParent())) return;

	BWidgets::PopupListBox* listbox = (BWidgets::PopupListBox*)(event->getWidget());
	ScaleEditor* scaleEditor = (ScaleEditor*)(listbox->getParent());
	int nr = -1;
	int value = ((BEvents::ValueChangedEvent*)event)->getValue ();

	for (int i = 0; i < ROWS; ++i)
	{
		if (listbox == &(scaleEditor->nrSymbolListbox[i]))
		{
			nr = i;
			break;
		}
	}

	if (nr >= 0)
	{
		if (value)
		{
			scaleEditor->scaleMap.elements[nr] = ((int(scaleEditor->nrNoteListbox[nr].getValue())) | 0x0100);
			scaleEditor->nrNoteListbox[nr].show ();
			scaleEditor->nrNoteLabel[nr].hide ();
			BWidgets::Label* l = (BWidgets::Label*) scaleEditor->nrNoteListbox[nr].getItem()->getWidget ();
			if (l) strncpy (scaleEditor->scaleMap.altSymbols[nr], l->getText ().c_str(), 15);
			scaleEditor->updateAltSymbol (nr);
		}

		else
		{
			scaleEditor->scaleMap.elements[nr] = 0;	// Will be substituted by auto numbering later
			scaleEditor->nrNoteListbox[nr].hide ();
			scaleEditor->nrNoteLabel[nr].show ();
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
			scaleEditor->updateAltSymbol (i);
		}
	}
}

void ScaleEditor::noteListboxValueChangedCallback (BEvents::Event* event)
{
	if ((!event) || (!event->getWidget()) || (!event->getWidget()->getParent())) return;

	BWidgets::PopupListBox* listbox = (BWidgets::PopupListBox*)(event->getWidget());
	ScaleEditor* scaleEditor = (ScaleEditor*)(listbox->getParent());
	int nr = -1;

	for (int i = 0; i < ROWS; ++i)
	{
		if (listbox == &(scaleEditor->nrNoteListbox[i]))
		{
			nr = i;
			break;
		}
	}

	if ((nr >=0 ) && (scaleEditor->scaleMap.elements[nr] & 0x0100))
	{
		scaleEditor->scaleMap.elements[nr] = (((int)(scaleEditor->nrNoteListbox[nr].getValue())) | 0x0100);
		BWidgets::Label* l = (BWidgets::Label*) scaleEditor->nrNoteListbox[nr].getItem()->getWidget ();
		if (l) strncpy (scaleEditor->scaleMap.altSymbols[nr], l->getText ().c_str(), 15);
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
			scaleEditor->postCloseRequest ();
		}
	}
}

void ScaleEditor::labelClickCallback (BEvents::Event* event)
{
	if ((event) && (event->getEventType() == BEvents::BUTTON_PRESS_EVENT) && (event->getWidget()))
	{
		BWidgets::Label* label = (BWidgets::Label*) event->getWidget();
		if (label->getParent ())
		{
			ScaleEditor* scaleEditor = (ScaleEditor*)(label->getParent());

			// Switch off edit mode for all other editable labels
			if (label == &scaleEditor->scaleNameLabel)
			{
				for (BWidgets::Label& l : scaleEditor->nrAltSymbolLabel)
				{
					if (l.getEditMode ()) l.applyEdit ();
				}
			}

			else
			{
				if (scaleEditor->scaleNameLabel.getMainWindow ()) scaleEditor->scaleNameLabel.getMainWindow ()->removeKeyGrab (&scaleEditor->scaleNameLabel);
				scaleEditor->scaleNameLabel.setEditMode (false);
				for (BWidgets::Label& l : scaleEditor->nrAltSymbolLabel)
				{
					if ((label != &l) && l.getEditMode ()) l.applyEdit ();
				}
			}
		}
	}
}

void ScaleEditor::labelMessageCallback (BEvents::Event* event)
{
	if ((event) && (event->getEventType() == BEvents::MESSAGE_EVENT) && (event->getWidget()))
	{
		BEvents::MessageEvent* me = (BEvents::MessageEvent*) event;

		if (me->getName () == BWIDGETS_LABEL_TEXT_CHANGED_MESSAGE)
		{
			BWidgets::Label* label = (BWidgets::Label*) event->getWidget();
			if (label->getParent ())
			{
				ScaleEditor* scaleEditor = (ScaleEditor*)(label->getParent());

				if (label == &scaleEditor->scaleNameLabel) strncpy (scaleEditor->scaleMap.name, scaleEditor->scaleNameLabel.getText ().c_str (), 63);

				else for (size_t i = 0; i < ROWS; ++i)
				{
					BWidgets::Label* l = &scaleEditor->nrAltSymbolLabel[i];
					if (label == l)
					{
						strncpy (scaleEditor->scaleMap.altSymbols[i], l->getText ().c_str (), 15);
						break;
					}
				}
			}
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

				// Update auto numbering of scale map elements
				for (int i = 0, count = 0; i < ROWS; ++i)
				{
					if (!(scaleEditor->scaleMap.elements[i] & 0x0100))
					{
						scaleEditor->scaleMap.elements[i] = count;
						scaleEditor->scaleMap.altSymbols[i][0] = '\0';
						++count;
					}
					scaleEditor->updateAltSymbol (i);
				}
			}
		}
	}
}

#endif /* SCALEEDITOR_HPP_ */
