/* BValues.hpp
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

#ifndef BVALUES_HPP_
#define BVALUES_HPP_

#include <stdint.h>
#include <string>
#include <sstream>


namespace BValues {

/**
 * Converts a value to a string
 * @param value Value to be converted
 * @param format Number format, see printf.
 * @return Converted value as a std:string.
 */
std::string toBString (const double value);
std::string toBString (const std::string& format, const double value);

}

#endif /* BVALUES_HPP_ */
