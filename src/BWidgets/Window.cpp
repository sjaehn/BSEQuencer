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
		Widget* widget = (Widget*) event->getWidget ();

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

void Window::handleEvents ()
{
	puglProcessEvents (view_);
	translateTimeEvent ();

	while (!eventQueue.empty ())
	{
		BEvents::Event* event = eventQueue.front ();
		if (event)
		{
			Widget* widget = (Widget*) event->getWidget ();
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
		eventQueue.erase (eventQueue.begin ());
	}
}

void Window::translatePuglEvent (PuglView* view, const PuglEvent* event)
{
	Window* w = (Window*) puglGetHandle (view);
	if (!w) return;

	// All PUGL events cause FOCUS_OUT
	if (w->pointer.widget && w->pointer.widget->getFocusWidget() && w->pointer.widget->getFocusWidget()->isFocused())
	{
		w->addEventToQueue(new BEvents::FocusEvent ((void*)w->pointer.widget, BEvents::FOCUS_OUT_EVENT, w->pointer.x, w->pointer.y));
		w->pointer.widget->getFocusWidget()->setFocused(false);
	}

	switch (event->type) {
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
			Widget* widget = w->getWidgetAt (event->button.x, event->button.y, true, false, false, true, false);
			if (widget)
			{
				w->addEventToQueue(new BEvents::WheelEvent (widget,
															BEvents::WHEEL_SCROLL_EVENT,
															event->scroll.x,
															event->scroll.y,
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
				addEventToQueue(new BEvents::FocusEvent (pointer.widget, BEvents::FOCUS_OUT_EVENT, pointer.x, pointer.y));
				focusWidget->setFocused(false);
			}
		}
		else if (diffMs > focusWidget->getFocusInMilliseconds())
		{
			if (!focusWidget->isFocused())
			{
				addEventToQueue(new BEvents::FocusEvent (pointer.widget, BEvents::FOCUS_IN_EVENT, pointer.x, pointer.y));
				focusWidget->setFocused(true);
			}
		}
	}
}

void Window::purgeEventQueue ()
{
	while (!eventQueue.empty ())
	{
		BEvents::Event* event = eventQueue.back ();
		if (event) delete event;
		eventQueue.pop_back ();
	}
}

}
