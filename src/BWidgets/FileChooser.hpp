/* FileChooser.hpp
 * Copyright (C) 2019  Sven Jähnichen
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

#ifndef BWIDGETS_FILECHOOSER_HPP_
#define BWIDGETS_FILECHOOSER_HPP_

#define BWIDGETS_DEFAULT_FILECHOOSER_WIDTH 400
#define BWIDGETS_DEFAULT_FILECHOOSER_HEIGHT 320
#define BWIDGETS_DEFAULT_FILECHOOSER_OK_INDEX 0
#define BWIDGETS_DEFAULT_FILECHOOSER_OPEN_INDEX 1
#define BWIDGETS_DEFAULT_FILECHOOSER_CANCEL_INDEX 2
#define BWIDGETS_DEFAULT_FILECHOOSER_FILE_EXISTS_INDEX 3
#define BWIDGETS_DEFAULT_FILECHOOSER_FILE_NOT_EXISTS_INDEX 4
#define BWIDGETS_DEFAULT_FILECHOOSER_NEW_FOLDER_INDEX 5
#define BWIDGETS_DEFAULT_FILECHOOSER_NEW_FOLDER_FAIL_INDEX 6

#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR "/"
#endif

#include "PopupListBox.hpp"
#include "TextButton.hpp"
#include <regex>

namespace BWidgets
{

struct FileFilter
{
	std::string name;
	std::regex regex;
};

/**
 * Class BWidgets::FileChooser
 *
 * Menu widget for selection of a file.
 */
class FileChooser : public ValueWidget
{
public:
	FileChooser ();
	FileChooser (const double x, const double y, const double width, const double height, const std::string& name);
	FileChooser (const double x, const double y, const double width, const double height, const std::string& name,
		     const std::string& path);
	FileChooser (const double x, const double y, const double width, const double height, const std::string& name,
		     const std::string& path, const std::vector<FileFilter>& filters);
	FileChooser (const double x, const double y, const double width, const double height, const std::string& name,
		     const std::string& path, const std::vector<FileFilter>& filters, const std::string& buttonText);
	FileChooser (const double x, const double y, const double width, const double height, const std::string& name,
		     const std::string& path, const std::vector<FileFilter>& filters, const std::vector<std::string>& texts);


	FileChooser (const FileChooser& that);

	/**
	 * Assignment. Copies the file chooser properties from a source and keeps
	 * its position within the widget tree. Emits a
	 * BEvents::ExposeEvent if the file chooser is visible.
	 * @param that Source file chooser
	 */
	FileChooser& operator= (const FileChooser& that);

	/**
	 * Pattern cloning. Creates a new instance of the widget and copies all
	 * its properties.
	 */
	virtual Widget* clone () const override;

	/**
	 * Sets the path of the file chooser.
	 * @param path	Path
	 */
	void setPath (const std::string& path);

	/**
	 * Gets the current path
	 * @return	Current path
	 */
	std::string getPath () const;

	/**
	 * Sets the file name of the file chooser.
	 * @param filename	File name
	 */
	virtual void setFileName (const std::string& filename);

	/**
	 * Gets the selected file name
	 * @return	Selected file name
	 */
	std::string getFileName () const;

	/**
	 * Sets the filters of the file chooser.
	 * @param filters	File filters
	 */
	void setFilters (const std::vector<FileFilter>& filters);

	/**
	 * Gets the file filters
	 * @return	File filters
	 */
	std::vector<FileFilter> getFilters () const;

	/**
	 * Selects and activates one filter out of the file filters.
	 * @param name	Filter name
	 */
	void selectFilter (const std::string& name);

	/**
	 * Sets the text of the OK button.
	 * @param text	Text
	 */
	void setButtonText (const std::string& buttonText);

	/**
	 * Gets the Text of the OK button
	 * @return	Text
	 */
	std::string getButtonText ();

	/**
	 * Sets the text of the labels.
	 * @param texts	Vectors containing the texts as strings
	 */
	void setLabels (const std::vector<std::string>& texts);

	/**
	 * Gets the text of the labels.
	 * @return	Vectors containing the texts as strings
	 */
	std::vector<std::string> getLabels () const;

	/**
	 * Resizes the widget, redraw and emits a BEvents::ExposeEvent if the
	 * widget is visible. If no parameters are given, the widget will be
	 * resized to the size of the containing child widgets or to the text
	 * extends (what is higher).
	 * @param width		New widgets width
	 * @param height	New widgets height
	 * @param extends	New widget extends
	 */
	virtual void resize () override;
	virtual void resize (const double width, const double height) override;
	virtual void resize (const BUtilities::Point extends) override;

	/**
	 * Calls a redraw of the widget and calls postRedisplay () if the the
	 * Widget is visible.
	 * This method should be called if the widgets properties are indirectly
	 * changed.
	 */
	virtual void update () override;

	/**
	 * Scans theme for widget properties and applies these properties.
	 * @param theme	Theme to be scanned.
	 * 		Styles used are:
	 * 		...
	 * @param name	Name of the BStyles::StyleSet within the theme to be
	 * 		applied.
	 */
	virtual void applyTheme (BStyles::Theme& theme) override;
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override;

	static void fileListBoxClickedCallback (BEvents::Event* event);
	static void filterPopupListBoxClickedCallback (BEvents::Event* event);
	static void cancelButtonClickedCallback (BEvents::Event* event);
	static void okButtonClickedCallback (BEvents::Event* event);
	static void confirmCancelButtonClickedCallback (BEvents::Event* event);
	static void confirmOkButtonClickedCallback (BEvents::Event* event);
	static void newFolderButtonClickedCallback (BEvents::Event* event);
	static void createCancelButtonClickedCallback (BEvents::Event* event);
	static void createOkButtonClickedCallback (BEvents::Event* event);

protected:

	void enterDir ();
	void processFileSelected();

	std::vector<FileFilter> filters;
	std::vector<std::string> dirs;
	std::vector<std::string> files;
	std::vector<std::string> labels;
	BColors::ColorSet bgColors;
	Label pathNameBox;
	ListBox fileListBox;
	Label fileNameLabel;
	Label fileNameBox;
	PopupListBox filterPopupListBox;
	TextButton cancelButton;
	TextButton okButton;
	Label fileListBoxFileLabel;
	Label fileListBoxDirLabel;
	Label filterPopupListBoxFilterLabel;

	Widget confirmBox;
	Label confirmLabel;
	TextButton confirmCancelButton;
	TextButton confirmOkButton;

	class NewFolderButton: public Button
	{
	public:
		NewFolderButton (const double x, const double y, const double width, const double height, const std::string& name, double defaultValue = 0.0);

	protected:
		virtual void draw (const BUtilities::RectArea& area) override;
	};

	NewFolderButton newFolderButton;

	Widget createBox;
	Label createLabel;
	Label createInput;
	Label createError;
	TextButton createCancelButton;
	TextButton createOkButton;

	virtual std::function<void (BEvents::Event*)> getFileListBoxClickedCallback();

	bool isDir (const std::string& path, const std::string& name) const;
};

}

#endif /* BWIDGETS_FILECHOOSER_HPP_ */
