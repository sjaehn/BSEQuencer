/* PopupListBox.cpp
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

#include "PopupListBox.hpp"

namespace BWidgets
{
PopupListBox::PopupListBox () :
	PopupListBox (0.0, 0.0, 0.0, 0.0, 0.0, 0.0, "popuplistbox") {}

PopupListBox::PopupListBox (const double x, const double y, const double width,
			    const double height, const double listWidth,
			    const double listHeight, const std::string& name) :
	PopupListBox (x, y, width, height, 0.0, 0.0, listWidth, listHeight, name, BItems::ItemList (), UNSELECTED) {}

PopupListBox::PopupListBox (const double x, const double y, const double width,
			    const double height, const double listWidth,
			    const double listHeight, const std::string& name,
			    const BItems::ItemList& items, double preselection) :
	PopupListBox (x, y, width, height, 0.0, 0.0, listWidth, listHeight, name, items, preselection) {}

PopupListBox::PopupListBox (const double x, const double y, const double width,
			    const double height, const double listXOffset,
			    const double listYOffset, const double listWidth,
			    const double listHeight, const std::string& name) :
	PopupListBox (x, y, width, height, listXOffset, listYOffset, listWidth, listHeight, name, BItems::ItemList (), UNSELECTED) {}


PopupListBox::PopupListBox (const double x, const double y, const double width,
			    const double height, const double listXOffset,
			    const double listYOffset, const double listWidth,
			    const double listHeight, const std::string& name,
			    const BItems::ItemList& items, double preselection) :
		ItemBox (x, y, width, height, name, {UNSELECTED, nullptr}),
		downButton (0, 0, 0, 0, name + BWIDGETS_DEFAULT_POPUPLISTBOX_BUTTON_NAME, 0.0),
		listBox (0, 0, 0, 0, name + BWIDGETS_DEFAULT_POPUPLISTBOX_LISTBOX_NAME, items, preselection),
		listBoxXOffset (listXOffset), listBoxYOffset (listYOffset), listBoxWidth (listWidth), listBoxHeight (listHeight)

{
	setScrollable (true);

	// Set item
	if (preselection != UNSELECTED)
	{
		for (BItems::Item const& i : *listBox.getItemList ())
		{
			if (i.getValue() == preselection)
			{
				value = i.getValue ();
				item = i;
				initItem ();
				if (item.getWidget ()) add (*item.getWidget ());
				break;
			}
		}
	}

	downButton.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, PopupListBox::handleDownButtonClicked);
	listBox.extensionData = this;
	listBox.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, PopupListBox::handleValueChanged);

	listBox.hide ();

	add (downButton);
}

PopupListBox::PopupListBox (const PopupListBox& that) :
		ItemBox (that), downButton (that.downButton), listBox (that.listBox),
		listBoxXOffset (that.listBoxXOffset),
		listBoxYOffset (that.listBoxYOffset), listBoxWidth (that.listBoxWidth), listBoxHeight (that.listBoxHeight)
{
	initItem ();
	if (item.getWidget ()) add (*item.getWidget ());
	add (downButton);
}

PopupListBox& PopupListBox::operator= (const PopupListBox& that)
{
	downButton = that.downButton;

	listBox = that.listBox;
	listBoxXOffset = that.listBoxXOffset;
	listBoxYOffset = that.listBoxYOffset;
	listBoxWidth = that.listBoxWidth;
	listBoxHeight = that.listBoxHeight;

	ItemBox::operator= (that);
	initItem ();
	if (item.getWidget ()) add (*item.getWidget ());

	listBox.extensionData = this;
	return *this;
}

Widget* PopupListBox::clone () const {return new PopupListBox (*this);}

BItems::ItemList* PopupListBox::getItemList () {return listBox.getItemList ();}

ListBox* PopupListBox::getListBox () {return &listBox;}

void PopupListBox::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void PopupListBox::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	ItemBox::applyTheme (theme, name);
	downButton.applyTheme (theme, name + BWIDGETS_DEFAULT_POPUPLISTBOX_BUTTON_NAME);
	listBox.applyTheme (theme, name + BWIDGETS_DEFAULT_POPUPLISTBOX_LISTBOX_NAME);
}

void PopupListBox::setValue (const double val)
{
	if (val != listBox.getValue ()) listBox.setValue (val);
	if (value != listBox.getValue ())
	{
		item = *listBox.getItem (listBox.getValue ());
		initItem ();
		if (item.getWidget ()) add (*item.getWidget ());
		update ();
	}
}

void PopupListBox::moveListBox (const double xOffset, const double yOffset)
{
	listBoxXOffset = xOffset;
	listBoxYOffset = yOffset;
	if (listBox.isVisible()) update ();
}

void PopupListBox::resizeListBox (const double width, const double height)
{
	listBoxWidth = width;
	listBoxHeight = height;
	if (listBox.isVisible()) update ();
}

void PopupListBox::update ()
{
	// Update super widget first
	ItemBox::update ();

	// Keep button on top
	int cs = children_.size ();
	if ((cs >= 2) && (children_[cs - 1] != (Widget*) &downButton))
	{
		downButton.moveToTop ();
	}

	// Calculate size and position of widget elements
	double x0 = getXOffset ();
	double y0 = getYOffset ();
	double w = getEffectiveWidth ();
	double h = getEffectiveHeight ();

	// Down button
	double dw = (w > BWIDGETS_DEFAULT_POPUPLISTBOX_BUTTON_WIDTH ? BWIDGETS_DEFAULT_POPUPLISTBOX_BUTTON_WIDTH : w);
	downButton.moveTo (x0 + w - dw, y0);
	downButton.setWidth (dw);
	downButton.setHeight (h);

	// List box
	if ((main_) && (!listBox.getMainWindow()))
	{
		main_->add (listBox);
	}
	if ((!main_) && (listBox.getMainWindow())) listBox.getMainWindow()->release (&listBox);
	if ((listBoxXOffset == 0.0) && (listBoxYOffset == 0.0)) listBox.moveTo (getOriginX (), getOriginY () + getHeight ());
	else listBox.moveTo (getOriginX () + listBoxXOffset, getOriginY () + listBoxYOffset);
	listBox.setWidth (listBoxWidth);
	listBox.setHeight (listBoxHeight);

}

void PopupListBox::onButtonPressed (BEvents::PointerEvent* event)
{
	if (listBox.isVisible ()) listBox.hide ();
	else
	{
		update ();
		listBox.show ();
		listBox.moveToTop ();
	}

	//Widget::cbfunction[BEvents::EventType::BUTTON_PRESS_EVENT] (event);
}

void PopupListBox::onWheelScrolled (BEvents::WheelEvent* event)
{
	BItems::ItemList* itemList = listBox.getItemList ();
	double newNr = LIMIT (listBox.getActive () - event->getDeltaY (), 1, itemList->size ());
	BItems::ItemList::iterator it = std::next ((*itemList).begin (), newNr - 1);
	setValue ((*it).getValue());
}

void PopupListBox::initItem ()
{
	Widget* w = item.getWidget ();
	if (w)
	{
		w->getBorder ()->setPadding (BWIDGETS_DEFAULT_ITEMBOX_ITEM_PADDING);
		w->setClickable (false);
		w->setDraggable (false);
		w->setScrollable (false);
		w->setFocusable (false);
		w->setState (BColors::NORMAL);
		w->moveTo (0, 0);
		w->show ();
	}
}

void PopupListBox::handleDownButtonClicked (BEvents::Event* event)
{
	if (event && (event->getEventType () == BEvents::EventType::BUTTON_PRESS_EVENT) && event->getWidget ())
	{
		BEvents::PointerEvent* ev = (BEvents::PointerEvent*) event;
		Widget* w = ev->getWidget ();
		if (w->getParent ())
		{
			PopupListBox* p = (PopupListBox*) w->getParent ();
			if (p->getParent ())
			{
				if (w == &(p->downButton)) p->onButtonPressed (ev);
			}
		}
	}
}

void PopupListBox::handleValueChanged (BEvents::Event* event)
{
if (event && (event->getEventType () == BEvents::EventType::VALUE_CHANGED_EVENT) && event->getWidget ())
	{
		BEvents::ValueChangedEvent* ev = (BEvents::ValueChangedEvent*) event;
		ValueWidget* w = (ValueWidget*) ev->getWidget ();
		if (w->extensionData)
		{
			PopupListBox* p = (PopupListBox*) w->extensionData;
			if (p->getParent () && (w == (ValueWidget*) &(p->listBox)))
			{
				p->setValue (w->getValue ());
				p->listBox.hide ();
			}
		}
	}

}

}
