/* UpButton.cpp
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

#include "UpButton.hpp"

namespace BWidgets
{
UpButton::UpButton () :
		UpButton (0.0, 0.0, BWIDGETS_DEFAULT_BUTTON_WIDTH, BWIDGETS_DEFAULT_BUTTON_HEIGHT, "upbutton", 0.0) {}

UpButton::UpButton (const double x, const double y, const double width, const double height, const std::string& name, double defaultValue) :
		Button (x, y, width, height, name, defaultValue) {}

Widget* UpButton::clone () const {return new UpButton (*this);}

void UpButton::draw (const double x, const double y, const double width, const double height)
{
	if ((!widgetSurface) || (cairo_surface_status (widgetSurface) != CAIRO_STATUS_SUCCESS)) return;

	if ((width_ >= 6) && (height_ >= 6))
	{

		Button::draw (x, y, width, height);

		cairo_t* cr = cairo_create (widgetSurface);
		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			// Limit cairo-drawing area
			cairo_rectangle (cr, x, y, width, height);
			cairo_clip (cr);

			double x0 = getXOffset ();
			double y0 = getYOffset ();
			double w = getEffectiveWidth ();
			double h = getEffectiveHeight ();
			double size = (w < h ? w * 0.6 : h * 0.6);
			BColors::Color butColor = *bgColors.getColor (getState ()); butColor.applyBrightness (BWIDGETS_DEFAULT_NORMALLIGHTED);
			BColors::Color frColor= *bgColors.getColor (getState ());

			if (value) frColor.applyBrightness (2 * BWIDGETS_DEFAULT_ILLUMINATED);
			else frColor.applyBrightness (2 * BWIDGETS_DEFAULT_SHADOWED);

			// Symbol
			cairo_set_line_width (cr, BWIDGETS_DEFAULT_BUTTON_BORDER);
			cairo_move_to (cr, x0 + w/2 - size/2, y0 + h/2 + size/4);
			cairo_line_to (cr, x0 + w/2, y0 + h/2 - size/4);
			cairo_line_to (cr, x0 + w/2 + size/2, y0 + h/2 + size/4);
			cairo_set_source_rgba (cr, CAIRO_RGBA (frColor));
			cairo_stroke (cr);

			cairo_destroy (cr);
		}
	}
}
}
