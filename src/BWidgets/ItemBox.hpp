/* ItemBox.hpp
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

#ifndef BWIDGETS_ITEMBOX_HPP_
#define BWIDGETS_ITEMBOX_HPP_

#include "BItems.hpp"
#include "Label.hpp"
#include "ValueWidget.hpp"
#include <cmath>

#define BWIDGETS_DEFAULT_ITEMBOX_WIDTH 100.0
#define BWIDGETS_DEFAULT_ITEMBOX_HEIGHT 20.0

#define BWIDGETS_DEFAULT_ITEMBOX_ITEM_NAME "/item"
#define BWIDGETS_DEFAULT_ITEMBOX_ITEM_PADDING 4.0
#define BWIDGETS_DEFAULT_ITEMBOX_PADDING (BWIDGETS_DEFAULT_MENU_PADDING - BWIDGETS_DEFAULT_ITEMBOX_ITEM_PADDING)

#ifndef UNSELECTED
#define UNSELECTED -HUGE_VAL
#endif

namespace BWidgets
{

/**
 * Class BWidgets::ItemBox
 *
 * Single line text box widget displaying the text string of a BItems::Item.
 */
class ItemBox : public ValueWidget
{
public:
	ItemBox ();
	ItemBox (const double x, const double y, const double width, const double height, const std::string& name, const BItems::Item& item);

	/**
	 * Creates a new (orphan) item box and copies the properties from a
	 * source item box widget.
	 * @param that Source choice box
	 */
	ItemBox (const ItemBox& that);

	~ItemBox ();

	/**
	 * Assignment. Copies the properties from a source item box widget
	 * and keeps its name and its position within the widget tree. Emits a
	 * BEvents::ExposeEvent if the text widget is visible.
	 * @param that Source text widget
	 */
	ItemBox& operator= (const ItemBox& that);

	/**
	 * Sets the item of this widget.
	 * @param item Item.
	 */
	void setItem (const BItems::Item& item);

	/**
	 * Sets only the text of the item.
	 * @param text Text string
	 */
	void setItemText (const std::string& text);

	/**
	 * Gets the item of the widget.
	 * @return Item.
	 */
	BItems::Item getItem () const;

	/**
	 * Gets (a pointer to) the internal BWidgets::Label of the widget.
	 * @return BWidgets::Label of the widget
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

#endif /* BWIDGETS_ITEMBOX_HPP_ */
