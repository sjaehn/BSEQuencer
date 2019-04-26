/* ListBox.cpp
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

#include "ListBox.hpp"

namespace BWidgets
{
ListBox::ListBox () : ListBox (0.0, 0.0, 0.0, 0.0, "listbox", std::vector<BItems::Item> {}, UNSELECTED) {}

ListBox::ListBox (const double x, const double y, const double width, const double height, const std::string& name,
					  std::vector<std::string> strings, double preselection) :
		ListBox (x, y, width, height, name, std::vector<BItems::Item> {}, preselection)
{
	addItemText (strings);
	if ((preselection >= 1.0) && (preselection <= strings.size ())) activeNr = preselection;
	if (!items.empty()) listTop = 1;
}

ListBox::ListBox (const double x, const double y, const double width, const double height, const std::string& name,
					  std::vector<BItems::Item> items, double preselection) :
		ChoiceBox (x, y, width, height, name, items, preselection)
{
	if (items.empty ()) listTop = 0;
	else listTop = 1;

	upButton.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, ListBox::handleButtonClicked);
	downButton.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, ListBox::handleButtonClicked);
}

ListBox::ListBox (const ListBox& that) : ChoiceBox (that), listTop (that.listTop) {}

ListBox::~ListBox () {}

ListBox& ListBox::operator= (const ListBox& that)
{
	listTop = that.listTop;

	ChoiceBox::operator= (that);
	return *this;
}

void ListBox::setTop (const int top)
{
	double oldtop = listTop;

	if (items.empty ()) listTop = 0;

	else
	{
		size_t size = items.size ();
		if (top <= 0) listTop = 1;
		else if (top <= size) listTop = top;
		else listTop = size;

		int lines = getLines ();
		if ((getBottom () > size) && (size - lines >= 1)) listTop = size - lines + 1;
	}

	if (oldtop != listTop) update ();
}

int ListBox::getTop () const {return listTop;}

void ListBox::onWheelScrolled (BEvents::WheelEvent* event)
{
	setTop (getTop() - event->getDeltaY ());
}

void ListBox::handleButtonClicked (BEvents::Event* event)
{
	if (event && (event->getEventType () == BEvents::EventType::BUTTON_PRESS_EVENT) && event->getWidget ())
	{
		BEvents::PointerEvent* ev = (BEvents::PointerEvent*) event;
		Button* w = (Button*) ev->getWidget ();
		if (w->getParent ())
		{
			ListBox* p = (ListBox*) w->getParent ();
			if (p->getParent ())
			{
				if (w == (Button*) &(p->upButton)) p->setTop (p->getTop () - 1);
				if (w == (Button*) &(p->downButton)) p->setTop (p->getTop () + 1);
			}
		}
	}
}

void ListBox::updateLabels ()
{
	validateLabels ();

	// Keep Buttons on top
	int cs = children_.size ();
	if ((cs >= 2) && ((children_[cs - 1] != (Widget*) &upButton) || (children_[cs - 2] != (Widget*) &downButton)))
	{
		downButton.moveToTop ();
		upButton.moveToTop ();
	}

	double x0 = getXOffset ();
	double y0 = getYOffset ();
	double width = getEffectiveWidth ();
	double height = getEffectiveHeight ();
	double listHeight = (height >= 2 * BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT ? height - 2 * BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT : 0);
	double upButtonHeight = (height >= BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT ? BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT : height);
	double lineHeight = (labels.empty () ? 20 : labels[0]->getFont()->getFontSize() * BWIDGETS_DEFAULT_CHOICEBOX_LINE_HEIGHT);
	int lines = ceil (listHeight / lineHeight);

	for (int i = 0; i < labels.size (); ++i)
	{
		if ((i + 1 >= listTop) && (i + 1 < listTop + lines))
		{
			labels[i]->moveTo (x0 + BWIDGETS_DEFAULT_CHOICEBOX_PADDING, y0 + upButtonHeight + (i + 1 - listTop) * lineHeight);
			labels[i]->setWidth (width > 2 * BWIDGETS_DEFAULT_CHOICEBOX_PADDING ? width - 2 * BWIDGETS_DEFAULT_CHOICEBOX_PADDING : 0);
			labels[i]->setHeight (lineHeight);

			if (i + 1 == activeNr) labels[i]->setState (BColors::ACTIVE);
			else labels[i]->setState (BColors::NORMAL);

			labels[i]->show ();
		}
		else
		{
			labels[i]->hide ();
		}

	}
}

int ListBox::getLines ()
{
	double height = getEffectiveHeight ();
	double listHeight = (height >= 2 * BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT ? height - 2 * BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT : 0);
	double lineHeight = (labels.empty () ? 20 : labels[0]->getFont()->getFontSize() * BWIDGETS_DEFAULT_CHOICEBOX_LINE_HEIGHT);
	int lines = (listHeight > lineHeight ? listHeight / lineHeight : 1);
	return lines;
}

}
