/* HSlider.cpp
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

#include "HSlider.hpp"

namespace BWidgets
{
HSlider::HSlider () : HSlider (0.0, 0.0, BWIDGETS_DEFAULT_HSLIDER_WIDTH, BWIDGETS_DEFAULT_HSLIDER_HEIGHT, "hslider",
		  	  	  	  	  	   BWIDGETS_DEFAULT_VALUE, BWIDGETS_DEFAULT_RANGE_MIN, BWIDGETS_DEFAULT_RANGE_MAX, BWIDGETS_DEFAULT_RANGE_STEP) {}

HSlider::HSlider (const double  x, const double y, const double width, const double height, const std::string& name,
				  const double value, const double min, const double max, const double step) :
		HScale (x, y, width, height, name, value, min, max, step),
		knob (0, 0, 0, 0, BWIDGETS_DEFAULT_KNOB_DEPTH, name), knobRadius (0), knobXCenter (0), knobYCenter (0)
{
	knob.setClickable (false);
	knob.setDragable (false);
	add (knob);
}

HSlider::HSlider (const HSlider& that) :
		HScale (that), knob (that.knob), knobRadius (that.knobRadius), knobXCenter (that.knobXCenter), knobYCenter (that.knobYCenter)
{
	add (knob);
}

HSlider::~HSlider () {}

HSlider& HSlider::operator= (const HSlider& that)
{
	release (&knob);

	knob = that.knob;
	knobRadius = that.knobRadius;
	knobXCenter = that.knobXCenter;
	knobYCenter = that.knobYCenter;
	HScale::operator= (that);

	add (knob);

	return *this;
}

void HSlider::update ()
{
	HScale::update ();

	// Update Knob
	knob.moveTo (knobXCenter - knobRadius, knobYCenter - knobRadius);
	knob.setWidth (2 * knobRadius);
	knob.setHeight (2 * knobRadius);
}

void HSlider::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void HSlider::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	HScale::applyTheme (theme, name);
	knob.applyTheme (theme, name);
}

void HSlider::updateCoords ()
{
	double w = getEffectiveWidth ();
	double h = getEffectiveHeight ();

	knobRadius = (h < w / 2 ? h / 2 : w / 4);
	scaleX0 = getXOffset () + knobRadius;
	scaleY0 = getYOffset () + h / 2 - knobRadius / 2;
	scaleWidth = w - 2 * knobRadius;
	scaleHeight = knobRadius;
	scaleXValue = scaleX0 + getRelativeValue () * scaleWidth;
	knobXCenter = scaleXValue;
	knobYCenter = scaleY0 + scaleHeight / 2;
}
}
