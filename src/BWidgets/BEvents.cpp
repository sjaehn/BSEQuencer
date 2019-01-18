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

Event::Event () : Event ((void*) nullptr, NO_EVENT) {}
Event::Event (void* widget, const EventType type) : eventWidget (widget), eventType (type) {}
void* Event::getWidget () {return eventWidget;}
EventType Event::getEventType () const {return eventType;}

/*
 * End of class BEvents::Event
 *****************************************************************************/


/*****************************************************************************
 * Class BEvents::ExposeEvent
 *****************************************************************************/

ExposeEvent::ExposeEvent () : ExposeEvent ((void*) nullptr, NO_EVENT, 0, 0, 0, 0) {}
ExposeEvent::ExposeEvent (void* widget, const EventType type, const double x, const double y, const double width, const double height) :
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

PointerEvent::PointerEvent () : PointerEvent ((void*) nullptr, NO_EVENT, 0, 0, 0, 0, NO_BUTTON) {}
PointerEvent::PointerEvent (void* widget, const EventType type, const double x, const double y, const double deltaX, const double deltaY,
							const InputDevice button) :
		Event (widget, type), xpos (x), ypos (y), deltaX (deltaX), deltaY (deltaY), buttonNr (button) {}
void PointerEvent::setX (const double x) {xpos = x;}
double PointerEvent::getX () const {return xpos;}
void PointerEvent::setY (const double y) {ypos = y;}
double PointerEvent::getY () const {return ypos;}
double PointerEvent::getDeltaX () const {return deltaX;}
double PointerEvent::getDeltaY () const {return deltaY;}
void PointerEvent::setButton (const InputDevice button) {buttonNr = button;}
InputDevice PointerEvent::getButton () const {return buttonNr;}

/*
 * End of class BEvents::PointerEvent
 *****************************************************************************/


/*****************************************************************************
 * Class BEvents::ValueChangedEvent
 *****************************************************************************/

ValueChangedEvent::ValueChangedEvent () : ValueChangedEvent ((void*) nullptr, 0.0) {}
ValueChangedEvent::ValueChangedEvent (void* widget, const double val) : Event (widget, VALUE_CHANGED_EVENT), value (val) {}
void ValueChangedEvent::setValue (const double val) {value = val;}
double ValueChangedEvent::getValue () const {return value;}

/*
 * End of class BEvents::ValueChangedEvent
 *****************************************************************************/

}
