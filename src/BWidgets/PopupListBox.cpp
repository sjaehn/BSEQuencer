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
PopupListBox::PopupListBox () : PopupListBox (0.0, 0.0, 0.0, 0.0, 0.0, 0.0, "popuplistbox", std::vector<BItems::Item> {}, UNSELECTED) {}

PopupListBox::PopupListBox (const double x, const double y, const double width, const double height,
							const double listWidth, const double listHeight, const std::string& name,
							std::vector<std::string> strings, double preselection) :
		PopupListBox (x, y, width, height, 0.0, 0.0, listWidth, listHeight, name, strings, preselection) {}

PopupListBox::PopupListBox (const double x, const double y, const double width, const double height,
							const double listXOffset, const double listYOffset, const double listWidth, const double listHeight,
							const std::string& name, std::vector<std::string> strings, double preselection) :
		PopupListBox (x, y, width, height, listXOffset, listYOffset, listWidth, listHeight, name, std::vector<BItems::Item> {}, preselection)
{
	listBox.addItemText (strings);
	itemLabel.setText (listBox.getItem (preselection).string);
	listBox.setValue (preselection);
	if (!strings.empty()) listBox.setTop (1);
}

PopupListBox::PopupListBox (const double x, const double y, const double width, const double height,
							const double listWidth, const double listHeight, const std::string& name,
							std::vector<BItems::Item> items, double preselection) :
		PopupListBox (x, y, width, height, 0.0, 0.0, listWidth, listHeight, name, items, preselection) {}

PopupListBox::PopupListBox (const double x, const double y, const double width, const double height,
							const double listXOffset, const double listYOffset, const double listWidth, const double listHeight,
							const std::string& name, std::vector<BItems::Item> items, double preselection) :
		ItemBox (x, y, width, height, name, {preselection, ""}),
		downButton (0, 0, 0, 0, name + BWIDGETS_DEFAULT_POPUPLISTBOX_BUTTON_NAME, 0.0),
		listBox (0, 0, 0, 0, name + BWIDGETS_DEFAULT_POPUPLISTBOX_LISTBOX_NAME, items, preselection),
		listBoxXOffset (listXOffset), listBoxYOffset (listYOffset), listBoxWidth (listWidth), listBoxHeight (listHeight)

{
	itemLabel.setText (listBox.getItem (preselection).string);

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
	add (downButton);
}

PopupListBox::~PopupListBox () {}

PopupListBox& PopupListBox::operator= (const PopupListBox& that)
{
	downButton = that.downButton;
	listBox = that.listBox;
	listBoxXOffset = that.listBoxXOffset;
	listBoxYOffset = that.listBoxYOffset;
	listBoxWidth = that.listBoxWidth;
	listBoxHeight = that.listBoxHeight;

	ItemBox::operator= (that);
	listBox.extensionData = this;
	return *this;
}

std::vector<BItems::Item>* PopupListBox::getItemList () {return listBox.getItemList ();}

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
	if (val !=listBox.getValue ()) listBox.setValue (val);
	if (value != listBox.getValue ())
	{
		ItemBox::setValue (listBox.getValue ());
		setItemText (listBox.getItem (val).string);
	}
}
void PopupListBox::update ()
{
	// Update super widget first
	//setItemText (listBox.getActiveItem ().string);
	ItemBox::update ();

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
	if ((listBoxXOffset == 0.0) && (listBoxYOffset == 0.0)) listBox.moveTo (getOriginX () + x0, getOriginY () + y0 + h);
	else listBox.moveTo (getOriginX () + x0 + listBoxXOffset, getOriginY () + y0 + listBoxYOffset);
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

void PopupListBox::handleDownButtonClicked (BEvents::Event* event)
{
	if (event && (event->getEventType () == BEvents::EventType::BUTTON_PRESS_EVENT) && event->getWidget ())
	{
		BEvents::PointerEvent* ev = (BEvents::PointerEvent*) event;
		Widget* w = (Widget*) ev->getWidget ();
		if (w->getParent ())
		{
			PopupListBox* p = (PopupListBox*) w->getParent ();
			if (p->getParent ())
			{
				if (w == (Widget*) &(p->downButton)) p->onButtonPressed (ev);
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
			if ((p->getParent ()) && (w == (ValueWidget*) &(p->listBox)))
			{
				p->setValue (w->getValue ());
				p->listBox.hide ();
			}
		}
	}

}

}
