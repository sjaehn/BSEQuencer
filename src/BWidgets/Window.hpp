/* Window.hpp
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

#ifndef BWIDGETS_WINDOW_HPP_
#define BWIDGETS_WINDOW_HPP_

// Default BWidgets::Window settings (Note: use non-transparent backgrounds only)
#define BWIDGETS_DEFAULT_WINDOW_BACKGROUND BStyles::blackFill

#include <chrono>
#include "Widget.hpp"
#include "FocusWidget.hpp"

namespace BWidgets
{

/**
 * Class BWidgets::Window
 *
 * Main window class of BWidgets. Add all other widgets (directly or
 * indirectly) to this window.
 * A BWidgets::Window is the BWidgets::Widget that is controlled by the host
 * via Pugl, receives host events via Pugl and coordinates handling of all
 * events. Configure, expose, and close events will be handled directly and
 * exclusively by this widget.
 */
class Window : public Widget
{
public:
	Window ();
	Window (const double width, const double height, const std::string& title, PuglNativeWindow nativeWindow, bool resizable = false);

	Window (const Window& that) = delete;	// Only one window in this version

	~Window ();

	Window& operator= (const Window& that) = delete;	// Only one Window in this version

	/**
	 * Gets in contact to the host system via Pugl
	 * @return Pointer to the PuglView
	 */
	PuglView* getPuglView ();

	/**
	 * Gets the Cairo context provided by the host system via Pugl
	 * @return Pointer to the Cairo context
	 */
	cairo_t* getPuglContext ();

	/**
	 * Runs the window until the close flag is set and thus it will be closed.
	 * For stand-alone applications.
	 */
	void run ();

	/**
	 * Queues an event until the next call of the handleEvents method.
	 * @param event Event
	 */
	void addEventToQueue (BEvents::Event* event);

	/**
	 * Main Event handler. Walks through the event queue and sorts the events
	 * to their respective onXXX handling methods
	 */
	void handleEvents ();

	/**
	 * Executes an reexposure of the area given by the expose event.
	 * @param event Expose event containing the widget that emitted the event
	 * 				and the area that should be reexposed.
	 */
	virtual void onExpose (BEvents::ExposeEvent* event) override;

	/**
	 * Predefined empty method to handle a BEvents::EventType::CONFIGURE_EVENT.
	 * BEvents::EventType::CONFIGURE_EVENTs will only be handled by
	 * BWidget::Window.
	 */
	virtual void onConfigure (BEvents::ExposeEvent* event) override;

	/**
	 * Sets the close flag and thus ends the run method.
	 */
	virtual void onClose () override;

	/*
	 * Links or unlinks a mouse button to a widget.
	 * @param device	Button
	 * @param widget	Pointer to the widget to be linked or nullptr to unlink
	 * @param x			X position relative to the widgets origin where the button
	 * 					was pressed
	 * @param y			Y position relative to the widgets origin where the button
	 * 					was pressed
	 */
	void setInput (const BEvents::InputDevice device, Widget* widget, double x, double y);

	/*
	 * Gets the links from mouse button to a widget.
	 * @param device Button
	 * @return Pointer to the linked widget or nullptr
	 */
	Widget* getInputWidget (BEvents::InputDevice device) const;

	/*
	 * Gets the button press position relative to the widgets origin
	 * @param device	Button
	 * @return			X position
	 */
	double getInputX (BEvents::InputDevice device) const;

	/*
	 * Gets the button press position relative to the widgets origin
	 * @param device	Button
	 * @return			Y position
	 */
	double getInputY (BEvents::InputDevice device) const;

	/*
	 * Adds (or replaces) a widget to the top of the KeyGrab stack and associates
	 * this widget with the given key(s). The top widget of the stack associated
	 * with the key(s) pressed or released will emit the respective
	 * BEvents::KeyEvent.
	 * @param widget	Widget that will emit the BEvents::KeyEvent.
	 * @param key			0 for all keys (default) or unicode of the key
	 * 								(or BEvent::KeyCode) or
	 * @param keys		vector of unicodes
	 */
	void setKeyGrab (Widget* widget, uint32_t key = 0);
	void setKeyGrab (Widget* widget, std::vector<uint32_t>& keys);

	/*
	 * Removes a widget (and its associated keys) from the KeyGrab stack.
	 * @param widget	Widget to remove.
	 */
	void removeKeyGrab (Widget* widget);

	/*
	 * Removes events (emited by a given widget) from the event queue
	 * @param widget	Emitting widget (nullptr for all widgets)
	 */
	void purgeEventQueue (Widget* widget = nullptr);

protected:

	/**
	 * Communication interface to the host via Pugl. Translates PuglEvents to
	 * BEvents::Event derived objects.
	 */
	static void translatePuglEvent (PuglView* view, const PuglEvent* event);
	void translateTimeEvent ();

	void mergeEvents ();

	/* Gets the widget that is resposible for emitting BEvents::KeyEvent's if
	 * the respective key has been pressed or released.
	 * @param key		Unicode of the key (or BEvent::KeyCode).
	 * @return			Responsible widget or nullptr.
	 */
	Widget* getKeyGrabWidget (uint32_t key);

	std::string title_;
	PuglView* view_;
	PuglNativeWindow nativeWindow_;
	bool quit_;

	typedef struct {
		double x;
		double y;
		Widget* widget;
		std::chrono::steady_clock::time_point time;
	} Pointer;

	Pointer pointer;

	typedef struct
	{
		Widget* widget;
		double x;
		double y;
	} Input;

	/**
	 * Stores either nullptr or (a pointer to) the widget that emitted the
	 * BEvents::BUTTON_PRESS_EVENT until a BEvents::BUTTON_RELEASE_EVENT or
	 * the linked widget is released or destroyed.
	 */
	std::array<Input, BEvents::InputDevice::NR_OF_BUTTONS> input;

	typedef struct
	{
		std::vector<uint32_t> keys;
		Widget* widget;
	} KeyGrab;

	std::vector<KeyGrab> keyGrabStack;

	std::vector<BEvents::Event*> eventQueue;
};

}



#endif /* BWIDGETS_WINDOW_HPP_ */
