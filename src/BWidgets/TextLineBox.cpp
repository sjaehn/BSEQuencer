/* TextLineBox.cpp
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

#include "TextLineBox.hpp"

namespace BWidgets
{
TextLineBox::TextLineBox () : TextLineBox (0.0, 0.0, 0.0, 0.0, "itembox", "") {}

TextLineBox::TextLineBox (const double x, const double y, const double width, const double height, const std::string& name, const std::string& text) :
		Widget (x, y, width, height, name),
		itemLabel (0, 0, 0, 0, name + BWIDGETS_DEFAULT_ITEMBOX_ITEM_NAME, text)

{
	itemLabel.getBorder () -> setPadding (BWIDGETS_DEFAULT_ITEMBOX_ITEM_PADDING);
	itemLabel.getFont ()->setTextAlign (BWIDGETS_DEFAULT_TEXT_ALIGN);
	itemLabel.getFont ()->setTextVAlign (BWIDGETS_DEFAULT_TEXT_VALIGN);
	itemLabel.setClickable (false);

	background_ = BWIDGETS_DEFAULT_MENU_BACKGROUND;
	border_ = BWIDGETS_DEFAULT_MENU_BORDER;

	add (itemLabel);
}

TextLineBox::TextLineBox (const TextLineBox& that) : Widget (that), itemLabel (that.itemLabel)
{
	add (itemLabel);
}

TextLineBox::~TextLineBox () {}

TextLineBox& TextLineBox::operator= (const TextLineBox& that)
{
	itemLabel = that.itemLabel;

	Widget::operator= (that);
	return *this;
}
void TextLineBox::setText (const std::string& text) {itemLabel.setText (text);}

std::string TextLineBox::getText () const {return itemLabel.getText ();}

void TextLineBox::setTextColors (const BColors::ColorSet& colorset) {itemLabel.setTextColors (colorset);}

BColors::ColorSet* TextLineBox::getTextColors () {return itemLabel.getTextColors ();}

void TextLineBox::setFont (const BStyles::Font& font) {itemLabel.setFont (font);}

BStyles::Font* TextLineBox::getFont () {return itemLabel.getFont ();}

Label* TextLineBox::getLabel () {return &itemLabel;}

void TextLineBox::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void TextLineBox::applyTheme (BStyles::Theme& theme, const std::string& name)
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

void TextLineBox::update ()
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
