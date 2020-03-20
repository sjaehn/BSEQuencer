/* ToggleButton.hpp
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

#ifndef BWIDGETS_TOGGLEBUTTON_HPP_
#define BWIDGETS_TOGGLEBUTTON_HPP_

#include "Button.hpp"

namespace BWidgets
{
/**
 * Class BWidgets::ToggleButton
 *
 * Basic toggle button widget. Is is a BWidgets::Button and thus a
 * BWidgets::ValueWidget having two conditions: on (value != 0) or off
 * (value == 0)
 */
class ToggleButton : public Button
{
public:
	ToggleButton ();
	ToggleButton (const double x, const double y, const double width, const double height, const std::string& name, double defaultValue = 0.0);

	/**
	 * Pattern cloning. Creates a new instance of the widget and copies all
	 * its properties.
	 */
	virtual Widget* clone () const override;

	/**
	 * Handles the BEvents::BUTTON_PRESS_EVENT.
	 * @param event Pointer to a pointer event emitted by the same widget.
	 */
	virtual void onButtonPressed (BEvents::PointerEvent* event) override;

	/**
	 * Overrides the BEvents::BUTTON_RELEASED_EVENT handled by
	 * BWidgets::Button.
	 * @param event Pointer to a pointer event emitted by the same widget.
	 */
	virtual void onButtonReleased (BEvents::PointerEvent* event) override;
};

}




#endif /* BWIDGETS_TOGGLEBUTTON_HPP_ */
