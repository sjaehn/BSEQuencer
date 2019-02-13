/* FocusWidget.hpp
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

#ifndef BWIDGETS_FOCUSWIDGET_HPP_
#define BWIDGETS_FOCUSWIDGET_HPP_

#include "Widget.hpp"
#include <chrono>

#define BWIDGETS_DEFAULT_FOCUS_IN_TIME 1000
#define BWIDGETS_DEFAULT_FOCUS_OUT_TIME 5000
#define BWIDGETS_DEFAULT_FOCUS_NAME "/focus"

namespace BWidgets
{

/**
 * Class BWidgets::FocusWidget
 *
 * Default widget container for focus events.
 */
class FocusWidget : public Widget
{
public:
	FocusWidget ();
	FocusWidget (Widget* hostWidget, const std::string& name);
	~FocusWidget ();

	//TODO Copy constructor

	void setFocusInMilliseconds (const std::chrono::milliseconds ms);

	std::chrono::milliseconds getFocusInMilliseconds () const;

	void setFocusOutMilliseconds (const std::chrono::milliseconds ms);

	std::chrono::milliseconds getFocusOutMilliseconds () const;

	void setFocused (const bool status);

	bool isFocused () const;

	void link (Widget* hostWidget);

	void unlink ();

protected:
	std::chrono::milliseconds focusInMs;
	std::chrono::milliseconds focusOutMs;
	bool focusEntered;
	Widget* host;
};

}

#endif /* BWIDGETS_FOCUSWIDGET_HPP_ */
