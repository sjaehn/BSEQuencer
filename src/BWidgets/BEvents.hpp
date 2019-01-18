/* BEvents.hpp
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

#ifndef BEVENTS_HPP_
#define BEVENTS_HPP_

#include <stdint.h>

namespace BEvents
{

//TODO switch toward three pointer event handling states: pass, handle, block

/**
 * Enumeration of mouse buttons as input device for event handling
 */
typedef enum{
	NO_BUTTON		= 0,
	LEFT_BUTTON		= 1,
	MIDDLE_BUTTON	= 2,
	RIGHT_BUTTON	= 3,
	NR_OF_BUTTONS	= 4
} InputDevice;

/**
 * Enumeration of event types
 */
typedef enum {
	CONFIGURE_EVENT,
	EXPOSE_EVENT,
	CLOSE_EVENT,
	BUTTON_PRESS_EVENT,
	BUTTON_RELEASE_EVENT,
	POINTER_MOTION_EVENT,
	POINTER_MOTION_WHILE_BUTTON_PRESSED_EVENT,
	VALUE_CHANGED_EVENT,
	NO_EVENT
} EventType;

/**
 * Class BEvents::Event
 *
 * Main class of events. Only contains the event type and (a pointer to) the
 * widget which caused the event. All other event classes are derived from this
 * class.
 */
class Event
{
public:
	Event ();
	Event (void* widget, const EventType type);

	/**
	 * Gets a pointer to the widget which caused the event.
	 * @return Pointer to the widget
	 */
	void* getWidget ();

	/**
	 * Gets the type of the event
	 * @return Event type
	 */
	EventType getEventType () const;

protected:
	void* eventWidget;
	EventType eventType;
};
/*
 * End of class BEvents::Event
 *****************************************************************************/


/**
 * Class BEvents::ExposeEvent
 *
 * Expose events are emitted (e.g., by a widget) if the visual output should be
 * updated. An expose event additionally contains the coordinates (x, y, width
 * and height) of the output region (relative to the widgets origin) that should
 * be updated. Expose events will be handled only by the respective main window
 * (a BWidgets::Window).
 */
class ExposeEvent : public Event
{
public:
	ExposeEvent ();
	ExposeEvent (void* widget, const EventType type, const double x, const double y, const double width, const double heigth);

	/**
	 * Redefines the x coordinate of the output region for the expose event
	 * @param x X coordinate relative to the widgets origin
	 */
	void setX (const double x);

	/**
	 * Gets the x coordinate of the output region for the expose event
	 * @return X coordinate relative to the widgets origin
	 */
	double getX () const;

	/**
	 * Redefines the y coordinate of the output region for the expose event
	 * @param y Y coordinate relative to the widgets origin
	 */
	void setY (const double y);

	/**
	 * Gets the y coordinate of the output region for the expose event
	 * @return Y coordinate relative to the widgets origin
	 */
	double getY () const;

	/**
	 * Redefines the width of the output region for the expose event
	 * @param width Width of the output region
	 */
	void setWidth (const double width);

	/**
	 * Gets the width of the output region for the expose event
	 * @return Width of the output region
	 */
	double getWidth () const;

	/**
	 * Redefines the height of the output region for the expose event
	 * @param height Height of the output region
	 */
	void setHeight (const double height);

	/**
	 * Gets the height of the output region for the expose event
	 * @return Height of the output region
	 */
	double getHeight () const;

protected:
	double exposeX0;
	double exposeY0;
	double exposeWidth;
	double exposeHeight;
};
/*
 * End of class BEvents::ExposeEvent
 *****************************************************************************/


/**
 * Class BEvents::PointerEvent
 *
 * Pointer events are emitted by the system (via pugl and the main window) if
 * buttons are pressed or released and/or the pointer is moved over a widget.
 * The pointer event contains data about the position (relative to the
 * respective widget and the button pressed (or not).
 * Pointer events will be handled by the respective widget and can be
 * redirected to external callback functions.
 */
class PointerEvent : public Event
{
public:
	PointerEvent ();
	PointerEvent (void* widget, const EventType type, const double x, const double y, const double deltaX, const double deltaY,
				  const InputDevice button);

	/**
	 * Redefines the x coordinate of the pointer event
	 * @param x X coordinate relative to the widgets origin
	 */
	void setX (const double x);

	/**
	 * Gets the x coordinate of the pointer event
	 * @return X coordinate relative to the widgets origin
	 */
	double getX () const;

	/**
	 * Redefines the y coordinate of the pointer event
	 * @param y y coordinate relative to the widgets origin
	 */
	void setY (const double y);

	/**
	 * Gets the y coordinate of the pointer event
	 * @return Y coordinate relative to the widgets origin
	 */
	double getY () const;

	double getDeltaX () const;

	double getDeltaY () const;

	/**
	 * Redefines the button pressed of the pointer event
	 * @param button Button pressed
	 */
	void setButton (const InputDevice button);

	/**
	 * Gets the button pressed of the pointer event
	 * @return Button pressed
	 */
	InputDevice getButton () const;

protected:
	double xpos;
	double ypos;
	double deltaX;
	double deltaY;
	InputDevice buttonNr;
};
/*
 * End of class BEvents::PointerEvent
 *****************************************************************************/


/**
 * Class BEvents::ValueChangedEvent
 *
 * Value changed events are emitted by widgets (namely BWidgets::ValueWidget)
 * if their setValue method is called. The event additionally exposes the
 * changed value (that should also be accessible via
 * BWidgets::ValueWidget::getValue ()). Value changed events can be handled
 * internally (e.g., by composite widgets) and can also be redirected to
 * external callback functions.
 */
class ValueChangedEvent : public Event
{
public:
	ValueChangedEvent ();
	ValueChangedEvent (void* widget, const double val);

	/**
	 * Redefines the value exposed by the event. This method doesn't change the
	 * value within a widget!
	 * @param val New value
	 */
	void setValue (const double val);

	/**
	 * Gets the value exposed by the event
	 * @return Value of the event
	 */
	double getValue () const;

protected:
	double value;
};
/*
 * End of class BEvents::ValueChangedEvent
 *****************************************************************************/
}

#endif /* BEVENTS_HPP_ */
