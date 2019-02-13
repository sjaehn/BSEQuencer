/* BValues.cpp
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

#include "BValues.hpp"

namespace BValues {

std::string toBString (const double value)
{
	std::ostringstream os;
	os << value;
	std::string str = os.str();
	return str;
}

std::string toBString (const std::string& format, const double value)
{
	char c[64];
	snprintf (c, 64, format.c_str (), value);
	std::string str = c;
	return c;
}

}
