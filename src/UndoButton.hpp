/* B.SEQuencer
 * MIDI Step Sequencer LV2 Plugin
 *
 * Copyright (C) 2018, 2019  Sven Jähnichen
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

#ifndef UNDOBUTTON_HPP_
#define UNDOBUTTON_HPP_

#include <cmath>
#include "HoverButton.hpp"

class UndoButton : public HoverButton
{
public:
        UndoButton () : UndoButton (0.0, 0.0, BWIDGETS_DEFAULT_BUTTON_WIDTH, BWIDGETS_DEFAULT_BUTTON_HEIGHT, "button", 0.0) {}
	UndoButton (const double x, const double y, const double width, const double height, const std::string& name, double defaultValue = 0.0) :
                HoverButton (x, y, width, height, name, "Undo", defaultValue) {}

protected:
        virtual void draw (const BUtilities::RectArea& area) override
        {

        	if ((!widgetSurface_) || (cairo_surface_status (widgetSurface_) != CAIRO_STATUS_SUCCESS)) return;

        	if ((getWidth() >= 6) && (getHeight() >= 6))
        	{
        		// Draw super class widget elements first
        		Button::draw (area);

        		cairo_t* cr = cairo_create (widgetSurface_);
        		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
        		{
                                // Limit cairo-drawing area
                		cairo_rectangle (cr, area.getX (), area.getY (), area.getWidth (), area.getHeight ());
                		cairo_clip (cr);

                		double x0 = getXOffset ();
                		double y0 = getYOffset ();
                		double w = getEffectiveWidth ();
                		double h = getEffectiveHeight ();

                                cairo_set_line_width (cr, 1.0);
                                cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::grey));

                                cairo_arc_negative (cr, x0 + 0.5 * w, y0 + 0.5 * h, 0.333 * w, 0.75 * M_PI, - 0.75 * M_PI);
                                cairo_rel_line_to (cr, w / 8, - h / 16);
                                cairo_rel_line_to (cr, - w / 16, - h / 16);
                                cairo_rel_line_to (cr, - w / 16, h / 8);

                                cairo_stroke (cr);
                        }
                        cairo_destroy (cr);
                }
        }
};

#endif /* UNDOBUTTON_HPP_ */
