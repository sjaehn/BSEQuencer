/* DisplayDial.cpp
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

#include "DisplayDial.hpp"

namespace BWidgets
{

DisplayDial::DisplayDial () :
		DisplayDial (0.0, 0.0, BWIDGETS_DEFAULT_DISPLAYDIAL_WIDTH, BWIDGETS_DEFAULT_DISPLAYDIAL_HEIGHT,
							  "displaydial",
							  BWIDGETS_DEFAULT_VALUE, BWIDGETS_DEFAULT_RANGE_MIN, BWIDGETS_DEFAULT_RANGE_MAX, BWIDGETS_DEFAULT_RANGE_STEP,
							  BWIDGETS_DEFAULT_VALUE_FORMAT) {}

DisplayDial::DisplayDial (const double x, const double y, const double width, const double height, const std::string& name,
											const double value, const double min, const double max, const double step,
											const std::string& valueFormat) :
	Dial (x, y, width, height, name, value, min, max, step),
	valueDisplay(0, 0.75 * height, width, 0.25 * height, name),
	valFormat (valueFormat)
{
	valueDisplay.setText (BValues::toBString (valueFormat, value));
	add (valueDisplay);
}

DisplayDial::DisplayDial (const DisplayDial& that) :
		Dial (that), valueDisplay (that.valueDisplay), valFormat (that.valFormat)
{
	add (valueDisplay);
}

DisplayDial::~DisplayDial () {}

DisplayDial& DisplayDial::operator= (const DisplayDial& that)
{
	release (&valueDisplay);
	valFormat = that.valFormat;
	Dial::operator= (that);
	valueDisplay = that.valueDisplay;
	add (valueDisplay);

	return *this;
}

void DisplayDial::setValue (const double val)
{
	Dial::setValue (val);
	valueDisplay.setText(BValues::toBString (valFormat, value));
}

void DisplayDial::setValueFormat (const std::string& valueFormat)
{
	valFormat = valueFormat;
	update ();
}

std::string DisplayDial::getValueFormat () const {return valFormat;}

Label* DisplayDial::getDisplayLabel () {return &valueDisplay;}

void DisplayDial::update ()
{
	Dial::update();

	// Update display
	valueDisplay.moveTo (dialCenterX - dialRadius, dialCenterY + 0.7 * dialRadius);
	valueDisplay.setWidth (2 * dialRadius);
	valueDisplay.setHeight (0.5 * dialRadius);
	if (valueDisplay.getFont ()->getFontSize () != 0.4 * dialRadius)
	{
		valueDisplay.getFont ()->setFontSize (0.4 * dialRadius);
		valueDisplay.update ();
	}
	valueDisplay.setText (BValues::toBString (valFormat, value));

}

void DisplayDial::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}
void DisplayDial::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Dial::applyTheme (theme, name);
	valueDisplay.applyTheme (theme, name);
	update ();
}

void DisplayDial::updateCoords ()
{
	double w = getEffectiveWidth ();
	double h = getEffectiveHeight ();
	dialRadius = (w < h / 1.2 ? w / 2 : h / 2.4);
	dialCenterX = width_ / 2;
	dialCenterY = height_ / 2 - 0.2 * dialRadius;
}

}
