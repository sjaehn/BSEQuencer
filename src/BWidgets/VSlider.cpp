/* VSlider.cpp
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

#include "VSlider.hpp"

namespace BWidgets
{
VSlider::VSlider () : VSlider (0.0, 0.0, BWIDGETS_DEFAULT_VSLIDER_WIDTH, BWIDGETS_DEFAULT_VSLIDER_HEIGHT, "vslider",
		  	  	  	  	  	   BWIDGETS_DEFAULT_VALUE, BWIDGETS_DEFAULT_RANGE_MIN, BWIDGETS_DEFAULT_RANGE_MAX, BWIDGETS_DEFAULT_RANGE_STEP) {}

VSlider::VSlider (const double  x, const double y, const double width, const double height, const std::string& name,
				  const double value, const double min, const double max, const double step) :
		VScale (x, y, width, height, name, value, min, max, step),
		knob (0, 0, 0, 0, BWIDGETS_DEFAULT_KNOB_DEPTH, name), knobRadius (0), knobXCenter (0), knobYCenter (0)
{
	knob.setClickable (false);
	knob.setDraggable (false);
	knob.setScrollable (false);
	add (knob);
}

VSlider::VSlider (const VSlider& that) :
		VScale (that), knob (that.knob), knobRadius (that.knobRadius), knobXCenter (that.knobXCenter), knobYCenter (that.knobYCenter)
{
	add (knob);
}

VSlider::~VSlider () {}

VSlider& VSlider::operator= (const VSlider& that)
{
	release (&knob);

	knob = that.knob;
	knobRadius = that.knobRadius;
	knobXCenter = that.knobXCenter;
	knobYCenter = that.knobYCenter;
	RangeWidget::operator= (that);

	add (knob);

	return *this;
}

Widget* VSlider::clone () const {return new VSlider (*this);}

void VSlider::update ()
{
	VScale::update ();

	// Update Knob
	knob.moveTo (knobXCenter -  knobRadius, knobYCenter - knobRadius);
	knob.setWidth (2 * knobRadius);
	knob.setHeight (2 * knobRadius);
}

void VSlider::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void VSlider::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	VScale::applyTheme (theme, name);
	knob.applyTheme (theme, name);
}

void VSlider::updateCoords ()
{
	double w = getEffectiveWidth ();
	double h = getEffectiveHeight ();

	knobRadius = (w < h / 2 ? w / 2 : h / 4);
	scaleX0 = getXOffset () + w / 2 - knobRadius / 2;
	scaleY0 = getYOffset () + knobRadius;
	scaleWidth = knobRadius;
	scaleHeight = h - 2 * knobRadius;
	scaleYValue = scaleY0 + (1 - getRelativeValue ()) * scaleHeight;
	knobXCenter = scaleX0 + scaleWidth / 2;
	knobYCenter = scaleYValue;
}
}
