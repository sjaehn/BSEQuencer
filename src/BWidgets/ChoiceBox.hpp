/* ChoiceBox.hpp
 * Copyright (C) 2018  Sven Jähnichen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef BWIDGETS_CHOICEBOX_HPP_
#define BWIDGETS_CHOICEBOX_HPP_

#include "BItems.hpp"
#include "ValueWidget.hpp"
#include "Label.hpp"
#include "UpButton.hpp"
#include "DownButton.hpp"
#include <cmath>

#define BWIDGETS_DEFAULT_CHOICEBOX_WIDTH 100.0
#define BWIDGETS_DEFAULT_CHOICEBOX_HEIGHTH 40.0
#define BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT 9.0
#define BWIDGETS_DEFAULT_CHOICEBOX_LINE_HEIGHT 1.6666

#define BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_NAME "/button"
#define BWIDGETS_DEFAULT_CHOICEBOX_ITEM_NAME "/item"
#define BWIDGETS_DEFAULT_CHOICEBOX_ITEM_PADDING 4.0
#define BWIDGETS_DEFAULT_CHOICEBOX_PADDING (BWIDGETS_DEFAULT_MENU_PADDING - BWIDGETS_DEFAULT_CHOICEBOX_ITEM_PADDING)

#ifndef UNSELECTED
#define UNSELECTED -HUGE_VAL
#endif

namespace BWidgets
{

/**
 * Class BWidgets::ChoiceBox
 *
 * Base Widget for selection of one item (string) out of a vector of strings.
 * It is a composite value widget consisting of a vector of labels, an up
 * button, and a down button. Selection directly results using the buttons.
 * The widget only shows the selected label. The value of this widget reflects
 * the number the item selected starting with 1.0. On change, a value changed
 * event is emitted and this widget.
 */
class ChoiceBox : public ValueWidget
{
public:
	ChoiceBox ();
	ChoiceBox (const double x, const double y, const double width, const double height,
				const std::string& name, std::vector<std::string> strings = {}, double preselection = UNSELECTED);
	ChoiceBox (const double x, const double y, const double width, const double height,
				const std::string& name, std::vector<BItems::Item> items = {}, double preselection = UNSELECTED);

	/**
	 * Creates a new (orphan) choice box and copies the properties from a
	 * source choice box widget.
	 * @param that Source choice box
	 */
	ChoiceBox (const ChoiceBox& that);

	~ChoiceBox ();

	/**
	 * Assignment. Copies the properties from a source choice box widget
	 * and keeps its name and its position within the widget tree. Emits a
	 * BEvents::ExposeEvent if the text widget is visible.
	 * @param that Source text widget
	 */
	ChoiceBox& operator= (const ChoiceBox& that);

	/**
	 * Gets (a pointer to) the vector of items and thus gets access to the
	 * internally stored list of items.
	 * @return Pointer to a string vector
	 */
	std::vector<BItems::Item>* getItemList ();

	/**
	 * Gets an item of the internally stored list of items.
	 * @param value Value of the item.
	 * @return Active item
	 */
	BItems::Item getItem (const double value) const;

	/**
	 * Gets the active item.
	 * @return Active item
	 */
	BItems::Item getActiveItem () const;

	/**
	 * Adds a new item or new items to the end of the internally stored list of
	 * items.
	 * @param newBItems::Item	A single new item to add.
	 * @param newBItems::Items	A vector of new items to add.
	 */
	void addItem (const BItems::Item& newItem);
	void addItem (const std::vector<BItems::Item>& newItems);

	/**
	 * Creates a new item from a text string or new items from a vector of text
	 * strings and appends it or them to the end of the internally stored list
	 * of items. The value of the added item will be set to the next full
	 * number following to the value of the last item before.
	 * @param newBItems::ItemText	Text string of a single new item to add.
	 * @param newBItems::ItemTexts	A vector of text strings of new items to add.
	 */
	void addItemText (const std::string& newItemText);
	void addItemText (const std::vector<std::string>& newItemTexts);

	/**
	 * Sets the BColors::ColorSet for this widget
	 * @param colors Color set.
	 */
	void setTextColors (const BColors::ColorSet& colorset);

	/**
	 * Gets (a pointer to) the BColors::ColorSet of this widget.
	 * @return Pointer to the color set.
	 */
	BColors::ColorSet* getTextColors ();

	/**
	 * Sets the font for the text output.
	 * @param font Font
	 */
	void setFont (const BStyles::Font& font);

	/**
	 * Gets (a pointer to) the font for the text output.
	 * @return Pointer to font
	 */
	BStyles::Font* getFont ();

	/**
	 * Scans theme for widget properties and applies these properties.
	 * @param theme Theme to be scanned.
	 * @param name Name of the BStyles::StyleSet within the theme to be
	 * 		  	   applied.
	 */
	virtual void applyTheme (BStyles::Theme& theme);
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name);

	/**
	 * Changes the value of the widget. Emits a value changed event and (if
	 * visible) an expose event.
	 * @param val Value
	 */
	virtual void setValue (const double val) override;

	/**
	 * Gets the number of top line of the shown list. In this case
	 * (BWidgets::ChoiceBox) it returns the number (not the value!) of the
	 * active item.
	 * @param return Top line of the list (starting with 1.0)
	 */
	virtual int getTop () const;

	/**
	 * Gets the bottom line of the shown list.
	 * @param return Bottom line of the list (starting with 1.0)
	 */
	int getBottom ();

	/**
	 * Calls a redraw of the widget and calls postRedisplay () if the the
	 * Widget is visible.
	 * This method should be called if the widgets properties are indirectly
	 * changed.
	 */
	virtual void update () override;

protected:
	void deleteLabels ();
	void validateLabels ();
	virtual void updateLabels ();
	virtual int getLines ();
	static void handleButtonClicked (BEvents::Event* event);
	static void handleLabelClicked (BEvents::Event* event);

	BColors::ColorSet itemColors;
	BStyles::Font itemFont;
	BStyles::Border itemBorder;
	BStyles::Fill itemBackground;
	UpButton upButton;
	DownButton downButton;
	std::vector<BItems::Item> items;
	std::vector<Label*> labels;

	int activeNr;
};

}

#endif /* BWIDGETS_CHOICEBOX_HPP_ */
