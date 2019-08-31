#ifndef PLAYSTOPBUTTON_HPP_
#define PLAYSTOPBUTTON_HPP_

#include "BWidgets/ValueWidget.hpp"
#include "BWidgets/DrawingSurface.hpp"
#include "definitions.h"
#include "drawbutton.hpp"

class PlayStopButton : public BWidgets::ValueWidget
{
private:
	BWidgets::DrawingSurface drawingSurface;

	void drawSymbol (const double val)
	{
		BColors::Color color;
		int symbol;
		if (val == 0.0)
		{
			color = BColors::green;
			symbol = CTRL_PLAY_FWD;
		}
		else
		{
			color = BColors::red;
			symbol = CTRL_STOP;
		}
		drawButton (drawingSurface.getDrawingSurface (), 2, 2, drawingSurface.getWidth () - 4, drawingSurface.getHeight () - 4, color, symbol);
		drawingSurface.update ();
	}

public:
	PlayStopButton (const double x, const double y, const double width, const double height, const std::string name, const double defaultvalue) :
		ValueWidget (x, y, width, height, name, defaultvalue),
		drawingSurface (0, 0, width, height, "")
	{
		drawSymbol (value);
		drawingSurface.setClickable (false);
		add (drawingSurface);
	}

	void setValue (const double val)
	{
		ValueWidget::setValue (val);
		drawSymbol (value);
	}

	virtual void resize (const double width, const double height) override
	{
		if ((width != width_) || (height != height_))
		{
			Widget::resize (width, height);
			drawingSurface.resize (width, height);
			drawSymbol (value);
		}
	}

	void onButtonPressed (BEvents::PointerEvent* event)
	{
		if (value == 0.0) setValue (1.0);
		else setValue (0.0);
		cbfunction[BEvents::EventType::BUTTON_PRESS_EVENT] (event);
	}
};



#endif /* PLAYSTOPBUTTON_HPP_ */
