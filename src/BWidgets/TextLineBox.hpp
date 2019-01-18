/* TextLineBox.hpp
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

#ifndef BWIDGETS_TEXTLINEBOX_HPP_
#define BWIDGETS_TEXTLINEBOX_HPP_

#include "Label.hpp"

#define BWIDGETS_DEFAULT_ITEMBOX_WIDTH 100.0
#define BWIDGETS_DEFAULT_ITEMBOX_HEIGHT 20.0

#define BWIDGETS_DEFAULT_ITEMBOX_ITEM_NAME "/item"
#define BWIDGETS_DEFAULT_ITEMBOX_ITEM_PADDING 4.0
#define BWIDGETS_DEFAULT_ITEMBOX_PADDING (BWIDGETS_DEFAULT_MENU_PADDING - BWIDGETS_DEFAULT_ITEMBOX_ITEM_PADDING)


namespace BWidgets
{

/**
 * Class BWidgets::TextLineBox
 *
 * Single line text box widget.
 */
class TextLineBox : public Widget
{
public:
	TextLineBox ();
	TextLineBox (const double x, const double y, const double width, const double height, const std::string& name, const std::string& text);

	/**
	 * Creates a new (orphan) item box and copies the properties from a
	 * source item box widget.
	 * @param that Source choice box
	 */
	TextLineBox (const TextLineBox& that);

	~TextLineBox ();

	/**
	 * Assignment. Copies the properties from a source item box widget
	 * and keeps its name and its position within the widget tree. Emits a
	 * BEvents::ExposeEvent if the text widget is visible.
	 * @param that Source text widget
	 */
	TextLineBox& operator= (const TextLineBox& that);

	/**
	 * Sets the text of the item.
	 * @param text Text string of the item
	 */
	void setText (const std::string& text);

	/**
	 * Gets the text of the item.
	 * @return Text string of the item
	 */
	std::string getText () const;

	/**
	 * Gets (a pointer to) the internal BWidgets::Label of the item.
	 * @return BWidgets::Label of the item
	 */
	Label* getLabel ();

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
	 * Calls a redraw of the widget and calls postRedisplay () if the the
	 * Widget is visible.
	 * This method should be called if the widgets properties are indirectly
	 * changed.
	 */
	virtual void update () override;

protected:
	Label itemLabel;

};

}

#endif /* BWIDGETS_TEXTLINEBOX_HPP_ */
