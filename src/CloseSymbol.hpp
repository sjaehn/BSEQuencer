#ifndef CLOSESYMBOL_HPP_
#define CLOSESYMBOL_HPP_

#include "BWidgets/Widget.hpp"
#include <cmath>

class CloseSymbol : public BWidgets::Widget
{
public:
	CloseSymbol (const double x, const double y, const double width, const double height, const std::string& name) :
		BWidgets::Widget (x, y, width, height, name) {}
	CloseSymbol () : CloseSymbol (0, 0, 0, 0, "closesymbol") {}

	BColors::ColorSet fgColors;

	virtual void applyTheme (BStyles::Theme& theme) override {applyTheme (theme, name_);}

	virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override
	{
		void* fgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_FGCOLORS);
		if (fgPtr)
		{
			fgColors = *((BColors::ColorSet*) fgPtr);
			update ();
		}
	}

protected:
	virtual void draw (const double x, const double y, const double width, const double height) override
	{
		BWidgets::Widget::draw (x, y, width, height);
		cairo_t* cr = cairo_create (widgetSurface);
		cairo_rectangle (cr, x, y, width, height);
		cairo_clip (cr);
		cairo_set_line_width (cr, 2.0);
		BColors::Color fg = *fgColors.getColor (getState ());
		cairo_set_source_rgba (cr, CAIRO_RGBA (fg));
		double radius = (getEffectiveWidth () > getEffectiveHeight () ? getEffectiveHeight () : getEffectiveWidth ()) / 2 - 1;
		cairo_move_to (cr, width_ / 2 - radius / 2, height_ / 2 - radius / 2);
		cairo_line_to (cr, width_ / 2 + radius / 2, height_ / 2 + radius / 2);
		cairo_move_to (cr, width_ / 2 + radius / 2, height_ / 2 - radius / 2);
		cairo_line_to (cr, width_ / 2 - radius / 2, height_ / 2 + radius / 2);
		cairo_stroke (cr);
		cairo_arc (cr, width_ / 2, height_ / 2, radius, 0, 2 * M_PI);
		cairo_close_path (cr);
		cairo_stroke (cr);
		cairo_destroy (cr);
	}

};

#endif /* CLOSESYMBOL_HPP_ */
