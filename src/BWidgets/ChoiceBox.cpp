/* ChoiceBox.cpp
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

#include "ChoiceBox.hpp"

namespace BWidgets
{
ChoiceBox::ChoiceBox () : ChoiceBox (0.0, 0.0, 0.0, 0.0, "choicebox", std::vector<BItems::Item> {}, UNSELECTED) {}

ChoiceBox::ChoiceBox (const double x, const double y, const double width, const double height, const std::string& name,
					  std::vector<std::string> strings, double preselection) :
		ChoiceBox (x, y, width, height, name, std::vector<BItems::Item> {}, preselection)
{
	addItemText (strings);
	if ((preselection >= 1.0) && (preselection <= strings.size ())) activeNr = preselection;
}

ChoiceBox::ChoiceBox (const double x, const double y, const double width, const double height, const std::string& name,
					  std::vector<BItems::Item> items, double preselection) :
		ValueWidget (x, y, width, height, name, preselection),
		upButton (0, 0, 0, 0, name + BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_NAME, 0.0),
		downButton (0, 0, 0, 0, name + BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_NAME, 0.0),
		items (items), labels ({}),
		itemBorder (BWIDGETS_DEFAULT_BORDER),
		itemBackground (BWIDGETS_DEFAULT_BACKGROUND),
		itemColors (BWIDGETS_DEFAULT_TEXT_COLORS),
		itemFont (BWIDGETS_DEFAULT_FONT),
		activeNr (0)

{
	setScrollable (true);

	for (int i = 0; i < items.size (); ++ i)
	{
		if (preselection == items[i].value)
		{
			activeNr = i + 1;
			break;
		}
	}

	itemBorder.setPadding (BWIDGETS_DEFAULT_CHOICEBOX_ITEM_PADDING);

	background_ = BWIDGETS_DEFAULT_MENU_BACKGROUND;
	border_ = BWIDGETS_DEFAULT_MENU_BORDER;

	upButton.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, ChoiceBox::handleButtonClicked);
	downButton.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, ChoiceBox::handleButtonClicked);

	add (upButton);
	add (downButton);
}

ChoiceBox::ChoiceBox (const ChoiceBox& that) :
		ValueWidget (that), upButton (that.upButton), downButton (that.downButton), items (that.items), labels ({}), activeNr (that.activeNr),
		itemColors (that.itemColors), itemFont (that.itemFont), itemBorder (that.itemBorder), itemBackground (that.itemBackground)
{
	add (upButton);
	add (downButton);
}

ChoiceBox::~ChoiceBox ()
{
	deleteLabels ();
}

ChoiceBox& ChoiceBox::operator= (const ChoiceBox& that)
{
	upButton = that.upButton;
	downButton = that.downButton;
	items = that.items;
	deleteLabels ();
	itemColors = that.itemColors;
	itemFont = that.itemFont;
	itemBorder = that.itemBorder;
	itemBackground = that.itemBackground;
	activeNr = that.activeNr;

	ValueWidget::operator= (that);
	return *this;
}

void ChoiceBox::setTextColors (const BColors::ColorSet& colorset)
{
	itemColors = colorset;
	update ();
}

BColors::ColorSet* ChoiceBox::getTextColors () {return &itemColors;}

void ChoiceBox::setFont (const BStyles::Font& font)
{
	itemFont = font;
	update ();
}

std::vector<BItems::Item>* ChoiceBox::getItemList () {return &items;}

BItems::Item ChoiceBox::getItem (const double val) const
{
	for (BItems::Item i : items)
	{
		if (i.value == val) return i;
	}
	return BItems::Item {UNSELECTED,""};
}

void ChoiceBox::addItem (const BItems::Item& newItem)
{
	items.push_back (newItem);
	if (isVisible ()) update ();
}

void ChoiceBox::addItem (const std::vector<BItems::Item>& newItems)
{
	for (BItems::Item ni : newItems) items.push_back (ni);
	if (isVisible ()) update ();
}
void ChoiceBox::addItemText (const std::string& newItemText)
{
	BItems::Item as;
	if (items.empty()) as.value = 1.0;
	else as.value = floor (items.back().value + 1);
	as.string = newItemText;
	items.push_back (as);
	if (isVisible ()) update ();
}

void ChoiceBox::addItemText (const std::vector<std::string>& newItemTexts)
{
	for (std::string nstr : newItemTexts)
	{
		BItems::Item as;
		if (items.empty()) as.value = 1.0;
		else as.value = floor (items.back().value + 1);
		as.string = nstr;
		items.push_back (as);
	}
	if (isVisible ()) update ();
}

BStyles::Font* ChoiceBox::getFont () {return &itemFont;}

void ChoiceBox::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void ChoiceBox::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);
	upButton.applyTheme (theme, name + BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_NAME);
	downButton.applyTheme (theme, name + BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_NAME);

	// Item styles
	void* borderPtr = theme.getStyle(name + BWIDGETS_DEFAULT_CHOICEBOX_ITEM_NAME, BWIDGETS_KEYWORD_BORDER);
	if (borderPtr) itemBorder = *((BStyles::Border*) borderPtr);
	void* backgroundPtr = theme.getStyle(name + BWIDGETS_DEFAULT_CHOICEBOX_ITEM_NAME, BWIDGETS_KEYWORD_BACKGROUND);
	if (backgroundPtr) itemBackground = *((BStyles::Fill*) backgroundPtr);
	void* colorsPtr = theme.getStyle(name + BWIDGETS_DEFAULT_CHOICEBOX_ITEM_NAME, BWIDGETS_KEYWORD_TEXTCOLORS);
	if (colorsPtr) itemColors = *((BColors::ColorSet*) colorsPtr);
	void* fontPtr = theme.getStyle(name + BWIDGETS_DEFAULT_CHOICEBOX_ITEM_NAME, BWIDGETS_KEYWORD_FONT);
	if (fontPtr) itemFont = *((BStyles::Font*) fontPtr);

	update ();
}

void ChoiceBox::setValue (const double val)
{
	if (items.empty ())
	{
		ValueWidget::setValue (UNSELECTED);
		activeNr = 0;
	}

	else
	{
		for (int i = 0; i < items.size(); ++i)
		{
			if (val == items[i].value)
			{
				ValueWidget::setValue (val);
				activeNr = i + 1;
				return;
			}
		}
	}
}

int ChoiceBox::getTop () const {return activeNr;}

int ChoiceBox::getBottom () {return (getTop () + getLines () - 1);}

void ChoiceBox::update ()
{
	// Update super widget first
	Widget::update ();

	// Validate value and update activeNr
	setValue (getValue ());

	// Update labels
	updateLabels ();

	// Set position of buttons and item label
	double x0 = getXOffset ();
	double y0 = getYOffset ();
	double width = getEffectiveWidth ();
	double height = getEffectiveHeight ();

	if (getTop () > 1.0) upButton.show ();
	else upButton.hide ();
	double upButtonHeight = (height >= BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT ? BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT : height);
	upButton.moveTo (x0, y0);
	upButton.setWidth (width);
	upButton.setHeight (upButtonHeight);

	if (height > BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT)
	{
		if (getBottom () < items.size ()) downButton.show ();
		else downButton.hide ();
		double downButtonHeight = (height >= 2 * BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT ?
								   BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT :
								   height - BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT);
		downButton.moveTo(x0, y0 + height - downButtonHeight);
		downButton.setWidth (width);
		downButton.setHeight (downButtonHeight);
	}
}

void ChoiceBox::onWheelScrolled (BEvents::WheelEvent* event)
{
	setValue (getValue() - event->getDeltaY ());
}

void ChoiceBox::deleteLabels ()
{
	while  (!labels.empty ())
	{
		Label* l = labels.back ();
		if (l) delete l;
		labels.pop_back ();
	}
}

void ChoiceBox::validateLabels ()
{
	// Enlarge vector if needed
	if (labels.size () < items.size ())
	{
		for (size_t i = labels.size (); i < items.size (); ++i)
		{
			Label* newlabel = new Label (0, 0, 0, 0, getName () + BWIDGETS_DEFAULT_CHOICEBOX_ITEM_NAME, "");
			labels.push_back (newlabel);
			add (*newlabel);
		}
	}

	// Shrink vector if needed
	else if (labels.size () > items.size ())
	{
		for (size_t i = labels.size (); i > items.size (); --i)
		{
			Label* l = labels.back ();
			if (l) delete labels.back ();
			labels.pop_back ();
		}
	}

	for (int i = 0; i < labels.size (); ++i)
	{
		labels[i]->setText (items[i].string);
		labels[i]->setCallbackFunction (BEvents::BUTTON_PRESS_EVENT, ChoiceBox::handleLabelClicked);
		labels[i]->setBorder (itemBorder);
		labels[i]->setBackground (itemBackground);
		labels[i]->setFont (itemFont);
		labels[i]->setTextColors (itemColors);
	}
}

void ChoiceBox::updateLabels ()
{
	validateLabels ();

	double x0 = getXOffset ();
	double y0 = getYOffset ();
	double width = getEffectiveWidth ();
	double height = getEffectiveHeight ();
	double labelHeight = (height >= 2 * BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT ? height - 2 * BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT : 0);
	double upButtonHeight = (height >= BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT ? BWIDGETS_DEFAULT_CHOICEBOX_BUTTON_HEIGHT : height);

	for (int i = 0; i < labels.size (); ++i)
	{
		if (i + 1 == activeNr) labels[i]->show ();
		else labels[i]->hide ();

		labels[i]->moveTo (x0 + BWIDGETS_DEFAULT_CHOICEBOX_PADDING, y0 + upButtonHeight);
		labels[i]->setWidth (width > 2 * BWIDGETS_DEFAULT_CHOICEBOX_PADDING ? width - 2 * BWIDGETS_DEFAULT_CHOICEBOX_PADDING : 0);
		labels[i]->setHeight (labelHeight);
	}
}

int ChoiceBox::getLines () {return 1.0;}

void ChoiceBox::handleButtonClicked (BEvents::Event* event)
{
	if (event && (event->getEventType () == BEvents::EventType::BUTTON_PRESS_EVENT) && event->getWidget ())
	{
		BEvents::PointerEvent* ev = (BEvents::PointerEvent*) event;
		Button* w = (Button*) ev->getWidget ();
		if (w->getParent ())
		{
			ChoiceBox* p = (ChoiceBox*) w->getParent ();
			if (p->getParent ())
			{
				double actNr = p->activeNr;
				if ((w == (Button*) &(p->upButton)) && (actNr >= 2)) p->setValue (p->items[actNr - 1 - 1].value);
				if ((w == (Button*) &(p->downButton)) && (actNr < p->items.size ())) p->setValue (p->items[actNr - 1 + 1].value);
			}
		}
	}
}

void ChoiceBox::handleLabelClicked (BEvents::Event* event)
{
if (event && (event->getEventType () == BEvents::EventType::BUTTON_PRESS_EVENT) && event->getWidget ())
	{
		BEvents::PointerEvent* ev = (BEvents::PointerEvent*) event;
		Label* w = (Label*) ev->getWidget ();
		if (w->getParent ())
		{
			ChoiceBox* p = (ChoiceBox*) w->getParent ();
			if (p->getParent ())
			{
				for (int i = 0; i < p->labels.size (); ++i)
				{
					if (w == p->labels[i]) p->setValue (p->items[i].value);
				}
			}
		}
	}

}

}
