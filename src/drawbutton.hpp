#ifndef DRAWBUTTON_HPP_
#define DRAWBUTTON_HPP_

#include <cairo/cairo.h>
#include <cmath>
#include "BWidgets/cairoplus.h"
#include "BWidgets/BColors.hpp"
#include "definitions.h"

void drawButton (cairo_t* cr, double x, double y, double width, double height, BColors::Color color, int symbol)
{
	if ((width <= 0) || (height <= 0)) return;
	
	// Draw button
	BColors::Color illuminated2 = color; illuminated2.applyBrightness (0.33);
	BColors::Color illuminated = color; illuminated.applyBrightness (0.05);
	BColors::Color darkened = color; darkened.applyBrightness (-0.33);
	BColors::Color darkened2 = color; darkened2.applyBrightness (-0.67);
	cairo_pattern_t* pat = cairo_pattern_create_radial (x + width / 2, y + height / 2, 0.125 * width, x + width / 2, y + height / 2, 0.5 * width);

	cairo_pattern_add_color_stop_rgba (pat, 0.0, CAIRO_RGBA (illuminated));
	cairo_pattern_add_color_stop_rgba (pat, 1.0, CAIRO_RGBA (darkened));

	double rad = ((width < 20) || (height < 20) ?  (width < height ? width : height) / 4 : 5);
	cairo_rectangle_rounded (cr, x, y, width, height, rad);
	cairo_set_source (cr, pat);
	cairo_fill (cr);
	cairo_pattern_destroy (pat);

	// Draw symbol
	if (symbol != NO_CTRL)
	{
		BColors::Color col = darkened2;
		if (color.getRed() + color.getGreen() + color.getBlue() <= 0.33) col = illuminated2;
		cairo_set_source_rgba(cr, CAIRO_RGBA (col));
		cairo_set_line_width (cr, 0);
		double symbolSize = (width > height ? 0.8 * height : 0.8 * width);

		switch (symbol)
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

			case EDIT_PICK:
			cairo_move_to (cr, x + width / 2 + 0.07 * symbolSize, y + height / 2 - 0.17 * symbolSize);
			cairo_line_to (cr, x + width / 2 + 0.17 * symbolSize, y + height / 2 - 0.07 * symbolSize);
			cairo_line_to (cr, x + width / 2 - 0.23 * symbolSize, y + height / 2 + 0.33 * symbolSize);
			cairo_line_to (cr, x + width / 2 - 0.33 * symbolSize, y + height / 2 + 0.35 * symbolSize);
			cairo_line_to (cr, x + width / 2 - 0.38 * symbolSize, y + height / 2 + 0.4 * symbolSize);
			cairo_line_to (cr, x + width / 2 - 0.4 * symbolSize, y + height / 2 + 0.38 * symbolSize);
			cairo_line_to (cr, x + width / 2 - 0.35 * symbolSize, y + height / 2 + 0.33 * symbolSize);
			cairo_line_to (cr, x + width / 2 - 0.33 * symbolSize, y + height / 2 + 0.28 * symbolSize);
			cairo_line_to (cr, x + width / 2 + 0.07 * symbolSize, y + height / 2 - 0.17 * symbolSize);
			cairo_line_to (cr, x + width / 2 + 0.02 * symbolSize, y + height / 2 - 0.22 * symbolSize);
			cairo_line_to (cr, x + width / 2 + 0.07 * symbolSize, y + height / 2 - 0.27 * symbolSize);
			cairo_line_to (cr, x + width / 2 + 0.12 * symbolSize, y + height / 2 - 0.22 * symbolSize);
			cairo_line_to (cr, x + width / 2 + 0.27 * symbolSize, y + height / 2 - 0.37 * symbolSize);
			cairo_line_to (cr, x + width / 2 + 0.37 * symbolSize, y + height / 2 - 0.27 * symbolSize);
			cairo_line_to (cr, x + width / 2 + 0.22 * symbolSize, y + height / 2 - 0.12 * symbolSize);
			cairo_line_to (cr, x + width / 2 + 0.27 * symbolSize, y + height / 2 - 0.07 * symbolSize);
			cairo_line_to (cr, x + width / 2 + 0.22 * symbolSize, y + height / 2 - 0.02 * symbolSize);
			cairo_line_to (cr, x + width / 2 + 0.17 * symbolSize, y + height / 2 - 0.07 * symbolSize);
			cairo_set_line_width (cr, 1.0);
			cairo_stroke (cr);
			break;

			case EDIT_CUT:
			{
				cairo_rectangle (cr, x + width / 2 - 0.375 * symbolSize, y + height / 2 - 0.25 * symbolSize, 0.5 * symbolSize, 0.75 * symbolSize);
				cairo_set_source_rgba (cr, 0, 0, 0, 1);
				cairo_fill_preserve (cr);
				cairo_set_source_rgba(cr, CAIRO_RGBA (col));
				const double dash[] = {2.0};
				cairo_set_dash (cr, dash, 1, 0);
				cairo_set_line_width (cr, 1.0);
				cairo_stroke (cr);
				cairo_rectangle (cr, x + width / 2 - 0.125 * symbolSize, y + height / 2 - 0.5 * symbolSize, 0.5 * symbolSize, 0.75 * symbolSize);
				cairo_fill_preserve (cr);
				cairo_set_source_rgba (cr, 0, 0, 0, 1);
				cairo_set_dash (cr, dash, 0, 0);
				cairo_set_line_width (cr, 1.0);
				cairo_stroke (cr);
			}
			break;

			case EDIT_COPY:
			{
				cairo_rectangle (cr, x + width / 2 - 0.375 * symbolSize, y + height / 2 - 0.25 * symbolSize, 0.5 * symbolSize, 0.75 * symbolSize);
				cairo_set_source_rgba(cr, CAIRO_RGBA (col));
				cairo_fill_preserve (cr);
				const double dash[] = {2.0};
				cairo_set_dash (cr, dash, 1, 0);
				cairo_set_line_width (cr, 1.0);
				cairo_stroke (cr);
				cairo_rectangle (cr, x + width / 2 - 0.125 * symbolSize, y + height / 2 - 0.5 * symbolSize, 0.5 * symbolSize, 0.75 * symbolSize);
				cairo_fill_preserve (cr);
				cairo_set_source_rgba (cr, 0, 0, 0, 1);
				cairo_set_dash (cr, dash, 0, 0);
				cairo_set_line_width (cr, 1.0);
				cairo_stroke (cr);
			}
			break;

			case EDIT_PASTE:
			{
				cairo_rectangle (cr, x + width / 2 - 0.375 * symbolSize, y + height / 2 - 0.5 * symbolSize, 0.5 * symbolSize, 0.75 * symbolSize);
				cairo_set_source_rgba(cr, CAIRO_RGBA (col));
				cairo_fill_preserve (cr);
				cairo_set_source_rgba (cr, 0, 0, 0, 1);
				cairo_set_line_width (cr, 1.0);
				cairo_stroke (cr);
				cairo_set_source_rgba(cr, CAIRO_RGBA (col));
				cairo_rectangle (cr, x + width / 2 - 0.125 * symbolSize, y + height / 2 - 0.25 * symbolSize, 0.5 * symbolSize, 0.75 * symbolSize);
				cairo_fill_preserve (cr);
				cairo_set_source_rgba (cr, 0, 0, 0, 1);
				cairo_set_line_width (cr, 1.0);
				cairo_stroke (cr);
			}
			break;

			default:
			break;
		}
	}
}

void drawButton (cairo_surface_t* surface, double x, double y, double width, double height, BColors::Color color, int symbol)
{
	cairo_t* cr = cairo_create (surface);
	drawButton (cr, x, y, width, height, color, symbol);
	cairo_destroy (cr);
}


#endif /* DRAWBUTTON_HPP_ */
