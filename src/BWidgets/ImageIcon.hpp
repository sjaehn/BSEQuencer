/* ImageIcon.hpp
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

#ifndef IMAGEICON_HPP_
#define IMAGEICON_HPP_

#include "Widget.hpp"

namespace BWidgets
{
/**
 * Class BWidgets::ImageIcon
 *
 * Icon widget displaying an image file or a Cairo surface.
 */
class ImageIcon : public Widget
{
public:
	ImageIcon ();
	ImageIcon (const double x, const double y, const double width, const double height, const std::string& name);
	ImageIcon (const double x, const double y, const double width, const double height, const std::string& name, cairo_surface_t* surface);
	ImageIcon (const double x, const double y, const double width, const double height, const std::string& name, const std::string& filename);

	/**
	 * Creates a new (orphan) image icon widget and copies the widget
	 * properties from a source image icon widget. This method doesn't
	 * copy any parent or child widgets.
	 * @param that Source drawing surface widget
	 */
	ImageIcon (const ImageIcon& that);

	~ImageIcon ();

	/**
	 * Assignment. Copies the widget properties from a source widget and keeps
	 * its name and its position within the widget tree. Emits a
	 * BEvents::ExposeEvent if the widget is visible.
	 * @param that Source widget
	 */
	ImageIcon& operator= (const ImageIcon& that);

	/**
	 * Loads an image from a Cairo surface or an image file.
	 * @param surface	Cairo surface
	 * @param filename	Filename
	 */
	void loadImage (cairo_surface_t* surface);
	void loadImage (const std::string& filename);

protected:
	virtual void draw (const double x, const double y, const double width, const double height) override;

	cairo_surface_t* originalSurface;
};

}

#endif /* IMAGEICON_HPP_ */
