/* ImageIcon.cpp
 * Copyright (C) 2019  Sven Jähnichen
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

#include "Icon.hpp"

namespace BWidgets
{
Icon::Icon () : Icon (0.0, 0.0, BWIDGETS_DEFAULT_WIDTH, BWIDGETS_DEFAULT_HEIGHT, "icon") {}

Icon::Icon (const double x, const double y, const double width, const double height, const std::string& name) :
		Widget (x, y, width, height, name), iconSurface ({})
{
	//iconSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, getEffectiveWidth (), getEffectiveHeight ());
}

Icon::Icon (const Icon& that) : Widget (that)
{
	// Copy icons
	for (cairo_surface_t* s : that.iconSurface)
	{
		cairo_surface_t* n = nullptr;
		if (s) n = cairo_image_surface_clone_from_image_surface (s);
		iconSurface.push_back (n);
	}
}

Icon::~Icon ()
{
	while (!iconSurface.empty())
	{
		cairo_surface_t* s = iconSurface.back ();
		if (s) cairo_surface_destroy (s);
		iconSurface.pop_back ();
	}
}

Icon& Icon::operator= (const Icon& that)
{
	Widget::operator= (that);

	// Clear old icons
	while (!iconSurface.empty())
	{
		cairo_surface_t* s = iconSurface.back ();
		if (s) cairo_surface_destroy (s);
		iconSurface.pop_back ();
	}

	// Copy new icons
	for (cairo_surface_t* s : that.iconSurface)
	{
		cairo_surface_t* n = nullptr;
		if (s) n = cairo_image_surface_clone_from_image_surface (s);
		iconSurface.push_back (n);
	}

	return *this;
}

Widget* Icon::clone () const {return new Icon (*this);}


void Icon::draw (const double x, const double y, const double width, const double height)
{
	if ((!widgetSurface) || (cairo_surface_status (widgetSurface) != CAIRO_STATUS_SUCCESS)) return;

	if ((width_ >= 1) && (height_ >= 1))
	{
		// Draw super class widget elements first
		Widget::draw (x, y, width, height);

		double w = getEffectiveWidth ();
		double h = getEffectiveHeight ();

		if (iconSurface.size () > getState ())
		{
			cairo_surface_t* stateSurface = iconSurface [getState ()];

			if (stateSurface && (cairo_surface_status (stateSurface) == CAIRO_STATUS_SUCCESS) && (w > 0) && (h > 0))
			{
				cairo_t* cr = cairo_create (widgetSurface);
				if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
				{
					// Limit cairo-drawing area
					cairo_rectangle (cr, x, y, width, height);
					cairo_clip (cr);
					//TODO also clip to inner borders

					double oriw = cairo_image_surface_get_width (stateSurface);
					double orih = cairo_image_surface_get_height (stateSurface);
					double sz = ((w / oriw < h / orih) ? (w / oriw) : (h / orih));
					double x0 = getXOffset () + w / 2 - oriw * sz / 2;
					double y0 = getYOffset () + h / 2 - orih * sz / 2;

					cairo_scale (cr, sz, sz);
					cairo_set_source_surface(cr, stateSurface, x0, y0);
					cairo_paint (cr);
				}

				cairo_destroy (cr);
			}
		}
	}
}

}
