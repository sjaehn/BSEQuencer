/* ListBox.hpp
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

#ifndef BWIDGETS_LISTBOX_HPP_
#define BWIDGETS_LISTBOX_HPP_

#include "ChoiceBox.hpp"

#define BWIDGETS_DEFAULT_LISTBOX_WIDTH BWIDGETS_DEFAULT_CHOICEBOX_WIDTH
#define BWIDGETS_DEFAULT_LISTBOX_HEIGHTH 100.0

namespace BWidgets
{

/**
 * Class BWidgets::ListBox
 *
 * Widget for selection of one item (string) out of a vector of strings.
 * It is a composite value widget consisting of a label, an up button, and a
 * down button. The value of this widget reflects the number the item selected
 * starting with 1.0. On change, a value changed event is emitted and this
 * widget.
 */
class ListBox : public ChoiceBox
{
public:
	ListBox ();
	ListBox (const double x, const double y, const double width, const double height,
				const std::string& name, std::vector<std::string> strings = {}, double preselection = UNSELECTED);
	ListBox (const double x, const double y, const double width, const double height,
				const std::string& name, std::vector<BItems::Item> items = {}, double preselection = UNSELECTED);

	/**
	 * Creates a new (orphan) choice box and copies the properties from a
	 * source choice box widget.
	 * @param that Source choice box
	 */
	ListBox (const ListBox& that);

	~ListBox ();

	/**
	 * Assignment. Copies the properties from a source choice box widget
	 * and keeps its name and its position within the widget tree. Emits a
	 * BEvents::ExposeEvent if the text widget is visible.
	 * @param that Source text widget
	 */
	ListBox& operator= (const ListBox& that);

	/**
	 * Sets the top line of the shown list.
	 * @param top Top Line of the list (starting with 1.0)
	 */
	void setTop (const int top);

	/**
	 * Gets the top line of the shown list.
	 * @param return Top Line of the list (starting with 1.0)
	 */
	virtual int getTop () const override;

protected:
	static void handleButtonClicked (BEvents::Event* event);
	virtual void updateLabels () override;
	virtual int getLines () override;

	int listTop;
};

}

#endif /* BWIDGETS_LISTBOX_HPP_ */
