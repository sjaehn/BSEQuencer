/* BEvents.cpp
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

#include "BEvents.hpp"

namespace BEvents
{
/*****************************************************************************
 * Class BEvents::Event
 *****************************************************************************/

Event::Event () : Event (nullptr, NO_EVENT) {}
Event::Event (BWidgets::Widget* widget, const EventType type) : eventWidget (widget), eventType (type) {}
BWidgets::Widget* Event::getWidget () {return eventWidget;}
EventType Event::getEventType () const {return eventType;}

/*
 * End of class BEvents::Event
 *****************************************************************************/


/*****************************************************************************
 * Class BEvents::ExposeEvent
 *****************************************************************************/

ExposeEvent::ExposeEvent () : ExposeEvent (nullptr, NO_EVENT, 0, 0, 0, 0) {}
ExposeEvent::ExposeEvent (BWidgets::Widget* widget, const EventType type, const double x, const double y, const double width, const double height) :
		Event (widget, type), exposeX0 (x), exposeY0 (y), exposeHeight (height), exposeWidth (width) {}
void ExposeEvent::setX (const double x) {exposeX0 = x;}
double ExposeEvent::getX () const {return exposeX0;}
void ExposeEvent::setY (const double y) {exposeY0 = y;}
double ExposeEvent::getY () const {return exposeY0;}
void ExposeEvent::setWidth (const double width) {exposeWidth = width;}
double ExposeEvent::getWidth () const {return exposeWidth;}
void ExposeEvent::setHeight (const double height) {exposeHeight = height;}
double ExposeEvent::getHeight () const {return exposeHeight;}

/*
 * End of class BEvents::ExposeEvent
 *****************************************************************************/


/*****************************************************************************
 * Class BEvents::PointerEvent
 *****************************************************************************/

PointerEvent::PointerEvent () : PointerEvent (nullptr, NO_EVENT, 0, 0, 0, 0, 0, 0, NO_BUTTON) {}
PointerEvent::PointerEvent (BWidgets::Widget* widget, const EventType type, const double x, const double y, const double xOrigin, const double yOrigin,
							const double deltaX, const double deltaY, const InputDevice button) :
		Event (widget, type), xpos (x), ypos (y), xOrigin (xOrigin), yOrigin (yOrigin), deltaX (deltaX), deltaY (deltaY), buttonNr (button) {}
void PointerEvent::setX (const double x) {xpos = x;}
double PointerEvent::getX () const {return xpos;}
void PointerEvent::setY (const double y) {ypos = y;}
double PointerEvent::getY () const {return ypos;}
void PointerEvent::setXOrigin (const double xOrigin) {this->xOrigin = xOrigin;}
double PointerEvent::getXOrigin () const {return xOrigin;}
void PointerEvent::setYOrigin (const double yOrigin) {this->yOrigin = yOrigin;}
double PointerEvent::getYOrigin () const {return yOrigin;}
void PointerEvent::setDeltaX (const double deltaX) {this->deltaX = deltaX;}
double PointerEvent::getDeltaX () const {return deltaX;}
void PointerEvent::setDeltaY (const double deltaY) {this->deltaY = deltaY;}
double PointerEvent::getDeltaY () const {return deltaY;}
void PointerEvent::setButton (const InputDevice button) {buttonNr = button;}
InputDevice PointerEvent::getButton () const {return buttonNr;}

/*
 * End of class BEvents::PointerEvent
 *****************************************************************************/


/*****************************************************************************
 * Class BEvents::WheelEvent
 *****************************************************************************/

WheelEvent::WheelEvent () : WheelEvent (nullptr, NO_EVENT, 0, 0, 0, 0) {}
WheelEvent::WheelEvent (BWidgets::Widget* widget, const EventType type, const double x, const double y, const double deltaX, const double deltaY) :
		Event (widget, type), xpos (x), ypos (y),deltaX (deltaX), deltaY (deltaY) {}
void WheelEvent::setX (const double x) {xpos = x;}
double WheelEvent::getX () const {return xpos;}
void WheelEvent::setY (const double y) {ypos = y;}
double WheelEvent::getY () const {return ypos;}
void WheelEvent::setDeltaX (const double deltaX) {this->deltaX = deltaX;}
double WheelEvent::getDeltaX () const {return deltaX;}
void WheelEvent::setDeltaY (const double deltaY) {this->deltaY = deltaY;}
double WheelEvent::getDeltaY () const {return deltaY;}

/*
 * End of class BEvents::WheelEvent
 *****************************************************************************/


/*****************************************************************************
 * Class BEvents::ValueChangedEvent
 *****************************************************************************/

ValueChangedEvent::ValueChangedEvent () : ValueChangedEvent (nullptr, 0.0) {}
ValueChangedEvent::ValueChangedEvent (BWidgets::Widget* widget, const double val) : Event (widget, VALUE_CHANGED_EVENT), value (val) {}
void ValueChangedEvent::setValue (const double val) {value = val;}
double ValueChangedEvent::getValue () const {return value;}

/*
 * End of class BEvents::ValueChangedEvent
 *****************************************************************************/

/*****************************************************************************
 * Class BEvents::WheelEvent
 *****************************************************************************/

FocusEvent::FocusEvent () : FocusEvent (nullptr, NO_EVENT, 0, 0) {}
FocusEvent::FocusEvent (BWidgets::Widget* widget, const EventType type, const double x, const double y) : Event (widget, type), xpos (x), ypos (y) {}
void FocusEvent::setX (const double x) {xpos = x;}
double FocusEvent::getX () const {return xpos;}
void FocusEvent::setY (const double y) {ypos = y;}
double FocusEvent::getY () const {return ypos;}

/*
 * End of class BEvents::WheelEvent
 *****************************************************************************/

}
