/* B.SEQuencer
 * MIDI Step Sequencer LV2 Plugin
 *
 * Copyright (C) 2018, 2019  Sven Jähnichen
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

#ifndef HOVERBUTTON_HPP_
#define HOVERBUTTON_HPP_

#include <cmath>
#include "BWidgets/Button.hpp"
#include "BWidgets/Focusable.hpp"
#include "BWidgets/Label.hpp"

class HoverButton : public BWidgets::Button, public BWidgets::Focusable
{
protected:
        BWidgets::Label focusLabel;

public:
        HoverButton () : HoverButton (0.0, 0.0, BWIDGETS_DEFAULT_BUTTON_WIDTH, BWIDGETS_DEFAULT_BUTTON_HEIGHT, "button", "", 0.0) {}
	HoverButton (const double x, const double y, const double width, const double height, const std::string& name,
                     const std::string& hovertext, double defaultValue = 0.0) :
                Button (x, y, width, height, name, defaultValue),
                Focusable (std::chrono::milliseconds (BWIDGETS_DEFAULT_FOCUS_IN_MS),
			   std::chrono::milliseconds (BWIDGETS_DEFAULT_FOCUS_OUT_MS)),
                focusLabel (0, 0, 40, 20, name + "/focus", hovertext)
        {
                focusLabel.setStacking (BWidgets::STACKING_OVERSIZE);
        	focusLabel.resize ();
        	focusLabel.hide ();
        	add (focusLabel);
        }

        virtual void onFocusIn (BEvents::FocusEvent* event) override
        {
        	if (event && event->getWidget())
        	{
                        raiseToTop();
        		BUtilities::Point pos = event->getPosition();
        		focusLabel.moveTo (pos.x - 0.5 * focusLabel.getWidth(), pos.y - focusLabel.getHeight());
        		focusLabel.show();
        	}
        	Widget::onFocusIn (event);
        }
        virtual void onFocusOut (BEvents::FocusEvent* event) override
        {
        	if (event && event->getWidget()) focusLabel.hide();
        	Widget::onFocusOut (event);
        }

        virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override
	{
		Button::applyTheme (theme, name);
                focusLabel.applyTheme (theme, name + "/focus");
                focusLabel.resize ();
	}

	virtual void applyTheme (BStyles::Theme& theme) override
	{
		applyTheme (theme, name_);
	}
};

#endif /* HOVERBUTTON_HPP_ */
