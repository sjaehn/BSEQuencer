/* DisplayHSlider.hpp
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

#ifndef BWIDGETS_DISPLAYHSLIDER_HPP_
#define BWIDGETS_DISPLAYHSLIDER_HPP_

#include "Label.hpp"
#include "HSlider.hpp"

#define BWIDGETS_DEFAULT_DISPLAYHSLIDER_WIDTH BWIDGETS_DEFAULT_HSLIDER_WIDTH
#define BWIDGETS_DEFAULT_DISPLAYHSLIDER_HEIGHT (BWIDGETS_DEFAULT_HSLIDER_HEIGHT + BWIDGETS_DEFAULT_LABEL_HEIGHT)

namespace BWidgets
{

/**
 * Class BWidgets::DisplayHSlider
 *
 * Composite BWidgets::HSlider widget that additionally displays the value.
 */
class DisplayHSlider : public HSlider
{
public:
	DisplayHSlider ();
	DisplayHSlider (const double x, const double y, const double width, const double height, const std::string& name,
				    const double value, const double min, const double max, const double step,
				    const std::string& valueFormat);

	/**
	 * Creates a new (orphan) slider and copies the slider properties from a
	 * source slider.
	 * @param that Source slider
	 */
	DisplayHSlider (const DisplayHSlider& that);

	~DisplayHSlider ();

	/**
	 * Assignment. Copies the slider properties from a source slider and keeps
	 * its name and its position within the widget tree. Emits an expose event
	 * if the widget is visible and a value changed event.
	 * @param that Source slider
	 */
	DisplayHSlider& operator= (const DisplayHSlider& that);

	/**
	 * Changes the value of the widget and keeps it within the defined range.
	 * Passes the value to its predefined child widgets.
	 * Emits a value changed event and (if visible) an expose event.
	 * @param val Value
	 */
	virtual void setValue (const double val) override;

	/**
	 * Sets the value output format.
	 * @valueFormat Format of the output in printf standard for type double.
	 */
	void setValueFormat (const std::string& valueFormat);

	/**
	 * Gets the value output format.
	 * @return Format of the output in printf standard for type double.
	 */
	std::string getValueFormat () const;

	/**
	 * Gets (a pointer to) the Label for direct access.
	 * @return Pointer to the label
	 */
	Label* getDisplayLabel ();

	/**
	 * Calls a redraw of the widget and calls postRedisplay () if the the
	 * Widget is visible.
	 * This method should be called if the widgets properties are indirectly
	 * changed.
	 */
	virtual void update () override;

	/**
	 * Scans theme for widget properties and applies these properties.
	 * @param theme Theme to be scanned.
	 * 				Styles used are:
	 * 				BWIDGETS_KEYWORD_BORDER
	 * 				BWIDGETS_KEYWORD_BACKGROUND
	 * 				BWIDGETS_KEYWORD_FGCOLORS
	 * 				BWIDGETS_KEYWORD_BGCOLORS
	 * 				BWIDGETS_KEYWORD_TEXTCOLORS
	 * 				BWIDGETS_KEYWORD_FONT.
	 * @param name Name of the BStyles::StyleSet within the theme to be
	 * 		  	   applied.
	 */
	virtual void applyTheme (BStyles::Theme& theme);
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name);

protected:
	virtual void updateCoords () override;

	Label valueDisplay;

	std::string valFormat;
	double displayHeight;
	double displayWidth;
	double displayX0;
	double displayY0;
};

}

#endif /* BWIDGETS_DISPLAYHSLIDER_HPP_ */
