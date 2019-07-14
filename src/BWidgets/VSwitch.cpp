/* VSwitch.cpp
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

#include "VSwitch.hpp"

namespace BWidgets
{
VSwitch::VSwitch () : VSwitch (0.0, 0.0, BWIDGETS_DEFAULT_VSWITCH_WIDTH, BWIDGETS_DEFAULT_VSWITCH_HEIGHT, "vswitch", BWIDGETS_DEFAULT_VALUE) {}

VSwitch::VSwitch (const double  x, const double y, const double width, const double height, const std::string& name,
				  const double defaultvalue) :
		VSlider (x, y, width, height, name, defaultvalue, 0.0, 1.0, 1.0) {}

Widget* VSwitch::clone () const {return new VSwitch (*this);}

void VSwitch::updateCoords ()
{
	double w = getEffectiveWidth ();
	double h = getEffectiveHeight ();

	knobRadius = (w < h / 2 ? w / 2 : h / 4);
	scaleX0 = getXOffset () + w / 2 - knobRadius;
	scaleY0 = getYOffset ();
	scaleWidth = 2 * knobRadius;
	scaleHeight = h;
	scaleYValue = scaleY0 + knobRadius + (1 - getRelativeValue ()) * (scaleHeight - 2 * knobRadius);

	knobXCenter = scaleX0 + scaleWidth / 2 + BWIDGETS_DEFAULT_KNOB_DEPTH;
	knobYCenter = scaleYValue + BWIDGETS_DEFAULT_KNOB_DEPTH;
}

}
