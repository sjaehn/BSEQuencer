/* Label.cpp
 * Copyright (C) 2018  Sven Jähnichen
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

#include "Label.hpp"
#include "Window.hpp"
#include <locale>
#include <codecvt>

namespace BWidgets
{
Label::Label () : Label (0.0, 0.0, 0.0, 0.0, "label", "") {}

Label::Label (const double x, const double y, const double width, const double height, const std::string& text) :
		Label (x, y, width, height, text, text) {}

Label::Label (const double x, const double y, const double width, const double height, const std::string& name, const std::string& text) :
		Widget (x, y, width, height, name),
		labelColors (BWIDGETS_DEFAULT_TEXT_COLORS),
		labelFont (BWIDGETS_DEFAULT_FONT),
		labelText (text),
		oldText (text),
		u32labelText (),
		editable (false),
		editMode (false),
		cursorFrom (0),
		cursorTo ()
{
	cbfunction[BEvents::EventType::POINTER_DRAG_EVENT] = Widget::defaultCallback;
	setDraggable (true);

	labelFont.setTextAlign (BWIDGETS_DEFAULT_LABEL_ALIGN);
	labelFont.setTextVAlign (BWIDGETS_DEFAULT_LABEL_VALIGN);

	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
	u32labelText = convert.from_bytes (labelText);
}

Label& Label::operator= (const Label& that)
{
	labelColors = that.labelColors;
	labelFont = that.labelFont;
	labelText = that.labelText;
	u32labelText = that.u32labelText;
	editable = that.editable;
	editMode = that.editMode;
	cursorFrom = that.cursorFrom;
	cursorTo = that.cursorTo;
	Widget::operator= (that);

	if (labelText != oldText)
	{
		postMessage (BWIDGETS_LABEL_TEXT_CHANGED_MESSAGE, BUtilities::makeAny<std::string> (labelText));
		oldText = labelText;
	}

	return *this;
}

Widget* Label::clone () const {return new Label (*this);}

void Label::setText (const std::string& text)
{
	if (text != labelText)
	{
		labelText = text;
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
		u32labelText = convert.from_bytes (labelText);
		size_t sz = u32labelText.length ();
		if (cursorFrom < sz) cursorFrom = sz;
		if (cursorTo < sz) cursorTo = sz;
		update ();
		if (labelText != oldText)
		{
			postMessage (BWIDGETS_LABEL_TEXT_CHANGED_MESSAGE, BUtilities::makeAny<std::string> (labelText));
			oldText = labelText;
		}
	}
}

std::string Label::getText () const {return labelText;}

void Label::setTextColors (const BColors::ColorSet& colorset)
{
	if (labelColors != colorset)
	{
		labelColors = colorset;
		update ();
	}
}
BColors::ColorSet* Label::getTextColors () {return &labelColors;}

void Label::setFont (const BStyles::Font& font)
{
	labelFont = font;
	update ();
}
BStyles::Font* Label::getFont () {return &labelFont;}

double Label::getTextWidth (std::string& text)
{
	double textwidth = 0.0;
	cairo_t* cr = cairo_create (widgetSurface);
	cairo_text_extents_t ext = labelFont.getTextExtents(cr, text.c_str ());
	textwidth = ext.width;
	cairo_destroy (cr);
	return textwidth;
}

void Label::resize ()
{
	// Get label text size
	cairo_t* cr = cairo_create (widgetSurface);
	cairo_text_extents_t ext = labelFont.getTextExtents(cr, labelText.c_str ());
	double height = ext.height + 2 * getYOffset ();
	double width = ext.width + 2 * getYOffset ();
	cairo_destroy (cr);

	// Or use embedded widgets size, if bigger
	for (Widget* w : children_)
	{
		if (w->getX () + w->getWidth() > width) width = w->getX () + w->getWidth();
		if (w->getY () + w->getHeight() > height) height = w->getY () + w->getHeight();
	}

	resize (width, height);
}

void Label::resize (const double width, const double height) {Widget::resize (width, height);}

void Label::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void Label::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);

	// Color
	void* colorsPtr = theme.getStyle(name, BWIDGETS_KEYWORD_TEXTCOLORS);
	if (colorsPtr) labelColors = *((BColors::ColorSet*) colorsPtr);

	// Font
	void* fontPtr = theme.getStyle(name, BWIDGETS_KEYWORD_FONT);
	if (fontPtr) labelFont = *((BStyles::Font*) fontPtr);

	if (colorsPtr || fontPtr) update ();
}

void Label::setEditable (const bool status)
{
	editable = true;
	if (editMode) update ();
}

bool Label::isEditable () const {return editable;}

void Label::setEditMode (const bool mode)
{
	if (mode != editMode)
	{
		editMode = mode;
		update ();
		if (editable) postMessage (BWIDGETS_LABEL_EDIT_ENTERED_MESSAGE, BUtilities::makeAny<bool> (editMode));
	}
}

bool Label::getEditMode () const {return editMode;}

void Label::setCursor (const size_t pos) {setCursor (pos, pos);}

void Label::setCursor (const size_t from, const size_t to)
{
	size_t cf = from;
	size_t ct = to;

	// Check limits
	size_t s32 = u32labelText.length ();
	if (cf > s32) cf = s32;
	if (ct > s32) ct = s32;

	// Apply changes
	if ((cf != cursorFrom) || (ct != cursorTo))
	{
		cursorFrom = cf;
		cursorTo = ct;
		update ();
	}
}

void Label::onKeyPressed (BEvents::KeyEvent* event)
{
	if (editable && (event) && (event->getWidget () == this) && (main_) && (main_->getKeyGrabWidget (0) == this))
	{
		uint32_t key = event->getKey ();

		switch (key)
		{
			case 8:		// Backspace
			{
				size_t cf = cursorFrom;
				size_t ct = cursorTo;
				if (ct < cf) { cf = cursorTo; ct = cursorFrom; }

				if (cf != ct) u32labelText.erase (cf, ct - cf);
				else if (cf > 0)
				{
					u32labelText.erase (cf - 1, 1);
					--cf;
				}

				std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
				labelText = convert.to_bytes (u32labelText);
				setCursor (cf);
			}
			break;

			case 13:	// Enter
			{
				main_->removeKeyGrab (this);
				setEditMode (false);
				if (labelText != oldText)
				{
					postMessage (BWIDGETS_LABEL_TEXT_CHANGED_MESSAGE, BUtilities::makeAny<std::string> (labelText));
					oldText = labelText;
				}
			}
			break;

			case 27:	// Escape
			{
				main_->removeKeyGrab (this);
				setEditMode (false);
				if (labelText != oldText) setText (oldText);
			}
			break;

			case 127:	// Delete
			{
				size_t cf = cursorFrom;
				size_t ct = cursorTo;
				if (ct < cf) { cf = cursorTo; ct = cursorFrom; }

				if (cf != ct) u32labelText.erase (cf, ct - cf);
				else if (cf < u32labelText.size ()) u32labelText.erase (cf, 1);

				std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
				labelText = convert.to_bytes (u32labelText);
				setCursor (cf);
				update ();
			}
			break;

			case PUGL_KEY_LEFT :
			if (cursorFrom > 0) setCursor (cursorFrom - 1);
			break;

			case PUGL_KEY_RIGHT :
			setCursor (cursorFrom + 1);
			break;

			default:
			{
				if ((key >= 0x20) && (key < 0x7F))
				{
					size_t cf = cursorFrom;
					size_t ct = cursorTo;
					if (ct < cf) { cf = cursorTo; ct = cursorFrom; }

					if (cf != ct) u32labelText.erase (cf, ct - cf);
					u32labelText.insert (u32labelText.begin () + cf, key);

					std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
					labelText = convert.to_bytes (u32labelText);
					setCursor (cf + 1);
				}
			}
			break;
		}
	}

	cbfunction[BEvents::EventType::KEY_PRESS_EVENT] (event);
}

void Label::onKeyReleased (BEvents::KeyEvent* event) {cbfunction[BEvents::EventType::KEY_RELEASE_EVENT] (event);}

void Label::onButtonPressed (BEvents::PointerEvent* event)
{
	if (editable && (event) && (event->getWidget () == this) && (main_) && (main_->getKeyGrabWidget (0) == this))
	{
		size_t cursor = getCursorFromCoords (event->getX (), event->getY ());
		setCursor (cursor, cursor);
	}

	cbfunction[BEvents::EventType::BUTTON_PRESS_EVENT] (event);
}

void Label::onButtonClicked (BEvents::PointerEvent* event)
{
	if (editable && (event) && (event->getWidget () == this) && (main_) && (main_->getKeyGrabWidget (0) != this))
	{
		main_->setKeyGrab (this);
		setEditMode (true);
	}

	cbfunction[BEvents::EventType::BUTTON_CLICK_EVENT] (event);
}

void Label::onPointerDragged (BEvents::PointerEvent* event)
{
	if (editable && (event) && (event->getWidget () == this) && (main_) && (main_->getKeyGrabWidget (0) == this))
	{
		size_t cursor = getCursorFromCoords (event->getX (), event->getY ());
		setCursor (cursorFrom, cursor);
	}

	cbfunction[BEvents::EventType::POINTER_DRAG_EVENT] (event);
}

size_t Label::getCursorFromCoords (const double x, const double y)
{
	size_t cursor = u32labelText.length ();
	if ((!widgetSurface) || (cairo_surface_status (widgetSurface) != CAIRO_STATUS_SUCCESS)) return 0;

	cairo_t* cr = cairo_create (widgetSurface);

	if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
	{
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;

		double xoff = getXOffset ();
		//double yoff = getYOffset ();
		double w = getEffectiveWidth ();
		//double h = getEffectiveHeight ();

		cairo_text_extents_t ext = labelFont.getTextExtents (cr, "|" + labelText + "|");
		cairo_text_extents_t ext0 = labelFont.getTextExtents(cr, "|");

		double x0;

		switch (labelFont.getTextAlign ())
		{
		case BStyles::TEXT_ALIGN_LEFT:		x0 = - ext.x_bearing;
							break;
		case BStyles::TEXT_ALIGN_CENTER:	x0 = w / 2 - (ext.width - 2 * ext0.width - 2 * ext0.x_bearing) / 2;
							break;
		case BStyles::TEXT_ALIGN_RIGHT:		x0 = w - (ext.width - 2 * ext0.width - 2 * ext0.x_bearing);
							break;
		default:				x0 = 0;
		}

		std::u32string u32fragment = U"";
		for (size_t i = 0; i < u32labelText.length (); ++i)
		{
			u32fragment += u32labelText[i];
			std::string fragment = convert.to_bytes (u32fragment);
			cairo_text_extents_t ext1 = labelFont.getTextExtents(cr, "|" + fragment + "|");

			if (x < xoff + x0 + ext1.width - 2 * ext0.width - 2 * ext0.x_bearing)
			{
				cursor = i;
				break;
			}
		}

		cairo_destroy (cr);
	}

	return cursor;
}

void Label::draw (const double x, const double y, const double width, const double height)
{
	if ((!widgetSurface) || (cairo_surface_status (widgetSurface) != CAIRO_STATUS_SUCCESS)) return;

	// Draw super class widget elements first
	Widget::draw (x, y, width, height);

	cairo_t* cr = cairo_create (widgetSurface);

	if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
	{
		// Limit cairo-drawing area
		cairo_rectangle (cr, x, y, width, height);
		cairo_clip (cr);

		double xoff = getXOffset ();
		double yoff = getYOffset ();
		double w = getEffectiveWidth ();
		double h = getEffectiveHeight ();

		cairo_text_extents_t ext = labelFont.getTextExtents(cr, "|" + labelText + "|");
		cairo_text_extents_t ext0 = labelFont.getTextExtents(cr, "|");
		cairo_select_font_face (cr, labelFont.getFontFamily ().c_str (), labelFont.getFontSlant (), labelFont.getFontWeight ());
		cairo_set_font_size (cr, labelFont.getFontSize ());

		double x0, y0;

		switch (labelFont.getTextAlign ())
		{
		case BStyles::TEXT_ALIGN_LEFT:		x0 = 0;
							break;
		case BStyles::TEXT_ALIGN_CENTER:	x0 = w / 2 - (ext.width - 2 * ext0.width - 2 * ext0.x_bearing) / 2;
							break;
		case BStyles::TEXT_ALIGN_RIGHT:		x0 = w - (ext.width - 2 * ext0.width - 2 * ext0.x_bearing);
							break;
		default:				x0 = 0;
		}

		switch (labelFont.getTextVAlign ())
		{
		case BStyles::TEXT_VALIGN_TOP:		y0 = - ext.y_bearing;
							break;
		case BStyles::TEXT_VALIGN_MIDDLE:	y0 = h / 2 - ext.height / 2 - ext.y_bearing;
							break;
		case BStyles::TEXT_VALIGN_BOTTOM:	y0 = h - ext.height - ext.y_bearing;
							break;
		default:				y0 = 0;
		}

		if (editable && editMode)
		{
			std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;

			size_t cf = cursorFrom;
			size_t ct = cursorTo;
			if (ct < cf) { cf = cursorTo; ct = cursorFrom; }
			std::string s1 = convert.to_bytes (u32labelText.substr (0, cf));
			std::string s2 = convert.to_bytes (u32labelText.substr (cf, ct - cf));
			std::string s3 = convert.to_bytes (u32labelText.substr (ct, std::u32string::npos));

			cairo_text_extents_t ext1 = labelFont.getTextExtents(cr, "|" + s1 + "|");
			cairo_text_extents_t ext2 = labelFont.getTextExtents(cr, "|" + s2 + "|");

			double w1 = ext1.width - 2 * ext0.width - 2 * ext0.x_bearing;
			double w2 = ext2.width - 2 * ext0.width - 2 * ext0.x_bearing;

			BColors::Color lc = *labelColors.getColor (BColors::ACTIVE);
			cairo_set_source_rgba (cr, lc.getRed (), lc.getGreen (), lc.getBlue (), lc.getAlpha ());
			cairo_set_line_width (cr, 1.0);
			cairo_rectangle (cr, xoff + x0 + w1, yoff + y0, w2, -ext0.height);
			cairo_stroke_preserve (cr);
			cairo_fill (cr);

			cairo_set_source_rgba (cr, lc.getRed (), lc.getGreen (), lc.getBlue (), lc.getAlpha ());
			cairo_move_to (cr, xoff + x0, yoff + y0);
			cairo_show_text (cr, s1.c_str ());

			cairo_set_source_rgba (cr, 1 - lc.getRed (), 1 - lc.getGreen (), 1 - lc.getBlue (), lc.getAlpha ());
			cairo_move_to (cr, xoff + x0 + w1, yoff + y0);
			cairo_show_text (cr, s2.c_str ());

			cairo_set_source_rgba (cr, lc.getRed (), lc.getGreen (), lc.getBlue (), lc.getAlpha ());
			cairo_move_to (cr, xoff + x0 + w1 + w2, yoff + y0);
			cairo_show_text (cr, s3.c_str ());
		}

		else
		{

			BColors::Color lc = *labelColors.getColor (getState ());
			cairo_set_source_rgba (cr, lc.getRed (), lc.getGreen (), lc.getBlue (), lc.getAlpha ());
			cairo_move_to (cr, xoff + x0, yoff + y0);
			cairo_show_text (cr, labelText.c_str ());
		}
	}

	cairo_destroy (cr);
}

}
