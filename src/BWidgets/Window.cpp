/* Window.cpp
 * Copyright (C) 2018, 2019 by Sven Jähnichen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "Window.hpp"

namespace BWidgets
{

Window::Window () : Window (BWIDGETS_DEFAULT_WIDTH, BWIDGETS_DEFAULT_HEIGHT, "window", 0.0) {}

Window::Window (const double width, const double height, const std::string& title, PuglNativeWindow nativeWindow, bool resizable) :
		Widget (0.0, 0.0, width, height, title), title_ (title), view_ (NULL), nativeWindow_ (nativeWindow), quit_ (false)
{
	pointer.x = 0;
	pointer.y = 0;
	pointer.widget = nullptr;
	pointer.time = std::chrono::steady_clock::now();

	input.fill({nullptr, 0.0, 0.0});

	main_ = this;
	view_ = puglInit(NULL, NULL);

	if (nativeWindow_ != 0)
	{
		puglInitWindowParent(view_, nativeWindow_);
	}

	puglInitWindowSize (view_, width_, height_);
	puglInitResizable (view_, resizable);
	puglInitContextType (view_, PUGL_CAIRO);
	puglIgnoreKeyRepeat (view_, true);
	puglCreateWindow (view_, title.c_str ());
	puglShowWindow (view_);
	puglSetHandle (view_, this);

	puglSetEventFunc (view_, Window::translatePuglEvent);

	setBackground (BWIDGETS_DEFAULT_WINDOW_BACKGROUND);
}

Window::~Window ()
{
	purgeEventQueue ();
	puglDestroy(view_);
	main_ = nullptr;	// Important switch for the super destructor. It took
						// days of debugging ...
}

PuglView* Window::getPuglView () {return view_;}

cairo_t* Window::getPuglContext ()
{
	if (view_) return (cairo_t*) puglGetContext (view_);
	else return NULL;
}

void Window::run ()
{
	while (!quit_)
	{
		handleEvents ();
	}
}

void Window::onConfigure (BEvents::ExposeEvent* event)
{
	if (width_ != event->getWidth ()) setWidth (event->getWidth ());
	if (height_ != event->getHeight ()) setHeight (event->getHeight ());
}

void Window::onClose ()
{
	quit_ = true;
}

void Window::onExpose (BEvents::ExposeEvent* event)
{
	if (event)
	{
		// Create a temporal storage surface and store all children surfaces on this
		cairo_surface_t* storageSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width_, height_);
		redisplay (storageSurface, event->getX (), event->getY (),
				   event->getWidth (), event->getHeight ());

		// Copy storage surface onto pugl provided surface
		cairo_t* cr = main_->getPuglContext ();
		cairo_save (cr);
		cairo_set_source_surface (cr, storageSurface, 0, 0);
		cairo_paint (cr);
		cairo_restore (cr);

		cairo_surface_destroy (storageSurface);
	}
}

void Window::addEventToQueue (BEvents::Event* event)
{
	// Try to merge with precursor event
	if ((event) && (!eventQueue.empty ()) && (eventQueue.back()))
	{
		BEvents::Event* precursor = eventQueue.back ();

		// Check for mergeable events
		// EXPOSE_EVENT
		if ((event->getEventType() == BEvents::EXPOSE_EVENT) && (precursor->getEventType() == BEvents::EXPOSE_EVENT))
		{
			// Only merge if this Window allows merging (ignore children mergeable flags)
			if (isMergeable(BEvents::EXPOSE_EVENT))
			{
				BEvents::ExposeEvent* firstEvent = (BEvents::ExposeEvent*) precursor;
				BEvents::ExposeEvent* nextEvent = (BEvents::ExposeEvent*) event;

				double first_x0 = firstEvent->getX();
				double first_x1 = first_x0 + firstEvent->getWidth();
				double first_y0 = firstEvent->getY();
				double first_y1 = first_y0 + firstEvent->getHeight();

				double next_x0 = nextEvent->getX();
				double next_x1 = next_x0 + nextEvent->getWidth();
				double next_y0 = nextEvent->getY();
				double next_y1 = next_y0 + nextEvent->getHeight();

				double x0 = (first_x0 < next_x0 ? first_x0 : next_x0);
				double y0 = (first_y0 < next_y0 ? first_y0 : next_y0);
				double x1 = (first_x1 > next_x1 ? first_x1 : next_x1);
				double y1 = (first_y1 > next_y1 ? first_y1 : next_y1);

				firstEvent->setX (x0);
				firstEvent->setY (y0);
				firstEvent->setWidth (x1 - x0);
				firstEvent->setHeight (y1 - y0);

				return;
			}
		}

		// POINTER_MOTION_EVENT
		else if ((event->getEventType() == BEvents::POINTER_MOTION_EVENT) && (precursor->getEventType() == BEvents::POINTER_MOTION_EVENT))
		{
			BEvents::PointerEvent* firstEvent = (BEvents::PointerEvent*) precursor;
			BEvents::PointerEvent* nextEvent = (BEvents::PointerEvent*) event;

			if (
					(nextEvent->getWidget() == firstEvent->getWidget()) &&
					(nextEvent->getWidget()->isMergeable(BEvents::POINTER_MOTION_EVENT)))
			{
				firstEvent->setX (nextEvent->getX());
				firstEvent->setY (nextEvent->getY());
				firstEvent->setDeltaX (nextEvent->getDeltaX() + firstEvent->getDeltaX());
				firstEvent->setDeltaY (nextEvent->getDeltaY() + firstEvent->getDeltaY());

				return;
			}
		}

		// POINTER_DRAG_EVENT
		else if ((event->getEventType() == BEvents::POINTER_DRAG_EVENT) && (precursor->getEventType() == BEvents::POINTER_DRAG_EVENT))
		{
			BEvents::PointerEvent* firstEvent = (BEvents::PointerEvent*) precursor;
			BEvents::PointerEvent* nextEvent = (BEvents::PointerEvent*) event;

			if (
					(nextEvent->getWidget() == firstEvent->getWidget()) &&
					(nextEvent->getWidget()->isMergeable(BEvents::POINTER_DRAG_EVENT)) &&
					(nextEvent->getButton() == firstEvent->getButton()) &&
					(nextEvent->getXOrigin() == firstEvent->getXOrigin()) &&
					(nextEvent->getYOrigin() == firstEvent->getYOrigin())
				)
			{
				firstEvent->setX (nextEvent->getX());
				firstEvent->setY (nextEvent->getY());
				firstEvent->setDeltaX (nextEvent->getDeltaX() + firstEvent->getDeltaX());
				firstEvent->setDeltaY (nextEvent->getDeltaY() + firstEvent->getDeltaY());

				return;
			}
		}


		// WHEEL_SCROLL_EVENT
		else if ((event->getEventType() == BEvents::WHEEL_SCROLL_EVENT) && (precursor->getEventType() == BEvents::WHEEL_SCROLL_EVENT))
		{
			BEvents::WheelEvent* firstEvent = (BEvents::WheelEvent*) precursor;
			BEvents::WheelEvent* nextEvent = (BEvents::WheelEvent*) event;

			if (
					(nextEvent->getWidget() == firstEvent->getWidget()) &&
					(nextEvent->getWidget()->isMergeable(BEvents::WHEEL_SCROLL_EVENT)) &&
					(nextEvent->getX() == firstEvent->getX()) &&
					(nextEvent->getY() == firstEvent->getY())
				)
			{
				firstEvent->setDeltaX (nextEvent->getDeltaX() + firstEvent->getDeltaX());
				firstEvent->setDeltaY (nextEvent->getDeltaY() + firstEvent->getDeltaY());

				return;
			}
		}
	}

	eventQueue.push_back (event);
}

void Window::setInput (const BEvents::InputDevice device, Widget* widget, double x, double y)
{
	if ((device > BEvents::NO_BUTTON) && (device < BEvents::NR_OF_BUTTONS))
	{
		input[device].widget = widget;
		input[device].x = x;
		input[device].y = y;
	}
}

Widget* Window::getInputWidget (BEvents::InputDevice device) const
{
	if ((device > BEvents::NO_BUTTON) && (device < BEvents::NR_OF_BUTTONS)) return input[device].widget;
	else return nullptr;
}

double Window::getInputX (BEvents::InputDevice device) const
{
	if ((device > BEvents::NO_BUTTON) && (device < BEvents::NR_OF_BUTTONS)) return input[device].x;
	else return 0.0;
}

double Window::getInputY (BEvents::InputDevice device) const
{
	if ((device > BEvents::NO_BUTTON) && (device < BEvents::NR_OF_BUTTONS)) return input[device].y;
	else return 0.0;
}

void Window::setKeyGrab (Widget* widget, uint32_t key)
{
	std::vector<uint32_t> keys = {key};
	setKeyGrab (widget, keys);
}

void Window::setKeyGrab (Widget* widget, std::vector<uint32_t>& keys)
{
	if (isChild (widget))
	{
		KeyGrab newKeyGrab = {keys, widget};
		removeKeyGrab (widget);
		keyGrabStack.push_back (newKeyGrab);
	}
}

void Window::removeKeyGrab (Widget* widget)
{
	for (std::vector<KeyGrab>::iterator it = keyGrabStack.begin(); it != keyGrabStack.end(); )
	{
		KeyGrab* gr = (KeyGrab*) &it;
		if (gr->widget == widget) it = keyGrabStack.erase (it);
		else ++it;
	}
}

Widget* Window::getKeyGrabWidget (uint32_t key)
{
	for (int i = keyGrabStack.size () - 1; i >= 0; --i)
	{
		KeyGrab gr = keyGrabStack.at (i);
		for (uint32_t k : gr.keys)
		{
			if ((k == 0) || (k == key)) return gr.widget;
		}
	}

	return nullptr;
}

void Window::handleEvents ()
{
	puglProcessEvents (view_);
	translateTimeEvent ();

	while (!eventQueue.empty ())
	{
		BEvents::Event* event = eventQueue.front ();
		eventQueue.erase (eventQueue.begin ());

		if (event)
		{
			Widget* widget = event->getWidget ();
			if (widget)
			{
				BEvents::EventType eventType = event->getEventType ();

				switch (eventType)
				{
				case BEvents::CONFIGURE_EVENT:
					onConfigure ((BEvents::ExposeEvent*) event);
					break;

				case BEvents::EXPOSE_EVENT:
					onExpose ((BEvents::ExposeEvent*) event);
					break;

				case BEvents::CLOSE_EVENT:
					onClose ();
					break;

				case BEvents::KEY_PRESS_EVENT:
					{
						BEvents::KeyEvent* be = (BEvents::KeyEvent*) event;
						widget->onKeyPressed (be);
					}
					break;

				case BEvents::KEY_RELEASE_EVENT:
					{
						BEvents::KeyEvent* be = (BEvents::KeyEvent*) event;
						widget->onKeyReleased (be);
					}
					break;

				case BEvents::BUTTON_PRESS_EVENT:
					{
						BEvents::PointerEvent* be = (BEvents::PointerEvent*) event;
						setInput (be->getButton (), widget, be->getX(), be->getY());
						widget->onButtonPressed (be);
					}
					break;

				case BEvents::BUTTON_RELEASE_EVENT:
					{
						BEvents::PointerEvent* be = (BEvents::PointerEvent*) event;
						setInput (be->getButton (), nullptr, 0.0, 0.0);
						widget->onButtonReleased (be);
					}
					break;

				case BEvents::BUTTON_CLICK_EVENT:
					{
						BEvents::PointerEvent* be = (BEvents::PointerEvent*) event;
						setInput (be->getButton (), nullptr, 0.0, 0.0);
						widget->onButtonClicked (be);
					}
					break;

				case BEvents::POINTER_MOTION_EVENT:
					widget->onPointerMotion((BEvents::PointerEvent*) event);
					break;

				case BEvents::POINTER_DRAG_EVENT:
					widget->onPointerDragged((BEvents::PointerEvent*) event);
					break;

				case BEvents::WHEEL_SCROLL_EVENT:
					widget->onWheelScrolled((BEvents::WheelEvent*) event);
					break;

				case BEvents::VALUE_CHANGED_EVENT:
					widget->onValueChanged((BEvents::ValueChangedEvent*) event);
					break;

				case BEvents::FOCUS_IN_EVENT:
					widget->onFocusIn((BEvents::FocusEvent*) event);
					break;

				case BEvents::FOCUS_OUT_EVENT:
					widget->onFocusOut((BEvents::FocusEvent*) event);
					break;

				default:
					break;
				}

			}
			delete event;
		}
	}
}

void Window::translatePuglEvent (PuglView* view, const PuglEvent* event)
{
	Window* w = (Window*) puglGetHandle (view);
	if (!w) return;

	// All PUGL events cause FOCUS_OUT
	if (w->pointer.widget && w->pointer.widget->getFocusWidget() && w->pointer.widget->getFocusWidget()->isFocused())
	{
		w->addEventToQueue(new BEvents::FocusEvent (w->pointer.widget,
													BEvents::FOCUS_OUT_EVENT,
													w->pointer.x - w->pointer.widget->getOriginX (),
													w->pointer.y - w->pointer.widget->getOriginY ()));
		w->pointer.widget->getFocusWidget()->setFocused(false);
	}

	switch (event->type) {

	case PUGL_KEY_PRESS:
		{
			uint32_t key = (event->key.character != 0 ? event->key.character : event->key.special);
			Widget* widget = w->getKeyGrabWidget (key);
			w->addEventToQueue (new BEvents::KeyEvent (widget,
																								 BEvents::KEY_PRESS_EVENT,
																							 	 event->key.x,
																							 	 event->key.y,
																							 	 key));
		}
		break;

	case PUGL_KEY_RELEASE:
		{
			uint32_t key = (event->key.character != 0 ? event->key.character : event->key.special);
			Widget* widget = w->getKeyGrabWidget (key);
			w->addEventToQueue (new BEvents::KeyEvent (widget,
																								 BEvents::KEY_RELEASE_EVENT,
																							 	 event->key.x,
																							 	 event->key.y,
																							 	 key));
		}
		break;

	case PUGL_BUTTON_PRESS:
		{
			Widget* widget = w->getWidgetAt (event->button.x, event->button.y, true, true, false, false, false);
			if (widget)
			{
				w->addEventToQueue (new BEvents::PointerEvent (widget,
															  BEvents::BUTTON_PRESS_EVENT,
															  event->button.x - widget->getOriginX (),
															  event->button.y - widget->getOriginY (),
															  event->button.x - widget->getOriginX (),
															  event->button.y - widget->getOriginY (),
															  0, 0,
															  (BEvents::InputDevice) event->button.button));
			}

			w->pointer.x = event->button.x;
			w->pointer.y = event->button.y;
		}
		break;

	case PUGL_BUTTON_RELEASE:
		{
			BEvents::InputDevice device = (BEvents::InputDevice) event->button.button;
			Widget* widget = w->getInputWidget (device);
			if (widget)
			{
				double xorigin = (widget->getMainWindow() ? widget->getMainWindow()->getInputX (device) : 0);
				double yorigin = (widget->getMainWindow() ? widget->getMainWindow()->getInputY (device) : 0);
				w->addEventToQueue (new BEvents::PointerEvent (widget,
															  BEvents::BUTTON_RELEASE_EVENT,
															  event->button.x - widget->getOriginX (),
															  event->button.y - widget->getOriginY (),
															  xorigin,
															  yorigin,
															  0, 0,
															  device));


				// Also emit BUTTON_CLICK_EVENT ?
				Widget* widget2 = w->getWidgetAt (event->button.x, event->button.y, true, true, false, false, false);
				if (widget == widget2)
				{
					w->addEventToQueue (new BEvents::PointerEvent (widget,
																  BEvents::BUTTON_CLICK_EVENT,
																  event->button.x - widget->getOriginX (),
																  event->button.y - widget->getOriginY (),
																  xorigin,
																  yorigin,
																  0, 0,
																  device));
				}
			}

			w->pointer.x = event->button.x;
			w->pointer.y = event->button.y;
		}
		break;

	case PUGL_MOTION_NOTIFY:
		{
			BEvents::InputDevice device = BEvents::NO_BUTTON;
			Widget* widget = nullptr;

			// Scan for pressed buttons associated with a widget
			for (int i = BEvents::NO_BUTTON + 1; i < BEvents::NR_OF_BUTTONS; ++i)
			{
				if (w->getInputWidget ((BEvents::InputDevice) i))
				{
					device = (BEvents::InputDevice) i;
					widget = w->getInputWidget (device);
					if (widget->isDraggable ())
					{
						double xorigin = w->getInputX (device);
						double yorigin = w->getInputY (device);

						// new
						w->addEventToQueue (new BEvents::PointerEvent (widget,
																	   BEvents::POINTER_DRAG_EVENT,
																	   event->motion.x - widget->getOriginX (),
																	   event->motion.y - widget->getOriginY (),
																	   xorigin,
																	   yorigin,
																	   event->motion.x - w->pointer.x,
																	   event->motion.y - w->pointer.y,
																	   device));
					}
				}
			}

			// No button associated with a widget? Only POINTER_MOTION_EVENT
			if (device == BEvents::NO_BUTTON)
			{
				widget = w->getWidgetAt (event->motion.x, event->motion.y, true, false, false, false, false);
				if (widget)
				{
					w->addEventToQueue (new BEvents::PointerEvent (widget,
																   BEvents::POINTER_MOTION_EVENT,
																   event->motion.x - widget->getOriginX (),
																   event->motion.y - widget->getOriginY (),
																   0, 0,
																   event->motion.x - w->pointer.x,
																   event->motion.y - w->pointer.y,
																   device));
				}
			}

			w->pointer.x = event->motion.x;
			w->pointer.y = event->motion.y;
		}
		break;

	case PUGL_SCROLL:
		{
			Widget* widget = w->getWidgetAt (event->scroll.x, event->scroll.y, true, false, false, true, false);
			if (widget)
			{
				w->addEventToQueue(new BEvents::WheelEvent (widget,
															BEvents::WHEEL_SCROLL_EVENT,
															event->scroll.x - widget->getOriginX (),
															event->scroll.y - widget->getOriginY (),
															event->scroll.dx,
															event->scroll.dy));
			}
		}
		break;

	case PUGL_CONFIGURE:
		w->addEventToQueue (new BEvents::ExposeEvent (w,
													  BEvents::CONFIGURE_EVENT,
													  event->configure.x,
													  event->configure.y,
													  event->configure.width,
													  event->configure.height));
		break;

	case PUGL_EXPOSE:
		w->postRedisplay ();
		break;

	case PUGL_CLOSE:
		w->addEventToQueue (new BEvents::Event (w, BEvents::CLOSE_EVENT));
		break;

	default:
		break;
	}

	w->pointer.widget = w->getWidgetAt (w->pointer.x, w->pointer.y, true, false, false, false, true);
	w->pointer.time = std::chrono::steady_clock::now();

}

void Window::translateTimeEvent ()
{
	if (pointer.widget && pointer.widget->getFocusWidget())
	{
		FocusWidget* focusWidget = pointer.widget->getFocusWidget();
		std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();

		std::chrono::milliseconds diffMs = std::chrono::duration_cast<std::chrono::milliseconds> (nowTime - pointer.time);

		if (diffMs > focusWidget->getFocusInMilliseconds() + focusWidget->getFocusOutMilliseconds())
		{
			if (focusWidget->isFocused())
			{
				addEventToQueue(new BEvents::FocusEvent (pointer.widget,
														 BEvents::FOCUS_OUT_EVENT,
														 pointer.x - pointer.widget->getOriginX (),
														 pointer.y - pointer.widget->getOriginY ()));
				focusWidget->setFocused(false);
			}
		}
		else if (diffMs > focusWidget->getFocusInMilliseconds())
		{
			if (!focusWidget->isFocused())
			{
				addEventToQueue(new BEvents::FocusEvent (pointer.widget,
														 BEvents::FOCUS_IN_EVENT,
														 pointer.x - pointer.widget->getOriginX (),
														 pointer.y - pointer.widget->getOriginY ()));
				focusWidget->setFocused(true);
			}
		}
	}
}

void Window::purgeEventQueue (Widget* widget)
{
	for (std::vector<BEvents::Event*>::iterator it = eventQueue.begin (); it != eventQueue.end (); )
	{
		BEvents::Event* event = *it;
		if ((event) && ((widget == nullptr) || (widget == event->getWidget ())))
		{
			it = eventQueue.erase (it);
			delete event;
		}
		else ++it;
	}
}

}
