/* ImageIcon.cpp
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

#include "ImageIcon.hpp"

namespace BWidgets
{
ImageIcon::ImageIcon () : ImageIcon (0.0, 0.0, BWIDGETS_DEFAULT_WIDTH, BWIDGETS_DEFAULT_HEIGHT, "icon") {}

ImageIcon::ImageIcon (const double x, const double y, const double width, const double height, const std::string& name) :
		Widget (x, y, width, height, name)
{
	originalSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, getEffectiveWidth (), getEffectiveHeight ());
}

ImageIcon::ImageIcon (const double x, const double y, const double width, const double height, const std::string& name,
					  cairo_surface_t* surface) :
		Widget (x, y, width, height, name), originalSurface (nullptr)
{
	loadImage (surface);
}

ImageIcon::ImageIcon (const double x, const double y, const double width, const double height, const std::string& name,
					  const std::string& filename) :
		Widget (x, y, width, height, name), originalSurface (nullptr)
{
	loadImage (filename);
}

ImageIcon::ImageIcon (const ImageIcon& that) :
		Widget (that)
{
	originalSurface = cairo_image_surface_clone_from_image_surface (that.originalSurface);
}

ImageIcon::~ImageIcon ()
{
	cairo_surface_destroy (originalSurface);
}

ImageIcon& ImageIcon::operator= (const ImageIcon& that)
{
	Widget::operator= (that);
	if (originalSurface) cairo_surface_destroy (originalSurface);
	originalSurface = cairo_image_surface_clone_from_image_surface (that.originalSurface);

	return *this;
}

void ImageIcon::loadImage (cairo_surface_t* surface)
{
	if (originalSurface && (cairo_surface_status (originalSurface) == CAIRO_STATUS_SUCCESS)) cairo_surface_destroy (originalSurface);
	originalSurface = cairo_image_surface_clone_from_image_surface (surface);
}

void ImageIcon::loadImage (const std::string& filename)
{
	if (originalSurface && (cairo_surface_status (originalSurface) == CAIRO_STATUS_SUCCESS)) cairo_surface_destroy (originalSurface);
	originalSurface = cairo_image_surface_create_from_png (filename.c_str());
}

void ImageIcon::draw (const double x, const double y, const double width, const double height)
{
	if ((!widgetSurface) || (cairo_surface_status (widgetSurface) != CAIRO_STATUS_SUCCESS)) return;

	if ((width_ >= 1) && (height_ >= 1))
	{
		// Draw super class widget elements first
		Widget::draw (x, y, width, height);

		double w = getEffectiveWidth ();
		double h = getEffectiveHeight ();

		if (originalSurface && (cairo_surface_status (originalSurface) == CAIRO_STATUS_SUCCESS) && (w > 0) && (h > 0))
		{
			cairo_t* cr = cairo_create (widgetSurface);
			if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
			{
				// Limit cairo-drawing area
				cairo_rectangle (cr, x, y, width, height);
				cairo_clip (cr);
				//TODO also clip to inner borders

				double oriw = cairo_image_surface_get_width (originalSurface);
				double orih = cairo_image_surface_get_height (originalSurface);
				double sz = ((w / oriw < h / orih) ? (w / oriw) : (h / orih));
				double x0 = getXOffset () + w / 2 - oriw * sz / 2;
				double y0 = getYOffset () + h / 2 - orih * sz / 2;

				cairo_scale (cr, sz, sz);
				cairo_set_source_surface(cr, originalSurface, x0, y0);
				cairo_paint (cr);
			}

			cairo_destroy (cr);
		}
	}
}

}
