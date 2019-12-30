#ifndef CIRCLEDSYMBOL_HPP_
#define CIRCLEDSYMBOL_HPP_

#include "BWidgets/Label.hpp"

#ifndef PI
#define PI 3.14159265
#endif

class CircledSymbol : public BWidgets::Label
{
public:
	CircledSymbol (const double x, const double y, const double width, const double height, const std::string& name, const std::string& text) :
		BWidgets::Label (x, y, width, height, name, text) {}
	CircledSymbol (const double x, const double y, const double width, const double height, const std::string& text) :
		CircledSymbol (x, y, width, height, text, text) {}
	CircledSymbol () : CircledSymbol (0, 0, 0, 0, "circledsymbol", "") {}

protected:
	virtual void draw (const BUtilities::RectArea& area) override
	{
		BWidgets::Label::draw (area);
		cairo_t* cr = cairo_create (widgetSurface_);
		cairo_rectangle (cr, area.getX (), area.getY (), area.getWidth (), area.getHeight ());
		cairo_clip (cr);
		cairo_set_line_width (cr, 2.0);
		BColors::Color lc = *labelColors.getColor (getState ());
		cairo_set_source_rgba (cr, CAIRO_RGBA (lc));
		double radius = (getEffectiveWidth () > getEffectiveHeight () ? getEffectiveHeight () : getEffectiveWidth ()) / 2 - 1;
		cairo_move_to (cr, getWidth() / 2 - radius, getHeight() / 2);
		cairo_arc (cr, getWidth() / 2, getHeight() / 2, radius, -PI, PI);
		cairo_close_path (cr);
		cairo_stroke (cr);
		cairo_destroy (cr);
	}

};

#endif /* CIRCLEDSYMBOL_HPP_ */
