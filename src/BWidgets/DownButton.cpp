/* DownButton.cpp
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

#include "DownButton.hpp"

namespace BWidgets
{
DownButton::DownButton () :
		DownButton (0.0, 0.0, BWIDGETS_DEFAULT_BUTTON_WIDTH, BWIDGETS_DEFAULT_BUTTON_HEIGHT, "upbutton", 0.0) {}

DownButton::DownButton (const double x, const double y, const double width, const double height, const std::string& name, double defaultValue) :
		Button (x, y, width, height, name, defaultValue) {}

void DownButton::draw (const double x, const double y, const double width, const double height)
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

			BColors::Color butColorLo = *bgColors.getColor (getState ()); butColorLo.applyBrightness (BWIDGETS_DEFAULT_NORMALLIGHTED);
			BColors::Color butColorHi = *bgColors.getColor (getState ()); butColorHi.applyBrightness (BWIDGETS_DEFAULT_ILLUMINATED);
			BColors::Color butColorSh = *bgColors.getColor (getState ()); butColorSh.applyBrightness (BWIDGETS_DEFAULT_SHADOWED);


			double w = getEffectiveWidth () - BWIDGETS_DEFAULT_BUTTON_DEPTH - 1;
			double h = getEffectiveHeight () - BWIDGETS_DEFAULT_BUTTON_DEPTH - 1;
			double size = (w < h ? w * 0.6 : h * 0.6);
			double x0, y0;

			if (value)
			{
				x0 = 0.5 + BWIDGETS_DEFAULT_BUTTON_DEPTH;
				y0 = 0.5 + BWIDGETS_DEFAULT_BUTTON_DEPTH;
			}
			else
			{
				x0 = 0.5;
				y0 = 0.5;
			}

			// Symbol
			cairo_save (cr);

			cairo_set_line_width (cr, 0.0);
			cairo_move_to (cr, x0 + w/2 - size/2, y0 + h/2 - size/2);
			cairo_line_to (cr, x0 + w/2, y0 + h/2 + size/2);
			cairo_line_to (cr, x0 + w/2 + size/2, y0 + h/2 - size/2);
			cairo_close_path (cr);
			cairo_clip_preserve (cr);

			cairo_set_source_rgba (cr, butColorHi.getRed (), butColorHi.getGreen (), butColorHi.getBlue (), butColorHi.getAlpha ());
			cairo_fill (cr);

			cairo_move_to (cr, x0 + w/2 - size/2 + BWIDGETS_DEFAULT_BUTTON_DEPTH, y0 + h/2 - size/2 + BWIDGETS_DEFAULT_BUTTON_DEPTH);
			cairo_line_to (cr, x0 + w/2 + BWIDGETS_DEFAULT_BUTTON_DEPTH, y0 + h/2 + size/2 + BWIDGETS_DEFAULT_BUTTON_DEPTH);
			cairo_line_to (cr, x0 + w/2 + size/2 + BWIDGETS_DEFAULT_BUTTON_DEPTH, y0 + h/2 - size/2 + BWIDGETS_DEFAULT_BUTTON_DEPTH);
			cairo_close_path (cr);
			cairo_set_source_rgba (cr, butColorSh.getRed (), butColorSh.getGreen (), butColorSh.getBlue (), butColorSh.getAlpha ());
			cairo_fill (cr);

			cairo_restore (cr);

			// Symbol frame
			cairo_set_line_width (cr, 0.2 * BWIDGETS_DEFAULT_BUTTON_DEPTH);
			cairo_move_to (cr, x0 + w/2 + size/2, y0 + h/2 - size/2);
			cairo_line_to (cr, x0 + w/2 - size/2, y0 + h/2 - size/2);
			cairo_line_to (cr, x0 + w/2, y0 + h/2 + size/2);
			cairo_set_source_rgba (cr, butColorHi.getRed (), butColorHi.getGreen (), butColorHi.getBlue (), butColorHi.getAlpha ());
			cairo_stroke (cr);

			cairo_move_to (cr, x0 + w/2, y0 + h/2 + size/2);
			cairo_line_to (cr, x0 + w/2 + size/2, y0 + h/2 - size/2);
			cairo_set_source_rgba (cr, butColorSh.getRed (), butColorSh.getGreen (), butColorSh.getBlue (), butColorSh.getAlpha ());
			cairo_stroke (cr);

			cairo_destroy (cr);
		}
	}
}
}
