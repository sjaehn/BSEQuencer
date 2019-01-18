/* Button.cpp
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

#include "Button.hpp"

namespace BWidgets
{
Button::Button () : Button (0.0, 0.0, BWIDGETS_DEFAULT_BUTTON_WIDTH, BWIDGETS_DEFAULT_BUTTON_HEIGHT, "button", 0.0) {}

Button::Button (const double x, const double y, const double width, const double height, const std::string& name, double defaultValue) :
		ValueWidget (x, y, width, height, name, defaultValue),
		bgColors (BWIDGETS_DEFAULT_BGCOLORS)
{
	setClickable (true);
}

Button::Button (const Button& that) : ValueWidget (that), bgColors (that.bgColors) {}

Button:: ~Button () {}

Button& Button::operator= (const Button& that)
{
	bgColors = that.bgColors;
	ValueWidget::operator= (that);
	return *this;
}

void Button::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void Button::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);

	void* bgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_BGCOLORS);
	if (bgPtr)
	{
		bgColors = *((BColors::ColorSet*) bgPtr);
		update ();
	}
}

void Button::onButtonPressed (BEvents::PointerEvent* event)
{
	setValue (1.0);
	Widget::cbfunction[BEvents::EventType::BUTTON_PRESS_EVENT] (event);
}

void Button::onButtonReleased (BEvents::PointerEvent* event)
{
	setValue (0.0);
	Widget::cbfunction[BEvents::EventType::BUTTON_RELEASE_EVENT] (event);
}

void Button::draw (const double x, const double y, const double width, const double height)
{
	if ((!widgetSurface) || (cairo_surface_status (widgetSurface) != CAIRO_STATUS_SUCCESS)) return;

	if ((width_ >= 6) && (height_ >= 6))
	{
		// Draw super class widget elements first
		Widget::draw (x, y, width, height);

		cairo_t* cr = cairo_create (widgetSurface);
		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			// Limit cairo-drawing area
			cairo_rectangle (cr, x, y, width, height);
			cairo_clip (cr);

			BColors::Color butColorLo = *bgColors.getColor (getState ()); butColorLo.applyBrightness (BWIDGETS_DEFAULT_NORMALLIGHTED);
			BColors::Color butColorHi = *bgColors.getColor (getState ()); butColorHi.applyBrightness (BWIDGETS_DEFAULT_ILLUMINATED);
			BColors::Color butColorMid = *bgColors.getColor (getState ()); butColorMid.applyBrightness ((BWIDGETS_DEFAULT_NORMALLIGHTED));
			BColors::Color butColorSh = *bgColors.getColor (getState ()); butColorSh.applyBrightness (BWIDGETS_DEFAULT_SHADOWED);


			double xb, yb, wb, hb;
			double xf, yf, wf, hf;

			cairo_set_line_width (cr, 0.0);

			if (value)
			{
				xb = 0.5 + BWIDGETS_DEFAULT_BUTTON_DEPTH;
				yb = 0.5 + BWIDGETS_DEFAULT_BUTTON_DEPTH;
				hf = 0.0;
			}
			else
			{
				xb = 0.5;
				yb = 0.5;
				hf = BWIDGETS_DEFAULT_BUTTON_DEPTH;
			}

			wb = width_ - 1;
			hb = height_ - 1 - BWIDGETS_DEFAULT_BUTTON_DEPTH;
			xf = xb;
			yf = yb + hb;
			wf = wb;

			// Button top
			cairo_set_source_rgba (cr, butColorMid.getRed (), butColorMid.getGreen (), butColorMid.getBlue (), butColorMid.getAlpha ());
			cairo_rectangle (cr, xb, yb, wb, hb);
			cairo_fill (cr);


			// Button front
			cairo_move_to (cr, xb, yf);
			cairo_line_to (cr, xb + wb, yf);
			cairo_line_to (cr, xb + wb, yb);
			cairo_line_to (cr, xb + wb + hf, yb + hf);
			cairo_line_to (cr, xb + wb + hf, yf + hf);
			cairo_line_to (cr, xb + hf, yf + hf);
			cairo_close_path (cr);
			cairo_set_source_rgba (cr, butColorHi.getRed (), butColorHi.getGreen (), butColorHi.getBlue (), butColorHi.getAlpha ());
			cairo_fill (cr);

			// Button edges
			cairo_set_source_rgba (cr, butColorSh.getRed (), butColorSh.getGreen (), butColorSh.getBlue (), butColorSh.getAlpha ());
			cairo_set_line_width (cr, 0.2 * BWIDGETS_DEFAULT_BUTTON_DEPTH);
			cairo_move_to (cr, xb, yb + hb);
			cairo_line_to (cr, xb, yb);
			cairo_line_to (cr, xb + wb, yb);
			cairo_stroke (cr);

			cairo_set_source_rgba (cr, butColorHi.getRed (), butColorHi.getGreen (), butColorHi.getBlue (), butColorHi.getAlpha ());
			cairo_move_to (cr, xb, yb + hb);
			cairo_line_to (cr, xb + wb, yb + hb);
			cairo_line_to (cr, xb + wb, yb);
			cairo_move_to (cr, xb + wb, yf);
			cairo_line_to (cr, xb + wb + hf, yf + hf);
			cairo_stroke (cr);

		}
		cairo_destroy (cr);
	}
}

}
