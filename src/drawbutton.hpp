#ifndef DRAWBUTTON_HPP_
#define DRAWBUTTON_HPP_

#include <cairo/cairo.h>
#include <cmath>
#include "BWidgets/cairoplus.h"
#include "BWidgets/BColors.hpp"
#include "definitions.h"

typedef struct
{
	BColors::Color color;
	CtrlButtons symbol;
} ButtonStyle;

void drawButton (cairo_t* cr, double x, double y, double width, double height, ButtonStyle style)
{
	// Draw button
	BColors::Color illuminated2 = style.color; illuminated2.applyBrightness (0.33);
	BColors::Color illuminated = style.color; illuminated.applyBrightness (0.05);
	BColors::Color darkened = style.color; darkened.applyBrightness (-0.33);
	BColors::Color darkened2 = style.color; darkened2.applyBrightness (-0.67);
	cairo_pattern_t* pat = cairo_pattern_create_radial (x + width / 2, y + height / 2, 0.125 * width,
														x + width / 2, y + height / 2, 0.5 * width);

	cairo_pattern_add_color_stop_rgba (pat, 0.0, CAIRO_RGBA (illuminated));
	cairo_pattern_add_color_stop_rgba (pat, 1.0, CAIRO_RGBA (darkened));

	double rad = ((width < 20) || (height < 20) ?  (width < height ? width : height) / 4 : 5);
	cairo_rectangle_rounded (cr, x, y, width, height, rad);
	cairo_set_source (cr, pat);
	cairo_fill (cr);
	cairo_pattern_destroy (pat);

	// Draw symbol
	if (style.symbol != NO_CTRL)
	{
		if (style.color.getRed() + style.color.getGreen() + style.color.getBlue() > 0.33) cairo_set_source_rgba(cr, CAIRO_RGBA (darkened2));
		else cairo_set_source_rgba(cr, CAIRO_RGBA (illuminated2));
		cairo_set_line_width (cr, 0);
		double symbolSize = (width > height ? 0.8 * height : 0.8 * width);

		switch (style.symbol)
		{
			case CTRL_PLAY_FWD:
			cairo_move_to (cr, x + width / 2 - symbolSize / 4, y + height / 2 - symbolSize / 3);
			cairo_line_to (cr, x + width / 2 + symbolSize / 4, y + height / 2);
			cairo_line_to (cr, x + width / 2 - symbolSize / 4, y + height / 2 + symbolSize / 3);
			cairo_close_path (cr);
			cairo_fill (cr);
			break;

			case CTRL_PLAY_REW:
			cairo_move_to (cr, x + width / 2 + symbolSize / 4, y + height / 2 - symbolSize / 3);
			cairo_line_to (cr, x + width / 2 - symbolSize / 4, y + height / 2);
			cairo_line_to (cr, x + width / 2 + symbolSize / 4, y + height / 2 + symbolSize / 3);
			cairo_close_path (cr);
			cairo_fill (cr);
			break;

			case CTRL_ALL_MARK:
			cairo_arc (cr, x + width / 2 - symbolSize / 4, y + height / 2, symbolSize / 6, 0, 2 * M_PI);
			cairo_fill (cr);
			cairo_arc (cr, x + width / 2 + symbolSize / 4, y + height / 2, symbolSize / 6, 0, 2 * M_PI);
			cairo_fill (cr);
			break;

			case CTRL_MARK:
			cairo_arc (cr, x + width / 2, y + height / 2, symbolSize / 4, 0, 2 * M_PI);
			cairo_fill (cr);
			break;

			case CTRL_JUMP_FWD:
			cairo_move_to (cr, x + width / 2 - symbolSize / 2, y + height / 2 - symbolSize / 6);
			cairo_line_to (cr, x + width / 2 - symbolSize / 4, y + height / 2);
			cairo_line_to (cr, x + width / 2 - symbolSize / 2, y + height / 2 + symbolSize / 6);
			cairo_close_path (cr);
			cairo_fill (cr);
			cairo_move_to (cr, x + width / 2 - symbolSize / 8, y + height / 2 - symbolSize / 6);
			cairo_line_to (cr, x + width / 2 + symbolSize / 8, y + height / 2);
			cairo_line_to (cr, x + width / 2 - symbolSize / 8, y + height / 2 + symbolSize / 6);
			cairo_close_path (cr);
			cairo_fill (cr);
			cairo_arc (cr, x + width / 2 + symbolSize / 3, y + height / 2, symbolSize / 6, 0, 2 * M_PI);
			cairo_fill (cr);
			break;

			case CTRL_JUMP_BACK:
			cairo_arc (cr, x + width / 2 - symbolSize / 3, y + height / 2, symbolSize / 6, 0, 2 * M_PI);
			cairo_fill (cr);
			cairo_move_to (cr, x + width / 2 + symbolSize / 8, y + height / 2 - symbolSize / 6);
			cairo_line_to (cr, x + width / 2 - symbolSize / 8, y + height / 2);
			cairo_line_to (cr, x + width / 2 + symbolSize / 8, y + height / 2 + symbolSize / 6);
			cairo_close_path (cr);
			cairo_fill (cr);
			cairo_move_to (cr, x + width / 2 + symbolSize / 2, y + height / 2 - symbolSize / 6);
			cairo_line_to (cr, x + width / 2 + symbolSize / 4, y + height / 2);
			cairo_line_to (cr, x + width / 2 + symbolSize / 2, y + height / 2 + symbolSize / 6);
			cairo_close_path (cr);
			cairo_fill (cr);
			break;

			case CTRL_SKIP:
			cairo_set_line_width (cr, symbolSize / 8);
			cairo_move_to (cr, x + width / 2 - symbolSize / 4, y + height / 2 - symbolSize / 4);
			cairo_line_to (cr, x + width / 2 + symbolSize / 4, y + height / 2 + symbolSize / 4);
			cairo_move_to (cr, x + width / 2 + symbolSize / 4, y + height / 2 - symbolSize / 4);
			cairo_line_to (cr, x + width / 2 - symbolSize / 4, y + height / 2 + symbolSize / 4);
			cairo_stroke (cr);
			break;

			case CTRL_STOP:
			cairo_rectangle (cr, x + width / 2 - symbolSize / 4, y + height / 2 - symbolSize / 4, symbolSize / 2, symbolSize / 2);
			cairo_fill (cr);
			break;

			default:
			break;
		}
	}
}

void drawButton (cairo_surface_t* surface, double x, double y, double width, double height, ButtonStyle style)
{
	cairo_t* cr = cairo_create (surface);
	drawButton (cr, x, y, width, height, style);
	cairo_destroy (cr);
}


#endif /* DRAWBUTTON_HPP_ */
