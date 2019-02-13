/* Label.cpp
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

#include "Label.hpp"

namespace BWidgets
{
Label::Label () : Label (0.0, 0.0, 0.0, 0.0, "label", "") {}

Label::Label (const double x, const double y, const double width, const double height, const std::string& text) :
		Label (x, y, width, height, text, text) {}

Label::Label (const double x, const double y, const double width, const double height, const std::string& name, const std::string& text) :
		Widget (x, y, width, height, name), labelColors (BWIDGETS_DEFAULT_TEXT_COLORS), labelFont (BWIDGETS_DEFAULT_FONT), labelText (text)
{
	labelFont.setTextAlign (BWIDGETS_DEFAULT_LABEL_ALIGN);
	labelFont.setTextVAlign (BWIDGETS_DEFAULT_LABEL_VALIGN);
}

Label::Label (const Label& that) : Widget (that)
{
	labelColors = that.labelColors;
	labelFont = that.labelFont;
	labelText = that.labelText;
}

Label::~Label () {}

Label& Label::operator= (const Label& that)
{
	labelColors = that.labelColors;
	labelFont = that.labelFont;
	labelText = that.labelText;
	Widget::operator= (that);
	return *this;
}

void Label::setText (const std::string& text)
{
	if (text != labelText)
	{
		labelText = text;
		update ();
	}
}
std::string Label::getText () const {return labelText;}

void Label::setTextColors (const BColors::ColorSet& colorset)
{
	if (labelColors != colorset)
	{
		labelColors = colorset;
		update ();
	}
}
BColors::ColorSet* Label::getTextColors () {return &labelColors;}

void Label::setFont (const BStyles::Font& font)
{
	labelFont = font;
	update ();
}
BStyles::Font* Label::getFont () {return &labelFont;}

double Label::getTextWidth (std::string& text)
{
	double textwidth = 0.0;
	cairo_t* cr = cairo_create (widgetSurface);
	cairo_text_extents_t ext = labelFont.getTextExtents(cr, text.c_str ());
	textwidth = ext.width;
	cairo_destroy (cr);
	return textwidth;
}

void Label::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void Label::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);

	// Color
	void* colorsPtr = theme.getStyle(name, BWIDGETS_KEYWORD_TEXTCOLORS);
	if (colorsPtr) labelColors = *((BColors::ColorSet*) colorsPtr);

	// Font
	void* fontPtr = theme.getStyle(name, BWIDGETS_KEYWORD_FONT);
	if (fontPtr) labelFont = *((BStyles::Font*) fontPtr);

	if (colorsPtr || fontPtr) update ();
}



void Label::draw (const double x, const double y, const double width, const double height)
{
	if ((!widgetSurface) || (cairo_surface_status (widgetSurface) != CAIRO_STATUS_SUCCESS)) return;

	// Draw super class widget elements first
	Widget::draw (x, y, width, height);

	cairo_t* cr = cairo_create (widgetSurface);

	if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
	{
		// Limit cairo-drawing area
		cairo_rectangle (cr, x, y, width, height);
		cairo_clip (cr);

		double xoff = getXOffset ();
		double yoff = getYOffset ();
		double w = getEffectiveWidth ();
		double h = getEffectiveHeight ();

		cairo_text_extents_t ext = labelFont.getTextExtents(cr, labelText);
		BColors::Color lc = *labelColors.getColor (getState ());
		cairo_select_font_face (cr, labelFont.getFontFamily ().c_str (), labelFont.getFontSlant (), labelFont.getFontWeight ());
		cairo_set_font_size (cr, labelFont.getFontSize ());

		double x0, y0;

		switch (labelFont.getTextAlign ())
		{
		case BStyles::TEXT_ALIGN_LEFT:		x0 = - ext.x_bearing;
											break;
		case BStyles::TEXT_ALIGN_CENTER:	x0 = w / 2 - ext.width / 2 - ext.x_bearing;
											break;
		case BStyles::TEXT_ALIGN_RIGHT:		x0 = w - ext.width - ext.x_bearing;
											break;
		default:							x0 = 0;
		}

		switch (labelFont.getTextVAlign ())
		{
		case BStyles::TEXT_VALIGN_TOP:		y0 = - ext.y_bearing;
											break;
		case BStyles::TEXT_VALIGN_MIDDLE:	y0 = h / 2 - ext.height / 2 - ext.y_bearing;
											break;
		case BStyles::TEXT_VALIGN_BOTTOM:	y0 = h - ext.height - ext.y_bearing;
											break;
		default:							y0 = 0;
		}

		cairo_set_source_rgba (cr, lc.getRed (), lc.getGreen (), lc.getBlue (), lc.getAlpha ());
		cairo_move_to (cr, xoff + x0, yoff + y0);
		cairo_show_text (cr, labelText.c_str ());
	}

	cairo_destroy (cr);
}

}
