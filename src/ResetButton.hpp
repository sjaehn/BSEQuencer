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

#ifndef RESETBUTTON_HPP_
#define RESETBUTTON_HPP_

#include <cmath>
#include "BWidgets/Button.hpp"

class ResetButton : public BWidgets::Button
{
public:
        ResetButton () : ResetButton (0.0, 0.0, BWIDGETS_DEFAULT_BUTTON_WIDTH, BWIDGETS_DEFAULT_BUTTON_HEIGHT, "button", 0.0) {}
	ResetButton (const double x, const double y, const double width, const double height, const std::string& name, double defaultValue = 0.0) :
                Button (x, y, width, height, name, defaultValue) {}

protected:
        virtual void draw (const double x, const double y, const double width, const double height) override
        {

        	if ((!widgetSurface) || (cairo_surface_status (widgetSurface) != CAIRO_STATUS_SUCCESS)) return;

        	if ((width_ >= 6) && (height_ >= 6))
        	{
        		// Draw super class widget elements first
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

                                cairo_set_line_width (cr, 0.0);
                                cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::grey));

                                cairo_arc (cr, x0 + 0.5 * w, y0 + 0.5 * h, 0.4 * w, 0, 2 * M_PI);
                                cairo_close_path (cr);
                                cairo_fill (cr);

                                cairo_set_line_width (cr, 1.5);
                                cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::black));

                                cairo_arc_negative (cr, x0 + 0.5 * w, y0 + 0.5 * h, 0.25 * w, 1.333 * M_PI, - 0.333 * M_PI);
                                cairo_rel_line_to (cr, w / 16, h / 8);
                                cairo_rel_line_to (cr, w / 16, - h / 16);
                                cairo_rel_line_to (cr, - w / 8, -h / 16);

                                cairo_stroke (cr);
                        }
                        cairo_destroy (cr);
                }
        }
};

#endif /* RESETBUTTON_HPP_ */
