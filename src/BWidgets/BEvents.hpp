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

#include <cstdint>
#include <string>


namespace BWidgets
{
class Widget;	// Forward declaration
}

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
enum EventType {
	CONFIGURE_EVENT,
	EXPOSE_EVENT,
	CLOSE_EVENT,
	KEY_PRESS_EVENT,
	KEY_RELEASE_EVENT,
	BUTTON_PRESS_EVENT,
	BUTTON_RELEASE_EVENT,
	BUTTON_CLICK_EVENT,
	POINTER_MOTION_EVENT,
	POINTER_DRAG_EVENT,
	WHEEL_SCROLL_EVENT,
	VALUE_CHANGED_EVENT,
	FOCUS_IN_EVENT,
	FOCUS_OUT_EVENT,
	NO_EVENT
};

enum KeyCode {
	KEY_F1 = 0xE000,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_LEFT,
	KEY_UP,
	KEY_RIGHT,
	KEY_DOWN,
	KEY_PAGE_UP,
	KEY_PAGE_DOWN,
	KEY_HOME,
	KEY_END,
	KEY_INSERT,
	KEY_SHIFT,
	KEY_CTRL,
	KEY_ALT,
	KEY_SUPER
};

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
	Event (BWidgets::Widget* widget, const EventType type);

	/**
	 * Gets a pointer to the widget which caused the event.
	 * @return Pointer to the widget
	 */
	BWidgets::Widget* getWidget ();

	/**
	 * Gets the type of the event
	 * @return Event type
	 */
	EventType getEventType () const;

protected:
	BWidgets::Widget* eventWidget;
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
	ExposeEvent (BWidgets::Widget* widget, const EventType type, const double x, const double y, const double width, const double heigth);

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
 * Class BEvents::KeyEvent
 *
 * Key events are emitted by the system if a key is pressed or released.
 */
 class KeyEvent : public Event
 {
 public:
	 KeyEvent ();
	 KeyEvent (BWidgets::Widget* widget, const EventType type, const double x, const double y, const uint32_t unicode);

	 /**
	  * Redefines the x coordinate of the key event
	  * @param x X coordinate relative to the widgets origin
	  */
	 void setX (const double x);

	 /**
	  * Gets the x coordinate of the key event
	  * @return X coordinate relative to the widgets origin
	  */
	 double getX () const;

	 /**
	  * Redefines the y coordinate of the key event
	  * @param y y coordinate relative to the widgets origin
	  */
	 void setY (const double y);

	 /**
	  * Gets the y coordinate of the key event
	  * @return Y coordinate relative to the widgets origin
	  */
	 double getY () const;

	 /**
	  * Gets the key that caused of the key event
	  * @return Unicode of the key
	  */
	 uint32_t getKey () const;

	 std::string getKeyUTF8 () const;

protected:
	double xpos;
	double ypos;
	uint32_t key;
 };
 /*
  * End of class BEvents::KeyEvent
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
	PointerEvent (BWidgets::Widget* widget, const EventType type, const double x, const double y, const double xOrigin, const double yOrigin,
				  const double deltaX, const double deltaY, const InputDevice button);

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

	/**
	 * Redefines the x coordinate of the position where the button was
	 * initially pressed
	 * @param xOrigin X coordinate relative to the widgets origin
	 */
	void setXOrigin (const double xOrigin);

	/**
	 * Gets the x coordinate of the pointer position where the respective
	 * button was initially pressed. The returned value is the same as getX ()
	 * for BUTTON_PRESS_EVENTs, 0.0 for POINTER_MOTION_EVENTs.
	 * @return X coordinate relative to the widgets origin
	 */
	double getXOrigin () const;

	/**
	 * Redefines the y coordinate of the position where the button was
	 * initially pressed
	 * @param yOrigin Y coordinate relative to the widgets origin
	 */
	void setYOrigin (const double xOrigin);

	/**
	 * Gets the y coordinate of the pointer position where the respective
	 * button was initially pressed. The returned value is the same as getY ()
	 * for BUTTON_PRESS_EVENTs, 0.0 for POINTER_MOTION_EVENTs.
	 * @return Y coordinate relative to the widgets origin
	 */
	double getYOrigin () const;

	/**
	 * Redefines the pointers x movement
	 * @param deltaX Movement of the pointer on x axis
	 */
	void setDeltaX (const double deltaX);

	/**
	 * Gets the x movement (relative to the last PointerEvent)
	 * @return Change in x coordinate
	 */
	double getDeltaX () const;

	/**
	 * Redefines the pointers y movement
	 * @param deltaY Movement of the pointer on y axis
	 */
	void setDeltaY (const double deltaY);

	/**
	 * Gets the y movement (relative to the last PointerEvent)
	 * @return Change in y coordinate
	 */
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
	double xOrigin;
	double yOrigin;
	double deltaX;
	double deltaY;
	InputDevice buttonNr;
};
/*
 * End of class BEvents::PointerEvent
 *****************************************************************************/

/**
 * Class BEvents::WheelEvent
 *
 * Wheel events are emitted by the system (via pugl and the main window) if
 * a (mouse) wheel is turned.
 * The wheel event contains data about the relative change of the wheel and
 * about the pointer position (relative to the respective widget.
 * Wheel events will be handled by the respective widget and can be
 * redirected to external callback functions.
 */
class WheelEvent : public Event
{
public:
	WheelEvent ();
	WheelEvent (BWidgets::Widget* widget, const EventType type, const double x, const double y, const double deltaX, const double deltaY);

	/**
	 * Redefines the pointers x coordinate
	 * @param x X coordinate relative to the widgets origin
	 */
	void setX (const double x);

	/**
	 * Gets the pointers x coordinate of the wheel event
	 * @return X coordinate relative to the widgets origin
	 */
	double getX () const;

	/**
	 * Redefines the pointers y coordinate
	 * @param y y coordinate relative to the widgets origin
	 */
	void setY (const double y);

	/**
	 * Gets the pointers y coordinate
	 * @return Y coordinate relative to the widgets origin
	 */
	double getY () const;

	/**
	 * Redefines the wheels x movement
	 * @param deltaX Movement of the wheel on x axis
	 */
	void setDeltaX (const double deltaX);

	/**
	 * Gets the x movement of the wheel
	 * @return Change in x coordinate
	 */
	double getDeltaX () const;

	/**
	 * Redefines the wheels y movement
	 * @param deltaY Movement of the wheel on y axis
	 */
	void setDeltaY (const double deltaY);

	/**
	 * Gets the y movement of the wheel
	 * @return Change in y coordinate
	 */
	double getDeltaY () const;

protected:
	double xpos;
	double ypos;
	double deltaX;
	double deltaY;
};
/*
 * End of class BEvents::WheelEvent
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
	ValueChangedEvent (BWidgets::Widget* widget, const double val);

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

/**
 * Class BEvents::FocusEvent
 *
 * Focus events are emitted by widgets if the pointer rests for a predefined
 * time over the widget
 */
class FocusEvent : public Event
{
public:
	FocusEvent ();
	FocusEvent (BWidgets::Widget* widget, const EventType type, const double x, const double y);

	/**
	 * Redefines the pointers x coordinate
	 * @param x X coordinate relative to the widgets origin
	 */
	void setX (const double x);

	/**
	 * Gets the pointers x coordinate of the wheel event
	 * @return X coordinate relative to the widgets origin
	 */
	double getX () const;

	/**
	 * Redefines the pointers y coordinate
	 * @param y y coordinate relative to the widgets origin
	 */
	void setY (const double y);

	/**
	 * Gets the pointers y coordinate
	 * @return Y coordinate relative to the widgets origin
	 */
	double getY () const;

protected:
	double xpos;
	double ypos;
};
/*
 * End of class BEvents::ValueChangedEvent
 *****************************************************************************/
}

#endif /* BEVENTS_HPP_ */
