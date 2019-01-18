/* ItemBox.cpp
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

#include "ItemBox.hpp"

namespace BWidgets
{
ItemBox::ItemBox () : ItemBox (0.0, 0.0, 0.0, 0.0, "itembox", {UNSELECTED, ""}) {}

ItemBox::ItemBox (const double x, const double y, const double width, const double height, const std::string& name, const BItems::Item& item) :
		ValueWidget (x, y, width, height, name, item.value),
		itemLabel (0, 0, 0, 0, name + BWIDGETS_DEFAULT_ITEMBOX_ITEM_NAME, item.string)

{
	itemLabel.getBorder () -> setPadding (BWIDGETS_DEFAULT_ITEMBOX_ITEM_PADDING);
	itemLabel.getFont ()->setTextAlign (BWIDGETS_DEFAULT_TEXT_ALIGN);
	itemLabel.getFont ()->setTextVAlign (BWIDGETS_DEFAULT_TEXT_VALIGN);
	itemLabel.setClickable (false);

	background_ = BWIDGETS_DEFAULT_MENU_BACKGROUND;
	border_ = BWIDGETS_DEFAULT_MENU_BORDER;

	add (itemLabel);
}

ItemBox::ItemBox (const ItemBox& that) : ValueWidget (that), itemLabel (that.itemLabel)
{
	add (itemLabel);
}

ItemBox::~ItemBox () {}

ItemBox& ItemBox::operator= (const ItemBox& that)
{
	itemLabel = that.itemLabel;

	Widget::operator= (that);
	return *this;
}

void ItemBox::setItem (const BItems::Item& item)
{
	setValue (item.value);
	itemLabel.setText (item.string);
}

void ItemBox::setItemText (const std::string& text) {itemLabel.setText (text);}

BItems::Item ItemBox::getItem () const {return {getValue (), itemLabel.getText ()};}

void ItemBox::setTextColors (const BColors::ColorSet& colorset) {itemLabel.setTextColors (colorset);}

BColors::ColorSet* ItemBox::getTextColors () {return itemLabel.getTextColors ();}

void ItemBox::setFont (const BStyles::Font& font) {itemLabel.setFont (font);}

BStyles::Font* ItemBox::getFont () {return itemLabel.getFont ();}

Label* ItemBox::getLabel () {return &itemLabel;}

void ItemBox::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void ItemBox::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);

	// Item styles
	void* borderPtr = theme.getStyle(name + BWIDGETS_DEFAULT_ITEMBOX_ITEM_NAME, BWIDGETS_KEYWORD_BORDER);
	if (borderPtr) itemLabel.setBorder (*((BStyles::Border*) borderPtr));
	void* backgroundPtr = theme.getStyle(name + BWIDGETS_DEFAULT_ITEMBOX_ITEM_NAME, BWIDGETS_KEYWORD_BACKGROUND);
	if (backgroundPtr) itemLabel.setBackground (*((BStyles::Fill*) backgroundPtr));
	void* colorsPtr = theme.getStyle(name + BWIDGETS_DEFAULT_ITEMBOX_ITEM_NAME, BWIDGETS_KEYWORD_TEXTCOLORS);
	if (colorsPtr) itemLabel.setTextColors (*((BColors::ColorSet*) colorsPtr));
	void* fontPtr = theme.getStyle(name + BWIDGETS_DEFAULT_ITEMBOX_ITEM_NAME, BWIDGETS_KEYWORD_FONT);
	if (fontPtr) itemLabel.setFont (*((BStyles::Font*) fontPtr));

	update ();
}

void ItemBox::update ()
{
	// Update super widget first
	Widget::update ();

	// Set position of label
	double x0 = getXOffset ();
	double y0 = getYOffset ();
	double w = getEffectiveWidth ();
	double h = getEffectiveHeight ();

	itemLabel.moveTo (x0 + BWIDGETS_DEFAULT_ITEMBOX_PADDING, y0);
	itemLabel.setWidth (w - 2 * BWIDGETS_DEFAULT_ITEMBOX_PADDING > 0 ? w - 2 * BWIDGETS_DEFAULT_ITEMBOX_PADDING : 0);
	itemLabel.setHeight (h);
}

}
