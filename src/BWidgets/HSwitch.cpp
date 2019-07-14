/* HSwitch.cpp
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

#include "HSwitch.hpp"

namespace BWidgets
{
HSwitch::HSwitch () : HSwitch (0.0, 0.0, BWIDGETS_DEFAULT_HSWITCH_WIDTH, BWIDGETS_DEFAULT_HSWITCH_HEIGHT, "hswitch", BWIDGETS_DEFAULT_VALUE) {}

HSwitch::HSwitch (const double  x, const double y, const double width, const double height, const std::string& name,
				  const double defaultvalue) :
		HSlider (x, y, width, height, name, defaultvalue, 0.0, 1.0, 1.0) {}

Widget* HSwitch::clone () const {return new HSwitch (*this);}

void HSwitch::updateCoords ()
{
	double w = getEffectiveWidth ();
	double h = getEffectiveHeight ();

	knobRadius = (h < w / 2 ? h / 2 : w / 4);
	scaleX0 = getXOffset ();
	scaleY0 = getYOffset () + h / 2 - knobRadius;
	scaleWidth = w;
	scaleHeight = 2 * knobRadius;
	scaleXValue = scaleX0 + knobRadius + getRelativeValue () * (scaleWidth - 2 * knobRadius);
	knobXCenter = scaleXValue + BWIDGETS_DEFAULT_KNOB_DEPTH;
	knobYCenter = scaleY0 + scaleHeight / 2 + BWIDGETS_DEFAULT_KNOB_DEPTH;
}

}
