/* BEvents.hpp
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

#ifndef BEVENTS_HPP_
#define BEVENTS_HPP_

#include <cstdint>
#include <string>
#include "BUtilities.hpp"

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
enum InputDevice
{
	NO_BUTTON	= 0,
	LEFT_BUTTON	= 1,
	MIDDLE_BUTTON	= 2,
	RIGHT_BUTTON	= 3,
	NR_OF_BUTTONS	= 4
};

/**
 * Enumeration of event types
 */
enum EventType
{
	CONFIGURE_REQUEST_EVENT,
	EXPOSE_REQUEST_EVENT,
	CLOSE_REQUEST_EVENT,
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
	MESSAGE_EVENT,
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
protected:
	BWidgets::Widget* eventWidget;
	EventType eventType;

public:
	Event () :
		Event (nullptr, NO_EVENT) {}
	Event (BWidgets::Widget* widget, const EventType type) :
		eventWidget (widget), eventType (type) {}

	/**
	 * Gets a pointer to the widget which caused the event.
	 * @return Pointer to the widget
	 */
	BWidgets::Widget* getWidget ()
	{return eventWidget;}

	/**
	 * Gets the type of the event
	 * @return Event type
	 */
	EventType getEventType () const
	{return eventType;}

};
/*
 * End of class BEvents::Event
 *****************************************************************************/


 /**
  * Class BEvents::WidgetEvent
  *
  * Widget events are emitted by an (event) widget if the widget is requested
  * by an other (request) widget. This event class is typically used if opening
  * or closing of a window or an request widget is requested.
  */
 class WidgetEvent : public Event
 {
 protected:
 	BWidgets::Widget* requestWidget;

 public:
 	WidgetEvent () :
 		WidgetEvent (nullptr, nullptr, NO_EVENT) {}
 	WidgetEvent (BWidgets::Widget* eventWidget, BWidgets::Widget* requestWidget, const EventType type) :
 		Event (eventWidget, type),  requestWidget (requestWidget) {}

 	/**
 	 * Gets a pointer to the widget which request the event.
 	 * @return Pointer to the widget
 	 */
 	BWidgets::Widget* getRequestWidget ()
 	{return requestWidget;}

 };
 /*
  * End of class BEvents::WidgetEvent
  *****************************************************************************/


/**
 * Class BEvents::ExposeEvent
 *
 * Expose events are emitted by a parent event widget (or window) if the visual
 * output of a child (request) widget is requested to be
 * updated. An expose event additionally contains the coordinates (x, y, width
 * and height) of the output region (relative to the widgets origin) that should
 * be updated.
 */
class ExposeEvent : public WidgetEvent
{
protected:
	double exposeX0;
	double exposeY0;
	double exposeWidth;
	double exposeHeight;

public:
	ExposeEvent () :
		ExposeEvent (nullptr, nullptr, NO_EVENT, 0, 0, 0, 0) {};
	ExposeEvent (BWidgets::Widget* eventWidget, BWidgets::Widget* requestWidget, const EventType type,
		     const double x, const double y, const double width, const double height) :
		WidgetEvent (eventWidget, requestWidget, type), exposeX0 (x), exposeY0 (y), exposeWidth (width), exposeHeight (height) {}

	/**
	 * Redefines the x coordinate of the output region for the expose event
	 * @param x X coordinate relative to the widgets origin
	 */
	void setX (const double x)
	{exposeX0 = x;}

	/**
	 * Gets the x coordinate of the output region for the expose event
	 * @return X coordinate relative to the widgets origin
	 */
	double getX () const
	{return exposeX0;}

	/**
	 * Redefines the y coordinate of the output region for the expose event
	 * @param y Y coordinate relative to the widgets origin
	 */
	void setY (const double y)
	{exposeY0 = y;}

	/**
	 * Gets the y coordinate of the output region for the expose event
	 * @return Y coordinate relative to the widgets origin
	 */
	double getY () const
	{return exposeY0;}

	/**
	 * Redefines the width of the output region for the expose event
	 * @param width Width of the output region
	 */
	void setWidth (const double width)
	{exposeWidth = width;}

	/**
	 * Gets the width of the output region for the expose event
	 * @return Width of the output region
	 */
	double getWidth () const
	{return exposeWidth;}

	/**
	 * Redefines the height of the output region for the expose event
	 * @param height Height of the output region
	 */
	void setHeight (const double height)
	{exposeHeight = height;}

	/**
	 * Gets the height of the output region for the expose event
	 * @return Height of the output region
	 */
	double getHeight () const
	{return exposeHeight;}
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
 protected:
	double xpos;
	double ypos;
	uint32_t key;

 public:
	 KeyEvent () :
	 	KeyEvent (nullptr, NO_EVENT, 0, 0, 0) {}
	 KeyEvent (BWidgets::Widget* widget, const EventType type, const double x, const double y, const uint32_t unicode) :
			Event (widget, type), xpos (x), ypos (y), key (unicode) {}

	 /**
	  * Redefines the x coordinate of the key event
	  * @param x X coordinate relative to the widgets origin
	  */
	 void setX (const double x)
	 {xpos = x;}

	 /**
	  * Gets the x coordinate of the key event
	  * @return X coordinate relative to the widgets origin
	  */
	 double getX () const
	 {return xpos;}

	 /**
	  * Redefines the y coordinate of the key event
	  * @param y y coordinate relative to the widgets origin
	  */
	 void setY (const double y)
	 {ypos = y;}

	 /**
	  * Gets the y coordinate of the key event
	  * @return Y coordinate relative to the widgets origin
	  */
	 double getY () const
	 {return ypos;}

	 /**
	  * Gets the key that caused of the key event
	  * @return Unicode of the key
	  */
	 uint32_t getKey () const
	 {return key;}

	 std::string getKeyUTF8 () const
	 {
		 // Invalide unicode
		 if (key > 0x0010ffff) return "";

		 std::string s = "";

		 // 7 bit ASCII: utf-8 = unicode
		 if (key < 0x80) s += char (key);

		 // 2/3/4(/5/6) byte utf-8
		 else
		 {
			 uint32_t steps = 2;
			 for (uint32_t i = 3; i <= 6; ++i)
			 {
				 if (key >= (uint32_t (2) << (5 * (i - 1)))) steps = i;
			 }

			 char c = char ((0xFF & (0xFF << (8 - steps))) | (key >> (6 * (steps - 1))));
			 s += c;

			 for (uint32_t i = steps - 1; i >= 1; --i)
			 {
				 char c = char (0x80 | ((key >> (6 * (i - 1))) & 0x3f));
				 s += c;
			 }
		 }

		 return s;
	 }
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
protected:
	double xpos;
	double ypos;
	double xOrigin;
	double yOrigin;
	double deltaX;
	double deltaY;
	InputDevice buttonNr;

public:
	PointerEvent () :
		PointerEvent (nullptr, NO_EVENT, 0, 0, 0, 0, 0, 0, NO_BUTTON) {}
	PointerEvent (BWidgets::Widget* widget, const EventType type,
			const double x, const double y, const double xOrigin, const double yOrigin,
			const double deltaX, const double deltaY, const InputDevice button) :
		Event (widget, type), xpos (x), ypos (y), xOrigin (xOrigin), yOrigin (yOrigin),
		deltaX (deltaX), deltaY (deltaY), buttonNr (button)
	{}

	/**
	 * Redefines the x coordinate of the pointer event
	 * @param x X coordinate relative to the widgets origin
	 */
	void setX (const double x)
	{xpos = x;}

	/**
	 * Gets the x coordinate of the pointer event
	 * @return X coordinate relative to the widgets origin
	 */
	double getX () const
	{return xpos;}

	/**
	 * Redefines the y coordinate of the pointer event
	 * @param y y coordinate relative to the widgets origin
	 */
	void setY (const double y)
	{ypos = y;}

	/**
	 * Gets the y coordinate of the pointer event
	 * @return Y coordinate relative to the widgets origin
	 */
	double getY () const
	{return ypos;}

	/**
	 * Redefines the x coordinate of the position where the button was
	 * initially pressed
	 * @param xOrigin X coordinate relative to the widgets origin
	 */
	void setXOrigin (const double xOrigin)
	{this->xOrigin = xOrigin;}

	/**
	 * Gets the x coordinate of the pointer position where the respective
	 * button was initially pressed. The returned value is the same as getX ()
	 * for BUTTON_PRESS_EVENTs, 0.0 for POINTER_MOTION_EVENTs.
	 * @return X coordinate relative to the widgets origin
	 */
	double getXOrigin () const
	{return xOrigin;}

	/**
	 * Redefines the y coordinate of the position where the button was
	 * initially pressed
	 * @param yOrigin Y coordinate relative to the widgets origin
	 */
	void setYOrigin (const double xOrigin)
	{this->yOrigin = yOrigin;}

	/**
	 * Gets the y coordinate of the pointer position where the respective
	 * button was initially pressed. The returned value is the same as getY ()
	 * for BUTTON_PRESS_EVENTs, 0.0 for POINTER_MOTION_EVENTs.
	 * @return Y coordinate relative to the widgets origin
	 */
	double getYOrigin () const
	{return yOrigin;}

	/**
	 * Redefines the pointers x movement
	 * @param deltaX Movement of the pointer on x axis
	 */
	void setDeltaX (const double deltaX)
	{this->deltaX = deltaX;}

	/**
	 * Gets the x movement (relative to the last PointerEvent)
	 * @return Change in x coordinate
	 */
	double getDeltaX () const
	{return deltaX;}

	/**
	 * Redefines the pointers y movement
	 * @param deltaY Movement of the pointer on y axis
	 */
	void setDeltaY (const double deltaY)
	{this->deltaY = deltaY;}

	/**
	 * Gets the y movement (relative to the last PointerEvent)
	 * @return Change in y coordinate
	 */
	double getDeltaY () const
	{return deltaY;}

	/**
	 * Redefines the button pressed of the pointer event
	 * @param button Button pressed
	 */
	void setButton (const InputDevice button)
	{buttonNr = button;}

	/**
	 * Gets the button pressed of the pointer event
	 * @return Button pressed
	 */
	InputDevice getButton () const
	{return buttonNr;}
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
protected:
	double xpos;
	double ypos;
	double deltaX;
	double deltaY;

public:
	WheelEvent () :
		WheelEvent (nullptr, NO_EVENT, 0, 0, 0, 0) {}
	WheelEvent (BWidgets::Widget* widget, const EventType type, const double x, const double y, const double deltaX, const double deltaY) :
		Event (widget, type), xpos (x), ypos (y),deltaX (deltaX), deltaY (deltaY) {}

	/**
	 * Redefines the pointers x coordinate
	 * @param x X coordinate relative to the widgets origin
	 */
	void setX (const double x)
	{xpos = x;}

	/**
	 * Gets the pointers x coordinate of the wheel event
	 * @return X coordinate relative to the widgets origin
	 */
	double getX () const
	{return xpos;}

	/**
	 * Redefines the pointers y coordinate
	 * @param y y coordinate relative to the widgets origin
	 */
	void setY (const double y)
	{ypos = y;}

	/**
	 * Gets the pointers y coordinate
	 * @return Y coordinate relative to the widgets origin
	 */
	double getY () const
	{return ypos;}

	/**
	 * Redefines the wheels x movement
	 * @param deltaX Movement of the wheel on x axis
	 */
	void setDeltaX (const double deltaX)
	{this->deltaX = deltaX;}

	/**
	 * Gets the x movement of the wheel
	 * @return Change in x coordinate
	 */
	double getDeltaX () const
	{return deltaX;}

	/**
	 * Redefines the wheels y movement
	 * @param deltaY Movement of the wheel on y axis
	 */
	void setDeltaY (const double deltaY)
	{this->deltaY = deltaY;}

	/**
	 * Gets the y movement of the wheel
	 * @return Change in y coordinate
	 */
	double getDeltaY () const
	{return deltaY;}
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
protected:
	double value;

public:
	ValueChangedEvent () :
		ValueChangedEvent (nullptr, 0.0) {}
	ValueChangedEvent (BWidgets::Widget* widget, const double val) :
		Event (widget, VALUE_CHANGED_EVENT), value (val) {}

	/**
	 * Redefines the value exposed by the event. This method doesn't change the
	 * value within a widget!
	 * @param val New value
	 */
	void setValue (const double val)
	{value = val;}

	/**
	 * Gets the value exposed by the event
	 * @return Value of the event
	 */
	double getValue () const
	{return value;}
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
protected:
	double xpos;
	double ypos;

public:
	FocusEvent () :
		FocusEvent (nullptr, NO_EVENT, 0, 0) {}
	FocusEvent (BWidgets::Widget* widget, const EventType type, const double x, const double y) :
		Event (widget, type), xpos (x), ypos (y) {}

	/**
	 * Redefines the pointers x coordinate
	 * @param x X coordinate relative to the widgets origin
	 */
	void setX (const double x)
	{xpos = x;}

	/**
	 * Gets the pointers x coordinate of the wheel event
	 * @return X coordinate relative to the widgets origin
	 */
	double getX () const
	{return xpos;}

	/**
	 * Redefines the pointers y coordinate
	 * @param y y coordinate relative to the widgets origin
	 */
	void setY (const double y)
	{ypos = y;}

	/**
	 * Gets the pointers y coordinate
	 * @return Y coordinate relative to the widgets origin
	 */
	double getY () const
	{return ypos;}
};
/*
 * End of class BEvents::ValueChangedEvent
 *****************************************************************************/

 /**
  * Class BEvents::MessageEvent
  *
  * Ubiquitous event type
  */
class MessageEvent : public Event
{
protected:
	std::string messageName;
	BUtilities::Any messageContent;

public:
	MessageEvent () :
		MessageEvent (nullptr, "", BUtilities::Any ()) {}
	MessageEvent (BWidgets::Widget* widget, const std::string& name, const BUtilities::Any& content) :
		Event (widget, MESSAGE_EVENT), messageName (name), messageContent (content) {}

	void setName (const std::string& name)
	{messageName = name;}

	std::string getName () const
	{return messageName;}

	void setContent (const BUtilities::Any& content)
	{messageContent = content;}

	BUtilities::Any getContent () const
	{return messageContent;}
};

}

#endif /* BEVENTS_HPP_ */
