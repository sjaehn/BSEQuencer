/* B.SEQuencer
 * MIDI Step Sequencer LV2 Plugin
 *
 * Copyright (C) 2018, 2019 by Sven Jähnichen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef MIDISTACK_HPP_
#define MIDISTACK_HPP_

#include "MidiData.hpp"
#include <cstring>
#include <array>

#define MIDIBUFFERSIZE 256

 class MidiStack : private std::array<MidiData, MIDIBUFFERSIZE>
 {
 private:
 	size_t sz = 0;

 public:
        using std::array<MidiData, MIDIBUFFERSIZE>::operator[];

 	void clear () {sz = 0;}

 	size_t size () {return sz;}

 	void append (const int64_t frames, const uint8_t ch, const uint8_t status, const int note, const uint8_t velocity, uint8_t size = 3)
 	{
 		if (sz < MIDIBUFFERSIZE)
 		{
 			size_t insertpos = sz;
 			for (; (insertpos > 0) && (frames < operator[] (insertpos - 1).frames); --insertpos);
 			if (insertpos != sz) memmove (&operator[] (insertpos + 1), &operator[] (insertpos), (sz - insertpos) * sizeof (MidiData));
 			operator[] (insertpos) = {frames, size, ch, status, note, velocity};
 			++sz;
 		}
 	}
 };

 #endif /* MIDISTACK_HPP_ */
