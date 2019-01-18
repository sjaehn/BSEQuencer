/* DisplayHSlider.cpp
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

#include "DisplayHSlider.hpp"

namespace BWidgets
{
DisplayHSlider::DisplayHSlider () :
		DisplayHSlider (0.0, 0.0, BWIDGETS_DEFAULT_DISPLAYHSLIDER_WIDTH, BWIDGETS_DEFAULT_DISPLAYHSLIDER_HEIGHT,
								 "displayhslider",
								 BWIDGETS_DEFAULT_VALUE, BWIDGETS_DEFAULT_RANGE_MIN, BWIDGETS_DEFAULT_RANGE_MAX, BWIDGETS_DEFAULT_RANGE_STEP,
								 BWIDGETS_DEFAULT_VALUE_FORMAT) {}

DisplayHSlider::DisplayHSlider (const double x, const double y, const double width, const double height, const std::string& name,
												  const double value, const double min, const double max, const double step,
												  const std::string& valueFormat) :
	HSlider (x, y, width, height, name, value, min, max, step),
	valueDisplay(0, 0, width, height / 2, name),
	valFormat (valueFormat), displayHeight (0), displayWidth (0), displayX0 (0), displayY0 (0)
{
	valueDisplay.setText (BValues::toBString (valueFormat, value));
	add (valueDisplay);
}

DisplayHSlider::DisplayHSlider (const DisplayHSlider& that) :
		HSlider (that), valueDisplay (that.valueDisplay), valFormat (that.valFormat),
		displayHeight (that.displayHeight), displayWidth (that.displayWidth), displayX0 (that.displayX0), displayY0 (that.displayY0)
{
	add (valueDisplay);
}

DisplayHSlider::~DisplayHSlider () {}

DisplayHSlider& DisplayHSlider::operator= (const DisplayHSlider& that)
{
	release (&valueDisplay);

	displayHeight = that.displayHeight;
	displayWidth = that.displayWidth;
	displayX0 = that.displayX0;
	displayY0 = that.displayY0;
	valFormat = that.valFormat;
	valueDisplay = that.valueDisplay;
	HSlider::operator= (that);

	add (valueDisplay);

	return *this;
}

void DisplayHSlider::setValue (const double val)
{
	RangeWidget::setValue (val);
	valueDisplay.setText(BValues::toBString (valFormat, value));
}

void DisplayHSlider::setValueFormat (const std::string& valueFormat) {valFormat = valueFormat;}
std::string DisplayHSlider::getValueFormat () const {return valFormat;}
Label* DisplayHSlider::getDisplayLabel () {return &valueDisplay;}

void DisplayHSlider::update ()
{
	HSlider::update ();

	// Update display
	valueDisplay.moveTo (displayX0, displayY0);
	valueDisplay.setWidth (displayWidth);
	valueDisplay.setHeight (displayHeight);
	if (valueDisplay.getFont ()->getFontSize () != displayHeight * 0.8)
	{
		valueDisplay.getFont ()->setFontSize (displayHeight * 0.8);
		valueDisplay.update ();
	}
	valueDisplay.setText (BValues::toBString (valFormat, value));
}

void DisplayHSlider::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}
void DisplayHSlider::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	HSlider::applyTheme (theme, name);
	valueDisplay.applyTheme (theme, name);
}

void DisplayHSlider::updateCoords ()
{
	double w = getEffectiveWidth ();
	double h = getEffectiveHeight () / 2;

	knobRadius = (h < w / 2 ? h / 2 : w / 4);
	scaleX0 = getXOffset () + knobRadius;
	scaleY0 = getYOffset () + h + knobRadius / 2;
	scaleWidth = w - 2 * knobRadius;
	scaleHeight = knobRadius;
	scaleXValue = scaleX0 + getRelativeValue () * scaleWidth;

	knobXCenter = scaleXValue;
	knobYCenter = scaleY0 + scaleHeight / 2;

	displayHeight = knobRadius * 2;
	displayWidth = 2.2 * displayHeight;
	displayY0 = getYOffset () + h - displayHeight;
	displayX0 = LIMIT (scaleXValue - displayWidth / 2, getXOffset (), getXOffset () + getEffectiveWidth () - displayWidth);
}

}
