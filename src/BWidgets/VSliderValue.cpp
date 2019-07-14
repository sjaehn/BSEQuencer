/* VSliderValue.cpp
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

#include "VSliderValue.hpp"

namespace BWidgets
{
VSliderValue::VSliderValue () :
		VSliderValue (0.0, 0.0, BWIDGETS_DEFAULT_VSLIDERVALUE_WIDTH, BWIDGETS_DEFAULT_VSLIDERVALUE_HEIGHT,
								 "vslidervalue",
								 BWIDGETS_DEFAULT_VALUE, BWIDGETS_DEFAULT_RANGE_MIN, BWIDGETS_DEFAULT_RANGE_MAX, BWIDGETS_DEFAULT_RANGE_STEP,
								 BWIDGETS_DEFAULT_VALUE_FORMAT) {}

VSliderValue::VSliderValue (const double x, const double y, const double width, const double height, const std::string& name,
												  const double value, const double min, const double max, const double step,
												  const std::string& valueFormat) :
	VSlider (x, y, width, height, name, value, min, max, step),
	valueDisplay(0, 0, width, height, name),
	valFormat (valueFormat), displayHeight (0), displayWidth (0), displayX0 (0), displayY0 (0)
{
	valueDisplay.setText (BValues::toBString (valueFormat, value));
	add (valueDisplay);
}

VSliderValue::VSliderValue (const VSliderValue& that) :
		VSlider (that), valueDisplay (that.valueDisplay), valFormat (that.valFormat),
		displayHeight (that.displayHeight), displayWidth (that.displayWidth), displayX0 (that.displayX0), displayY0 (that.displayY0)
{
	add (valueDisplay);
}

VSliderValue::~VSliderValue () {}

VSliderValue& VSliderValue::operator= (const VSliderValue& that)
{
	release (&valueDisplay);

	displayHeight = that.displayHeight;
	displayWidth = that.displayWidth;
	displayX0 = that.displayX0;
	displayY0 = that.displayY0;
	valFormat = that.valFormat;
	valueDisplay = that.valueDisplay;
	VSlider::operator= (that);

	add (valueDisplay);

	return *this;
}

Widget* VSliderValue::clone () const {return new VSliderValue (*this);}

void VSliderValue::setValue (const double val)
{
	VScale::setValue (val);
	valueDisplay.setText(BValues::toBString (valFormat, value));
}

void VSliderValue::setValueFormat (const std::string& valueFormat)
{
	valFormat = valueFormat;
	update ();
}

std::string VSliderValue::getValueFormat () const {return valFormat;}

Label* VSliderValue::getDisplayLabel () {return &valueDisplay;}

void VSliderValue::update ()
{
	VSlider::update ();

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

void VSliderValue::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}
void VSliderValue::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	VSlider::applyTheme (theme, name);
	valueDisplay.applyTheme (theme, name);
}

void VSliderValue::updateCoords ()
{
	double w = getEffectiveWidth ();
	double h = getEffectiveHeight ();

	displayWidth = (w < h ? w : h);
	displayHeight = displayWidth / 2.2;
	displayX0 = getXOffset () + w / 2 - displayWidth / 2;
	displayY0 = getYOffset ();

	double h2 = h - displayHeight;
	double w2 = displayWidth / 2;
	knobRadius = (w2 < h2 / 2 ? w2 / 2 : h2 / 4);
	scaleX0 = getXOffset () + w / 2 - knobRadius / 2;
	scaleY0 = getYOffset () + displayHeight + knobRadius;
	scaleWidth = knobRadius;
	scaleHeight = h2 - 2 * knobRadius;
	scaleYValue = scaleY0 + (1 - getRelativeValue ()) * scaleHeight;
	knobXCenter = scaleX0 + scaleWidth / 2;
	knobYCenter = scaleYValue;
}

}
