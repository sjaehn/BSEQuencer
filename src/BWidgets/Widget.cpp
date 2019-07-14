/* Widget.cpp
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

#include "Widget.hpp"
#include "Window.hpp"
#include "FocusWidget.hpp"

namespace BWidgets
{

Widget::Widget () : Widget (0.0, 0.0, BWIDGETS_DEFAULT_WIDTH, BWIDGETS_DEFAULT_HEIGHT, "widget") {}

Widget::Widget (const double x, const double y, const double width, const double height) : Widget (x, y, width, height, "widget") {}

Widget::Widget(const double x, const double y, const double width, const double height, const std::string& name) :
		extensionData (nullptr), x_ (x), y_ (y), width_ (width), height_ (height), visible (true), clickable (true), draggable (false),
		scrollable (false), focusable (false), focusWidget (nullptr),
		main_ (nullptr), parent_ (nullptr), children_ (), border_ (BWIDGETS_DEFAULT_BORDER), background_ (BWIDGETS_DEFAULT_BACKGROUND),
		name_ (name), widgetState (BWIDGETS_DEFAULT_STATE)
{
	mergeable.fill (false);
	mergeable[BEvents::EXPOSE_EVENT] = true;
	mergeable[BEvents::POINTER_MOTION_EVENT] = true;
	mergeable[BEvents::POINTER_DRAG_EVENT] = true;
	mergeable[BEvents::WHEEL_SCROLL_EVENT] = true;
	cbfunction.fill (Widget::defaultCallback);
	cbfunction[BEvents::EventType::POINTER_DRAG_EVENT] = Widget::dragAndDropCallback;
	cbfunction[BEvents::EventType::FOCUS_IN_EVENT] = Widget::focusInCallback;
	cbfunction[BEvents::EventType::FOCUS_OUT_EVENT] = Widget::focusOutCallback;
	widgetSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
	id = (long) this;
}

Widget::Widget (const Widget& that) :
		extensionData (that.extensionData), x_ (that.x_), y_ (that.y_), width_ (that.width_), height_ (that.height_),
		visible (that.visible), clickable (that.clickable), draggable (that.draggable), scrollable (that.scrollable),
		focusable (that.focusable), focusWidget (nullptr), mergeable (that.mergeable),
		main_ (nullptr), parent_ (nullptr), children_ (), border_ (that.border_), background_ (that.background_), name_ (that.name_),
		cbfunction (that.cbfunction), widgetState (that.widgetState)
{
	widgetSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, that.width_, that.height_);
	id = (long) this;
}

Widget::~Widget()
{
	// Hide widget first (prevents filling the event stack with superfluous expose
	// events from released child widgets)
	hide ();

	// Release children
	while (!children_.empty ())
	{
		Widget* w = children_.back ();
		release (w);

		// Hard kick out if release failed
		if (w == children_.back ()) children_.pop_back ();
	}

	// Release from parent (and main) if still linked
	if (parent_) parent_->release (this);

	cairo_surface_destroy (widgetSurface);
}

Widget& Widget::operator= (const Widget& that)
{
	extensionData = that.extensionData;
	x_ = that.x_;
	y_ = that.y_;
	width_ = that.width_;
	height_ = that.height_;
	visible = that.visible;
	clickable = that.clickable;
	draggable = that.draggable;
	scrollable = that.scrollable;
	focusable = that.focusable;
	focusWidget = nullptr;
	mergeable = that.mergeable;
	border_ = that.border_;
	background_ = that.background_;
	cbfunction = that.cbfunction;
	widgetState = that.widgetState;

	if (widgetSurface) cairo_surface_destroy (widgetSurface);
	widgetSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, that.width_, that.height_);
	update ();
	return *this;
}

Widget* Widget::clone () const {return new Widget (*this);}

void Widget::show ()
{
	visible = true;

	if (isVisible ())
	{
		// (Re-)draw children as they may become visible too
		std::vector<Widget*> queue = getChildrenAsQueue ();
		for (Widget* w : queue)
		{
			if (w->isVisible ()) w->draw (0, 0, w->width_, w->height_);
		}

		// (Re-)draw this widget and post redisplay
		update ();
	}
}

void Widget::hide ()
{
	bool wasVisible = isVisible ();
	visible = false;
	if (wasVisible && parent_) parent_->postRedisplay ();
}

void Widget::add (Widget& child)
{
	// Check if already added? -> Release first
	if (child.parent_) child.parent_->release (&child);

	child.main_ = main_;
	child.parent_ = this;

	children_.push_back (&child);

	// Link all children of child to main_ and update children of child as
	// they may become visible too
	if (main_)
	{
		std::vector<Widget*> queue = child.getChildrenAsQueue ();
		for (Widget* w : queue)
		{
			w->main_ = main_;
			/*if (w->isVisible ())*/ w->update ();
		}
	}

	// (Re-)draw child widget and post redisplay
	if (child.isVisible ()) child.update ();
}

void Widget::release (Widget* child)
{
	if (child)
	{
		//std::cout << "Release " << child->name_ << ":" << &(*child) << "\n";
		bool wasVisible = child->isVisible ();

		// Delete child's connection to this widget
		child->parent_ = nullptr;

		// Release child from main window and from main windows input connections
		if (child->main_)
		{
			for (int i = (int) BEvents::NO_BUTTON; i < (int) BEvents::NR_OF_BUTTONS; ++i)
			{
				if (child->main_->getInputWidget ((BEvents::InputDevice) i) == child)
				{
					child->main_->setInput ((BEvents::InputDevice) i, nullptr, 0.0, 0.0);
				}
			}

			child->main_->purgeEventQueue (child);
			child->main_->removeKeyGrab (child);
			child->main_ = nullptr;
		}

		// And the same for all children of child
		std::vector<Widget*> queue = child->getChildrenAsQueue ();
		for (Widget* w : queue)
		{
			if (w->main_)
			{
				for (int i = (int) BEvents::NO_BUTTON; i < (int) BEvents::NR_OF_BUTTONS; ++i)
				{
					if (w->main_-> getInputWidget ((BEvents::InputDevice) i) == w)
					{
						w->main_->setInput ((BEvents::InputDevice) i, nullptr, 0.0, 0.0);
					}
				}

				w->main_->purgeEventQueue (w);
				w->main_->removeKeyGrab (w);
				w->main_ = nullptr;
			}
		}

		// Delete connection to released child
		for (std::vector<Widget*>::iterator it = children_.begin (); it !=children_.end (); ++it)
		{
			if ((Widget*) *it == child)
			{
				children_.erase (it);
				if (wasVisible) postRedisplay ();
				return;
			}
		}

		std::cerr << "Msg from BWidgets::Widget::release(): Child " << child->name_ << ":" << child->id << " is not a child of "
				  << name_ << ":" << id << std::endl;
	}
}

void Widget::moveTo (const double x, const double y)
{
	if ((x_ != x) || (y_ != y))
	{
		x_ = x;
		y_ = y;
		if (isVisible () && parent_) parent_->postRedisplay ();
	}
}

double Widget::getX () const {return x_;}

double Widget::getY () const {return y_;}

double Widget::getOriginX ()
{
	double x = 0.0;
	for (Widget* w = this; w->parent_; w = w->parent_) x += w->x_;
	return x;
}

double Widget::getOriginY ()
{
	double y = 0.0;
	for (Widget* w = this; w->parent_; w = w->parent_) y += w->y_;
	return y;
}

void Widget::moveFrontwards ()
{
	if (parent_)
	{
		int size = parent_->children_.size ();
		for (int i = 0; (i + 1) < size; ++i)
		{
			if (parent_->children_[i] == this)
			{
				// Swap
				Widget* w = parent_->children_[i + 1];
				parent_->children_[i + 1] = parent_->children_[i];
				parent_->children_[i] = w;

				if (parent_->isVisible ()) parent_->postRedisplay ();
				return;
			}
		}
	}
}

void Widget::moveBackwards ()
{
	if (parent_)
	{
		int size = parent_->children_.size ();
		for (int i = 1; i < size; ++i)
		{
			if (parent_->children_[i] == this)
			{
				// Swap
				Widget* w = parent_->children_[i];
				parent_->children_[i] = parent_->children_[i - 1];
				parent_->children_[i - 1] = w;

				if (parent_->isVisible ()) parent_->postRedisplay ();
				return;
			}
		}
	}
}

void Widget::moveToTop ()
{
	if (parent_)
	{
		// Delete old connection from parent to this widget
		for (std::vector<Widget*>::iterator it = parent_->children_.begin (); it !=parent_->children_.end (); ++it)
		{
			if ((Widget*) *it == this)
			{
				parent_->children_.erase (it);
				break;
			}
		}
		parent_->children_.push_back (this);

		if (parent_->isVisible ()) parent_->postRedisplay ();
	}
}

void Widget::setWidth (const double width)
{
	if (width_ != width)
	{
		width_ =  width;
		cairo_surface_destroy (widgetSurface);	// destroy old surface first
		widgetSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width_, height_);
		update ();
		if (isVisible () && parent_) parent_->postRedisplay ();
	}
}

double Widget::getWidth () const {return width_;}

void Widget::setHeight (const double height)
{
	if (height_ != height)
	{
		height_ = height;
		cairo_surface_destroy (widgetSurface);	// destroy old surface first
		widgetSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width_, height_);
		update ();
		if (isVisible () && parent_) parent_->postRedisplay ();
	}
}

void Widget::resize ()
{
	double height = 0.0;
	double width = 0.0;

	for (Widget* w : children_)
	{
		if (w->getX () + w->getWidth() > width) width = w->getX () + w->getWidth();
		if (w->getY () + w->getHeight() > height) height = w->getY () + w->getHeight();
	}
	resize (width, height);
}

void Widget::resize (const double width, const double height)
{
	if ((width_ != width) || (height_ != height))
	{
		width_ =  width;
		height_ = height;
		cairo_surface_destroy (widgetSurface);	// destroy old surface first
		widgetSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width_, height_);
		update ();
		if (isVisible () && parent_) parent_->postRedisplay ();
	}
}

double Widget::getHeight () const {return height_;}

void Widget::setState (const BColors::State state)
{
	widgetState = state;
	update ();
}

BColors::State Widget::getState () const {return widgetState;}

void Widget::setBorder (const BStyles::Border& border)
{
	border_ = border;
	update ();
}

BStyles::Border* Widget::getBorder () {return &border_;}

void Widget::setBackground (const BStyles::Fill& background)
{
	background_ = background;
	update ();
}

BStyles::Fill* Widget::getBackground () {return &background_;}

Window* Widget::getMainWindow () const {return main_;}

Widget* Widget::getParent () const {return parent_;}

bool Widget::hasChildren () const {return (children_.size () > 0 ? true : false);}

bool Widget::isChild (Widget* child)
{
	for (Widget* w : children_)
	{
		if (w == child) return true;
		if ((!w->children_.empty()) && w->isChild (child)) return true;
	}

	return false;
}

std::vector<Widget*> Widget::getChildren () const {return children_;}

void  Widget::setFocusWidget (FocusWidget* widget) {focusWidget = widget;}

FocusWidget*  Widget::getFocusWidget () {return focusWidget;}

void Widget::rename (const std::string& name) {name_ = name;}

std::string Widget::getName () const {return name_;}

void Widget::setCallbackFunction (const BEvents::EventType eventType, const std::function<void (BEvents::Event*)>& callbackFunction)
{
	if (eventType <= BEvents::EventType::NO_EVENT) cbfunction[eventType] = callbackFunction;
}

bool Widget::isVisible()
{
	Widget* w;
	for (w = this; w; w = w->parent_)				// Go backwards in widget tree until nullptr
	{
		if (!w->visible || !main_) return false;	// widget invisible? -> break as invisible
		if (w == main_) return true;				// main reached ? -> visible
	}
	return false;									// nullptr reached ? -> not connected to main -> invisible
}

void Widget::setClickable (const bool status) {clickable = status;}

bool Widget::isClickable () const {return clickable;}

void Widget::setDraggable (const bool status) {draggable = status;}

bool Widget::isDraggable () const {return draggable;}

void Widget::setScrollable (const bool status) {scrollable = status;}

bool Widget::isScrollable () const {return scrollable;}

void Widget::setFocusable (const bool status) {focusable = status;}

bool Widget::isFocusable () const {return focusable;}

void Widget::setMergeable (const BEvents::EventType eventType, const bool status) {mergeable[eventType] = status;}

bool Widget::isMergeable (const BEvents::EventType eventType) const {return mergeable[eventType];}

void Widget::update ()
{
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

bool Widget::isPointInWidget (const double x, const double y) const {return ((x >= 0.0) && (x <= width_) && (y >= 0.0) && (y <= height_));}

Widget* Widget::getWidgetAt (const double x, const double y, const bool checkVisibility, const bool checkClickability,
							 const bool checkDraggability, const bool checkScrollability, const bool checkFocusability)
{
	if (main_ &&
	    isPointInWidget (x, y) &&
	    ((!checkVisibility) || visible))
	{
		Widget* finalw = ((((!checkVisibility) || visible) &&
						   ((!checkClickability) || clickable) &&
						   ((!checkDraggability) || draggable) &&
						   ((!checkScrollability) || scrollable) &&
				   	   	   ((!checkFocusability) || focusable)) ?
						  this :
						  nullptr);
		for (Widget* w : children_)
		{
			if (w)
			{
				double xNew = x - w->x_;
				double yNew = y - w->y_;

				Widget* nextw = nullptr;
				if (filter (w))
				{
					nextw = w->getWidgetAt (xNew, yNew, checkVisibility,
								checkClickability, checkDraggability,
								checkScrollability, checkFocusability);
				}

				if (nextw)
				{
					finalw = nextw;
				}
			}
		}
		return finalw;
	}

	else return nullptr;
}

void Widget::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void Widget::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	// Border
	void* borderPtr = theme.getStyle(name, BWIDGETS_KEYWORD_BORDER);
	if (borderPtr) setBorder (*((BStyles::Border*) borderPtr));

	// Background
	void* backgroundPtr = theme.getStyle(name, BWIDGETS_KEYWORD_BACKGROUND);
	if (backgroundPtr) setBackground (*((BStyles::Fill*) backgroundPtr));

	if (borderPtr || backgroundPtr)
	{
		update ();
	}
}

void Widget::onConfigure (BEvents::ExposeEvent* event) {} // Empty, only Windows handle configure events
void Widget::onExpose (BEvents::ExposeEvent* event) {} // Empty, only Windows handle expose events
void Widget::onClose () {} // Empty, only Windows handle close events
void Widget::onKeyPressed (BEvents::KeyEvent* event) {cbfunction[BEvents::EventType::KEY_PRESS_EVENT] (event);}
void Widget::onKeyReleased (BEvents::KeyEvent* event) {cbfunction[BEvents::EventType::KEY_RELEASE_EVENT] (event);}
void Widget::onButtonPressed (BEvents::PointerEvent* event) {cbfunction[BEvents::EventType::BUTTON_PRESS_EVENT] (event);}
void Widget::onButtonReleased (BEvents::PointerEvent* event) {cbfunction[BEvents::EventType::BUTTON_RELEASE_EVENT] (event);}
void Widget::onButtonClicked (BEvents::PointerEvent* event) {cbfunction[BEvents::EventType::BUTTON_CLICK_EVENT] (event);}
void Widget::onPointerMotion (BEvents::PointerEvent* event) {cbfunction[BEvents::EventType::POINTER_MOTION_EVENT] (event);}
void Widget::onPointerDragged (BEvents::PointerEvent* event) {cbfunction[BEvents::EventType::POINTER_DRAG_EVENT] (event);}
void Widget::onWheelScrolled (BEvents::WheelEvent* event){cbfunction[BEvents::EventType::WHEEL_SCROLL_EVENT] (event);}
void Widget::onValueChanged (BEvents::ValueChangedEvent* event) {cbfunction[BEvents::EventType::VALUE_CHANGED_EVENT] (event);}
void Widget::onFocusIn (BEvents::FocusEvent* event) {cbfunction[BEvents::EventType::FOCUS_IN_EVENT] (event);}
void Widget::onFocusOut (BEvents::FocusEvent* event) {cbfunction[BEvents::EventType::FOCUS_OUT_EVENT] (event);}

void Widget::defaultCallback (BEvents::Event* event) {}

void Widget::dragAndDropCallback (BEvents::Event* event)
{
	if (event && event->getWidget())
	{
		Widget* w = event->getWidget();
		BEvents::PointerEvent* pev = (BEvents::PointerEvent*) event;

		w->moveTo (w->x_ + pev->getDeltaX (), w->y_ + pev->getDeltaY ());
	}
}

void Widget::focusInCallback (BEvents::Event* event)
{
	if (event && event->getWidget())
	{
		Widget* w = event->getWidget();
		BEvents::FocusEvent* focusEvent = (BEvents::FocusEvent*) event;
		if (w->getMainWindow() && w->getFocusWidget())
		{
			Window* main = w->getMainWindow();
			FocusWidget* focusWidget = w->getFocusWidget();

			// Release focusWidget first, if already added (somewhere)
			if (focusWidget->getParent()) focusWidget->getParent()->release (focusWidget);

			main->add (*focusWidget);
			focusWidget->moveTo (w->getOriginX () + focusEvent->getX() + 2, w->getOriginY () + focusEvent->getY() - focusWidget->getHeight() - 2);
			focusWidget->show ();
		}
	}
}

void Widget::focusOutCallback (BEvents::Event* event)
{
	if (event && event->getWidget())
	{
		Widget* w = event->getWidget();
		if (w->getFocusWidget() && w->getMainWindow())
		{
			Window* main = w->getMainWindow();
			FocusWidget* focusWidget = w->getFocusWidget();

			main->release (focusWidget);
		}
	}
}

double Widget::getXOffset () {return border_.getMargin () + border_.getLine()->getWidth() + border_.getPadding ();}

double Widget::getYOffset () {return border_.getMargin () + border_.getLine()->getWidth() + border_.getPadding ();}

double Widget::getEffectiveWidth ()
{
	double totalBorderWidth = getXOffset ();
	return (width_ > 2 * totalBorderWidth ? width_ - 2 * totalBorderWidth : 0);
}

double Widget::getEffectiveHeight ()
{
	double totalBorderHeight = getYOffset ();
	return (height_ > 2 * totalBorderHeight ? height_ - 2 * totalBorderHeight : 0);
}

std::vector <Widget*> Widget::getChildrenAsQueue (std::vector <Widget*> queue) const
{
	for (Widget* w : children_)
	{
		queue.push_back (w);
		if (!w->children_.empty()) queue = w->getChildrenAsQueue (queue);
	}
	return queue;
}

void Widget::postRedisplay () {postRedisplay (getOriginX (), getOriginY (), width_, height_);}

void Widget::postRedisplay (const double xabs, const double yabs, const double width, const double height)
{
	if (main_)
	{
		BEvents::ExposeEvent* event = new BEvents::ExposeEvent (this, BEvents::EXPOSE_EVENT, xabs, yabs, width, height);
		main_->addEventToQueue (event);
	}
}

void Widget::redisplay (cairo_surface_t* surface, double x, double y, double width, double height)
{
	if (main_ && visible && fitToArea (x, y, width, height))
	{
		// Copy widgets surface onto main surface
		double x0 = getOriginX ();
		double y0 = getOriginY ();

		cairo_t* cr = cairo_create (surface);
		cairo_set_source_surface (cr, widgetSurface, x0, y0);
		cairo_rectangle (cr, x + x0, y + y0, width, height);
		cairo_fill (cr);
		cairo_destroy (cr);

		for (Widget* w : children_)
		{
			if (w)
			{
				double xNew = x - w->x_;
				double yNew = y - w->y_;
				if (filter (w))
				{
					w->redisplay (surface, xNew, yNew, width, height);
				}
			}
		}
	}
}

bool Widget::filter (Widget* widget) {return true;}

void Widget::draw (const double x, const double y, const double width, const double height)
{
	if ((!widgetSurface) || (cairo_surface_status (widgetSurface) != CAIRO_STATUS_SUCCESS)) return;
	cairo_surface_clear (widgetSurface);
	cairo_t* cr = cairo_create (widgetSurface);

	if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
	{
		// Limit cairo-drawing area
		cairo_rectangle (cr, x, y, width, height);
		cairo_clip (cr);

		double radius = border_.getRadius ();

		// Draw background
		double innerBorders = getXOffset ();
		double innerRadius = (radius > border_.getPadding () ? radius - border_.getPadding () : 0);
		cairo_surface_t* fillSurface = background_.getCairoSurface ();
		BColors::Color bc = *background_.getColor();

		if ((getEffectiveWidth () > 0) && (getEffectiveHeight () > 0))
		{
			if ((fillSurface && cairo_surface_status (fillSurface) == CAIRO_STATUS_SUCCESS) || (bc.getAlpha() != 0.0))
			{
				// Background_image ?
				if (fillSurface && cairo_surface_status (fillSurface) == CAIRO_STATUS_SUCCESS) cairo_set_source_surface (cr, fillSurface, 0, 0);

				// Plain Background color ?
				else cairo_set_source_rgba (cr, bc.getRed(), bc.getGreen(), bc.getBlue(), bc.getAlpha());

				// If drawing area < background are, draw only a rectangle for the drawing area (faster)
				if ((x >= innerBorders) && (x + width <= width_ - innerBorders) &&
					(y >= innerBorders) && (y + height <= height_ - innerBorders))
				{
					cairo_rectangle (cr, x, y, width, height);
				}
				else
				{
					cairo_rectangle_rounded (cr, innerBorders, innerBorders, getEffectiveWidth (), getEffectiveHeight (), innerRadius);
				}
				cairo_fill (cr);
			}

		}

		// Draw border frame
		double outerBorders = border_.getMargin ();
		BColors::Color lc = *border_.getLine()->getColor();

		if ((lc.getAlpha() != 0.0) &&
			(border_.getLine()->getWidth() != 0.0) &&
			(width_ >= 2 * outerBorders) &&
			(height_ >= 2 * outerBorders))
		{
			double lw = border_.getLine()->getWidth();
			cairo_rectangle_rounded (cr, outerBorders + lw / 2, outerBorders + lw / 2,
									 width_ - 2 * outerBorders - lw, height_ - 2 * outerBorders - lw, radius);

			cairo_set_source_rgba (cr, lc.getRed(), lc.getGreen(), lc.getBlue(), lc.getAlpha());
			cairo_set_line_width (cr, lw);
			cairo_stroke (cr);
		}
	}

	cairo_destroy (cr);
}

bool Widget::fitToArea (double& x, double& y, double& width, double& height)
{
	bool isInArea = true;
	if (x < 0.0)
	{
		if (x + width < 0.0)
		{
			x = 0.0;
			width = 0.0;
			isInArea = false;
		}
		else
		{
			width = x + width;
			x = 0.0;

		}
	}
	if (x + width > width_)
	{
		if (x > width_)
		{
			x = width_;
			width = 0.0;
			isInArea = false;
		}
		else
		{
			width = width_ - x;
		}
	}
	if (y < 0.0)
	{
		if (y + height < 0.0)
		{
			y = 0.0;
			height = 0.0;
			isInArea = false;
		}
		else
		{
			height = y + height;
			y = 0.0;

		}
	}
	if (y + height > height_)
	{
		if (y > height_)
		{
			y = height_;
			height = 0.0;
			isInArea = false;
		}
		else
		{
			height = height_ - y;
		}
	}

	return isInArea;
}

}
