/* DialRangeValue.hpp
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

#ifndef BWIDGETS_DIALRANGEVALUE_HPP_
#define BWIDGETS_DIALRANGEVALUE_HPP_

#include "BWidgets/Dial.hpp"
#include "BWidgets/Label.hpp"
#include "BUtilities/to_string.hpp"

#define BWIDGETS_DEFAULT_DIALRANGEVALUE_WIDTH BWIDGETS_DEFAULT_DIAL_WIDTH
#define BWIDGETS_DEFAULT_DIALRANGEVALUE_HEIGHT (BWIDGETS_DEFAULT_DIAL_HEIGHT * 1.2)

enum RangeDirections
{
	UNIDIRECTIONAL	= 0,
	BIDIRECTIONAL	= 1
};

class DialRangeValue : public BWidgets::Dial
{
public:
	DialRangeValue () : DialRangeValue
	(
		0.0, 0.0, BWIDGETS_DEFAULT_DIALRANGEVALUE_WIDTH, BWIDGETS_DEFAULT_DIALRANGEVALUE_HEIGHT,
		"dialvalue",
		BWIDGETS_DEFAULT_VALUE, BWIDGETS_DEFAULT_RANGE_MIN, BWIDGETS_DEFAULT_RANGE_MAX, BWIDGETS_DEFAULT_RANGE_STEP,
		BWIDGETS_DEFAULT_VALUE_FORMAT
	) {}

	DialRangeValue (const double x, const double y, const double width, const double height, const std::string& name,
		   const double value, const double min, const double max, const double step, const std::string& valueFormat,
	   	   RangeDirections dir = BIDIRECTIONAL) :
		Dial (x, y, width, height, name, value, min, max, step),
		range (0, 0, 0, 0, "name/range", 0.0, (dir == BIDIRECTIONAL ? 0.0 : min - max), max - min, step),
		valueDisplay(0, 0.75 * height, width, 0.25 * height, name),
		valFormat (valueFormat),
		direction (dir)
	{
		valueDisplay.setText (BUtilities::to_string (value, valueFormat));
		add (valueDisplay);
		add (range);
	}

	DialRangeValue (const DialRangeValue& that) :
		Dial (that), range (that.range), valueDisplay (that.valueDisplay), valFormat (that.valFormat), direction (that.direction)
	{
		add (valueDisplay);
		add (range);
	}

	~DialRangeValue () {}

	DialRangeValue& operator= (const DialRangeValue& that)
	{
		release (&valueDisplay);
		release (&range);
		valFormat = that.valFormat;
		direction = that.direction;
		Dial::operator= (that);
		range = that.range;
		valueDisplay = that.valueDisplay;
		add (range);
		add (valueDisplay);

		return *this;
	}

	virtual Widget* clone () const override  {return new DialRangeValue (*this);}

	virtual void setValue (const double val) override
	{
		Dial::setValue (val);
		valueDisplay.setText(BUtilities::to_string (value, valFormat));
	}

	void setValueFormat (const std::string& valueFormat)
	{
		valFormat = valueFormat;
		update ();
	}

	std::string getValueFormat () const {return valFormat;}

	virtual void setMin (const double min) override
	{
		Dial::setMin (min);
		range.setMin (min - getMax());
	}

	virtual void setMax (const double max) override
	{
		Dial::setMin (max);
		range.setMin (max - getMin());
	}

	virtual void setStep (const double step) override
	{
		Dial::setMin (step);
		range.setMin (step);
	}

	BWidgets::Label* getDisplayLabel () {return &valueDisplay;}

	virtual void update () override
	{
		Dial::update();

		// Update display
		valueDisplay.moveTo (dialCenter.x - dialRadius / 0.8, dialCenter.y + 0.7 * dialRadius / 0.8);
		valueDisplay.setWidth (2 * dialRadius / 0.8);
		valueDisplay.setHeight (0.5 * dialRadius / 0.8);
		if (valueDisplay.getFont ()->getFontSize () != 0.4 * dialRadius / 0.8)
		{
			valueDisplay.getFont ()->setFontSize (0.4 * dialRadius / 0.8);
			valueDisplay.update ();
		}
		valueDisplay.setText (BUtilities::to_string (value, valFormat));

	}

	virtual void onButtonPressed (BEvents::PointerEvent* event) override
	{
		// Perform only if minimum requirements are satisfied
		if (main_ && isVisible () && (event->getButton () == BDevices::LEFT_BUTTON))
		{
			BUtilities::Point pos = event->getPosition ();
			BUtilities::Point ori = event->getOrigin ();
			double dist = (sqrt (pow (ori.x - dialCenter.x, 2) + pow (ori.y - dialCenter.y, 2)));
			double min = getMin ();
			double max = getMax ();

			// Direct dial point setting in hardChangeable mode
			if (hardChangeable)
			{
				if ((dist >= 0.1 * dialRadius) && (dist <= 1.0 * dialRadius))
				{
					double angle = atan2 (pos.x - dialCenter.x, dialCenter.y - pos.y) + M_PI;
					if ((angle >= 0.2 * M_PI) && (angle <= 1.8 * M_PI))
					{
						double corrAngle = LIMIT (angle, 0.25 * M_PI, 1.75 * M_PI);
						double frac = (corrAngle - 0.25 * M_PI) / (1.5 * M_PI);
						if (getStep () < 0) frac = 1 - frac;
						setValue (getMin () + frac * (getMax () - getMin ()));

					}
				}

				else if ((dist > 1.0 * dialRadius) && (dist <= 1.2 * dialRadius))
				{
					double angle = atan2 (pos.x - dialCenter.x, dialCenter.y - pos.y) + M_PI;
					if ((angle >= 0.2 * M_PI) && (angle <= 1.8 * M_PI))
					{
						double corrAngle = LIMIT (angle, 0.25 * M_PI, 1.75 * M_PI);
						double frac = (corrAngle - 0.25 * M_PI) / (1.5 * M_PI);
						if (getStep () < 0) frac = 1 - frac;
						range.setValue (getMin () + frac * (getMax () - getMin ()));

					}
				}
			}

			// Otherwise relative value change by dragging up or down
			// TODO Isn't there really no way to turn the dial in a sensitive but
			// safe way?
			else
			{
				if ((min != max) && (dialRadius >= 1))
				{
					if ((dist >= 0.1 * dialRadius) && (dist <= 1.0 * dialRadius))
					{
						double deltaFrac = -event->getDelta ().y / (dialRadius * 1.5 * M_PI);
						if (getStep () < 0) deltaFrac = -deltaFrac;
						softValue += deltaFrac * (max - min);
						setValue (getValue() + softValue);
					}

					else if ((dist > 1.0 * dialRadius) && (dist <= 1.2 * dialRadius))
					{
						double deltaFrac = -event->getDelta ().y / (dialRadius * 1.5 * M_PI);
						if (getStep () < 0) deltaFrac = -deltaFrac;
						double dv = deltaFrac * (max - min);
						range.setValue (range.getValue() + dv);
					}
				}
			}
		}
	}

	virtual void onWheelScrolled (BEvents::WheelEvent* event) override
	{
		BUtilities::Point pos = event->getPosition ();
		double dist = (sqrt (pow (pos.x - dialCenter.x, 2) + pow (pos.y - dialCenter.y, 2)));
		double min = getMin ();
		double max = getMax ();

		if ((min != max) && (dialRadius >= 1))
		{
			double step = (getStep () != 0 ? getStep () : (max - min) / (dialRadius * 1.5 * M_PI));
			if ((dist >= 0.1 * dialRadius) && (dist <= 1.0 * dialRadius)) setValue (getValue() + event->getDelta ().y * step);
			else if ((dist > 1.0 * dialRadius) && (dist <= 1.2 * dialRadius)) range.setValue (range.getValue() + event->getDelta ().y * step);
		}
	}

	virtual void onFocusIn (BEvents::FocusEvent* event) override
	{
		if (event && event->getWidget())
		{
			BUtilities::Point pos = event->getPosition();
			double dist = (sqrt (pow (pos.x - dialCenter.x, 2) + pow (pos.y - dialCenter.y, 2)));
			std::string valstr =
			(
				dist <= 1.0 * dialRadius ?
				BUtilities::to_string (getValue()) :
				(
					direction == BIDIRECTIONAL ?
					"Range: ±" + BUtilities::to_string (abs (range.getValue())) :
					"Range: " + BUtilities::to_string (range.getValue())
				)
			);
			focusLabel.setText(valstr);
			focusLabel.resize();
			focusLabel.moveTo (pos.x - 0.5 * focusLabel.getWidth(), pos.y - focusLabel.getHeight());
			focusLabel.show();
		}
		Widget::onFocusIn (event);
	}

	virtual void applyTheme (BStyles::Theme& theme) override {applyTheme (theme, name_);}
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override
	{
		Dial::applyTheme (theme, name);
		valueDisplay.applyTheme (theme, name);
		update ();
	}

	static void valueChangedCallback (BEvents::Event* event)
	{
		if (event && event->getWidget() && event->getWidget()->getParent()) event->getWidget()->getParent()->update();
	}

	RangeWidget range;

protected:
	virtual void updateCoords () override
	{
		double w = getEffectiveWidth ();
		double h = getEffectiveHeight ();
		dialRadius = (w < h / 1.2 ? w / 2 : h / 2.4) * 0.8;
		dialCenter.x = getWidth () / 2;
		dialCenter.y = getHeight () / 2 - 0.2 * dialRadius;
	}

	virtual void draw (const BUtilities::RectArea& area) override
	{
		if ((!widgetSurface_) || (cairo_surface_status (widgetSurface_) != CAIRO_STATUS_SUCCESS)) return;

		// Draw dial
		// only if minimum requirements satisfied
		if (dialRadius >= 12)
		{
			// Draw super class widget elements first
			Dial::draw (area);

			cairo_t* cr = cairo_create (widgetSurface_);

			if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
			{
				// Limit cairo-drawing area
				cairo_rectangle (cr, area.getX (), area.getY (), area.getWidth (), area.getHeight ());
				cairo_clip (cr);
				cairo_new_path (cr);

				double v0 = getRelativeValue ();
				double vr = (getMax() != getMin() ? range.getValue() / (getMax() - getMin()) : 0.0);
				double v1 = (direction == UNIDIRECTIONAL ? v0 : LIMIT (v0 - vr, 0.0, 1.0));
				double v2 = LIMIT (v0 + vr, 0.0, 1.0);
				if (v2 < v1) std::swap (v1, v2);
				double p1 = M_PI * (0.75 + 1.5 * v1);
				double p2 = M_PI * (0.75 + 1.5 * v2);

				// Colors uses within this method
				BColors::Color fgHi = *fgColors.getColor (getState ()); fgHi.applyBrightness (BWIDGETS_DEFAULT_ILLUMINATED);

				// Arc
				cairo_set_source_rgba (cr, CAIRO_RGBA (fgHi));
				cairo_set_line_width (cr, 0.0);
				cairo_arc (cr, dialCenter.x, dialCenter.y, 1.06 * dialRadius, p1, p2);
				cairo_arc_negative (cr, dialCenter.x, dialCenter.y ,  1.14 * dialRadius, p2, p1);
				cairo_close_path (cr);
				cairo_fill (cr);

				// Arrow
				cairo_save (cr);
				if ((direction == BIDIRECTIONAL) || (vr <= 0))
				{
					cairo_translate (cr, dialCenter.x, dialCenter.y);
					cairo_rotate (cr, p1);
					cairo_move_to (cr, 1.2 * dialRadius, 0);
					cairo_rel_line_to (cr, -0.2 * dialRadius, 0);
					cairo_rel_line_to (cr, 0.1 * dialRadius, -0.16 * dialRadius);
					cairo_close_path (cr);
					cairo_fill (cr);
				}
				if ((direction == BIDIRECTIONAL) || (vr > 0))
				{
					cairo_restore (cr);
					cairo_translate (cr, dialCenter.x, dialCenter.y);
					cairo_rotate (cr, p2);
					cairo_move_to (cr, 1.2 * dialRadius, 0);
					cairo_rel_line_to (cr, -0.2 * dialRadius, 0);
					cairo_rel_line_to (cr, 0.1 * dialRadius, 0.16 * dialRadius);
					cairo_close_path (cr);
					cairo_fill (cr);
				}

			}

			cairo_destroy (cr);
		}
	}

	BWidgets::Label valueDisplay;

	std::string valFormat;
	RangeDirections direction;
};

#endif /* BWIDGETS_DIALRANGEVALUE_HPP_ */
