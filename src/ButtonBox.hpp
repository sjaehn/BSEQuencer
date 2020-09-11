#ifndef BUTTONBOX_HPP_
#define BUTTONBOX_HPP_

#include <vector>
#include <cmath>
#include "BWidgets/ValueWidget.hpp"
#include "BWidgets/DrawingSurface.hpp"
#include "BWidgets/Focusable.hpp"
#include "BWidgets/Label.hpp"
#include "drawbutton.hpp"
#include "ButtonStyle.hpp"

class ButtonBox : public BWidgets::ValueWidget, public BWidgets::Focusable
{
private:
	struct Button
	{
		BWidgets::DrawingSurface* widget;
		ButtonStyle style;
	};
	std::vector<Button> buttons;

	BWidgets::Label focusLabel;

public:
	ButtonBox (const double x, const double y, const double width, const double height, const std::string name) :
		BWidgets::ValueWidget (x, y, width, height, name, 0.0),
		Focusable (std::chrono::milliseconds (BWIDGETS_DEFAULT_FOCUS_IN_MS),
			std::chrono::milliseconds (BWIDGETS_DEFAULT_FOCUS_OUT_MS)),
		buttons(),
		focusLabel (0, 0, 40, 20, name + "/focus", "")
	{
		focusLabel.setStacking (BWidgets::STACKING_OVERSIZE);
        	focusLabel.resize ();
        	focusLabel.hide ();
        	add (focusLabel);
	}

	~ButtonBox ()
	{
		while (!buttons.empty ())
		{
			BWidgets::DrawingSurface* b = buttons.back ().widget;
			if (b) delete b;
			buttons.pop_back ();
		}
	}

	static void handleButtonClicked (BEvents::Event* event)
	{
		if (event && (event->getEventType () == BEvents::EventType::BUTTON_PRESS_EVENT) && event->getWidget ())
		{
			BEvents::PointerEvent* ev = (BEvents::PointerEvent*) event;
			Widget* w = ev->getWidget ();
			if (w->getParent ())
			{
				ButtonBox* p = (ButtonBox*) w->getParent ();
				for (unsigned int i = 0; i < p->buttons.size (); ++i)
				{
					if (w == (Widget*) p->buttons[i].widget)
					{
						p->setValue (i);
						return;
					}
				}
			}
		}
	}

	virtual void setValue (const double val) override
	{
		if ((round (val) >= 0.0) && (round (val) <= buttons.size () - 1)) ValueWidget::setValue (round (val));
	}

	virtual void resize (const double width, const double height) override
	{
		if ((width != getWidth()) || (height != getHeight()))
		{
			double w = width / getWidth ();
			double h = height / getHeight ();
			Widget::resize (width, height);

			for (Button b : buttons)
			{
				b.widget->moveTo (b.widget->getPosition().x * w, b.widget->getPosition().y * h);
				b.widget->resize (b.widget->getWidth () * w, b.widget->getHeight () * h);
				drawButton (b.widget->getDrawingSurface(), 0, 0, b.widget->getEffectiveWidth(), b.widget->getEffectiveHeight(), b.style.color, b.style.symbol);
			}
		}
	}


	virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override
	{
		ValueWidget::applyTheme (theme, name);
                focusLabel.applyTheme (theme, name + "/focus");
                focusLabel.resize ();
	}

	virtual void applyTheme (BStyles::Theme& theme) override
	{
		applyTheme (theme, name_);
	}

	void onFocusIn (BEvents::FocusEvent* event) override
        {
        	if (event && event->getWidget())
        	{
        		BUtilities::Point pos = event->getPosition();

			// Get focused button
			Widget* w = getWidgetAt (pos, BWidgets::isVisible);
			if (w)
			{
				focusLabel.setText ("");
				for (Button const& b : buttons)
				{
					if (w == b.widget)
					{
						focusLabel.setText (b.style.name);
						break;
					}
				}

			}

			focusLabel.resize();
			focusLabel.raiseToTop();
        		focusLabel.moveTo (pos.x - 0.5 * focusLabel.getWidth(), pos.y - focusLabel.getHeight());
        		focusLabel.show();
        	}
        	Widget::onFocusIn (event);
        }
        void onFocusOut (BEvents::FocusEvent* event) override
        {
        	if (event && event->getWidget()) focusLabel.hide();
        	Widget::onFocusOut (event);
        }

	void addButton (const double x, const double y, const double width, const double height, const ButtonStyle style)
	{
		BWidgets::DrawingSurface* newWidget = new BWidgets::DrawingSurface (x - 3, y - 3, width + 6, height + 6, "buttonbox");
		if (!newWidget) throw std::bad_alloc ();
		newWidget->setBorder ({{{1.0, 1.0, 1.0, 0.0}, 1.0}, 0.0, 2.0, 0.0});
		newWidget->setFocusable (false);
		newWidget->setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, ButtonBox::handleButtonClicked);
		drawButton (newWidget->getDrawingSurface(), 0, 0, width, height, style.color, style.symbol);
		add (*newWidget);
		Button newButton = {newWidget, style};
		buttons.push_back (newButton);
	}

	virtual void update () override
	{
		BWidgets::Widget::update ();

		for (unsigned int i = 0; i < buttons.size (); ++i)
		{
			if (buttons[i].widget)
			{
				if (i == getValue ()) buttons[i].widget->getBorder()->getLine()->getColor()->setAlpha(1.0);
				else buttons[i].widget->getBorder()->getLine()->getColor()->setAlpha(0.0);
				buttons[i].widget->update();
			}
		}
	}
};



#endif /* BUTTONBOX_HPP_ */
