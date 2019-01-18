#ifndef DRAWBUTTON_HPP_
#define DRAWBUTTON_HPP_

#include <cairo.h>
#include "BWidgets/cairoplus.h"
#include "BWidgets/BColors.hpp"

typedef struct
{
	BColors::Color color;
	std::string symbol;
} ButtonStyle;

void drawButton (cairo_t* cr, double x, double y, double width, double height, ButtonStyle style)
{
	// Draw button
	BColors::Color illuminated = style.color; illuminated.applyBrightness (0.05);
	BColors::Color darkened = style.color; darkened.applyBrightness (-0.33);
	BColors::Color darkened2 = style.color; darkened2.applyBrightness (-0.67);
	cairo_pattern_t* pat = cairo_pattern_create_radial (x + width / 2, y + height / 2, 0.125 * width,
														x + width / 2, y + height / 2, 0.5 * width);

	cairo_pattern_add_color_stop_rgba (pat, 0.0, CAIRO_RGBA (illuminated));
	cairo_pattern_add_color_stop_rgba (pat, 1.0, CAIRO_RGBA (darkened));
	cairo_rectangle_rounded (cr, x, y, width, height, 5);
	cairo_set_source (cr, pat);
	cairo_fill (cr);
	cairo_pattern_destroy (pat);

	// Draw symbol
	if (style.symbol != "")
	{
		cairo_set_source_rgba(cr, CAIRO_RGBA (darkened2));
		cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size(cr, (width > height ? height / 2 : width / 2));
		cairo_text_extents_t ext;
		cairo_text_extents (cr, style.symbol.c_str(), &ext);
		cairo_move_to (cr, x + width / 2 - ext.width / 2 - ext.x_bearing, y + height / 2 - ext.height / 2 - ext.y_bearing);
		cairo_show_text(cr, style.symbol.c_str());
	}
}

void drawButton (cairo_surface_t* surface, double x, double y, double width, double height, ButtonStyle style)
{
	cairo_t* cr = cairo_create (surface);
	drawButton (cr, x, y, width, height, style);
	cairo_destroy (cr);
}


#endif /* DRAWBUTTON_HPP_ */
