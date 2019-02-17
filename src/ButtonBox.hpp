#ifndef BUTTONBOX_HPP_
#define BUTTONBOX_HPP_

#include <vector>
#include <cmath>
#include "BWidgets/BColors.hpp"
#include "BWidgets/ValueWidget.hpp"
#include "BWidgets/DrawingSurface.hpp"
#include "drawbutton.hpp"

class ButtonBox : public BWidgets::ValueWidget
{
private:
	typedef struct {
		BWidgets::DrawingSurface* widget;
		ButtonStyle style;
	} Button;
	std::vector<Button> buttons;

public:
	ButtonBox (const double x, const double y, const double width, const double height, const std::string name) :
		BWidgets::ValueWidget (x, y, width, height, name, 0.0) {}

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
			Widget* w = (Widget*) ev->getWidget ();
			if (w->getParent ())
			{
				ButtonBox* p = (ButtonBox*) w->getParent ();
				for (int i = 0; i < p->buttons.size (); ++i)
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
		if ((width != width_) || (height != height_))
		{
			double w = width / getWidth ();
			double h = height / getHeight ();
			Widget::resize (width, height);

			for (Button b : buttons)
			{
				b.widget->moveTo (b.widget->getX () * w, b.widget->getY () * h);
				b.widget->resize (b.widget->getWidth () * w, b.widget->getHeight () * h);
				drawButton (b.widget->getDrawingSurface(), 0, 0, b.widget->getEffectiveWidth(), b.widget->getEffectiveHeight(), b.style);
			}
		}
	}

	void addButton (const double x, const double y, const double width, const double height, const ButtonStyle style)
	{
		BWidgets::DrawingSurface* newWidget = new BWidgets::DrawingSurface (x - 3, y - 3, width + 6, height + 6, "buttonbox");
		newWidget->setBorder ({{{1.0, 1.0, 1.0, 0.0}, 1.0}, 0.0, 2.0, 0.0});
		newWidget->setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, ButtonBox::handleButtonClicked);
		drawButton (newWidget->getDrawingSurface(), 0, 0, width, height, style);
		add (*newWidget);
		Button newButton = {newWidget, style};
		buttons.push_back (newButton);
	}

	virtual void update () override
	{
		BWidgets::Widget::update ();

		for (int i = 0; i < buttons.size (); ++i)
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
