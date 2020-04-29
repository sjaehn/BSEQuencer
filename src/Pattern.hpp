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

#ifndef PATTERN_HPP_
#define PATTERN_HPP_

#include <vector>
#include <algorithm>
#include "definitions.h"
#include "Pad.hpp"
#include "PadMessage.hpp"
#include "Journal.hpp"

#define MAXUNDO 20

class Pattern
{
public:
        void clear ()
        {
        	Pad pad0 = Pad ();

        	changes.oldMessage.clear ();
        	changes.newMessage.clear ();
        	journal.clear ();

        	for (int r = 0; r < ROWS; ++r)
        	{
        		for (int s = 0; s < MAXSTEPS; ++s)
        		{
        			setPad (r, s, pad0);
        		}
        	}

        	store ();
        }

        Pad getPad (const size_t row, const size_t step) const
        {
                size_t r = LIMIT (row, 0, ROWS);
        	size_t s = LIMIT (step, 0, MAXSTEPS);
        	return pads[r][s];
        }

        void setPad (const size_t row, const size_t step, const Pad& pad)
        {
        	size_t r = LIMIT (row, 0, ROWS);
                size_t s = LIMIT (step, 0, MAXSTEPS);

                changes.oldMessage.push_back (PadMessage (s, r, pads[r][s]));
                changes.newMessage.push_back (PadMessage (s, r, pad));
        	pads[r][s] = pad;
        }

        std::vector<PadMessage> undo ()
        {
        	if (!changes.newMessage.empty ()) store ();

        	std::vector<PadMessage> padMessages = journal.undo ();
        	std::reverse (padMessages.begin (), padMessages.end ());
        	for (PadMessage const& p : padMessages)
        	{
        		size_t r = LIMIT (p.row, 0, ROWS);
        		size_t s = LIMIT (p.step, 0, MAXSTEPS);
        		pads[r][s] = Pad (p);
        	}

        	return padMessages;
        }

        std::vector<PadMessage> redo ()
        {
        	if (!changes.newMessage.empty ()) store ();

        	std::vector<PadMessage> padMessages = journal.redo ();
        	for (PadMessage const& p : padMessages)
        	{
        		size_t r = LIMIT (p.row, 0, ROWS);
        		size_t s = LIMIT (p.step, 0, MAXSTEPS);
        		pads[r][s] = Pad (p);
        	}

        	return padMessages;
        }

        void store ()
        {
        	if (changes.newMessage.empty ()) return;

        	journal.push (changes.oldMessage, changes.newMessage);
        	changes.oldMessage.clear ();
        	changes.newMessage.clear ();
        }

        bool padHasAntecessor (const int row, const int step) const
        {
        	if (step <= 0) return false;

                const size_t r = LIMIT (row, 0, ROWS);
                const size_t s = LIMIT (step, 0, MAXSTEPS);
        	const Pad pad = pads[r][s];
        	const Pad ant = pads[r][s-1];

        	if (((int (pad.ch) & 0x0F) == (int (ant.ch) & 0x0F)) && (ant.duration > 1.0)) return true;
        	return false;
        }

        bool padHasSuccessor (const int row, const int step) const
        {
        	if (step >= MAXSTEPS - 1) return false;

                const size_t r = LIMIT (row, 0, ROWS);
                const size_t s = LIMIT (step, 0, MAXSTEPS);
        	const Pad pad = pads[r][s];
        	const Pad suc = pads[r][s+1];

        	if (((int (pad.ch) & 0x0F) == (int (suc.ch) & 0x0F)) && (pad.duration > 1.0)) return true;
        	return false;

        }

        int padGetSize (const int row, const int step)
        {
        	if (padHasAntecessor (row, step)) return padGetSize (row, step - 1);

        	for (int s = 0; ; ++s)
        	{
        		if (!padHasSuccessor (row, step + s)) return s + 1;
        	}
        }

private:
        Journal<std::vector<PadMessage>, MAXUNDO> journal;
        Pad pads [ROWS] [MAXSTEPS];
        struct
        {
                std::vector<PadMessage> oldMessage;
                std::vector<PadMessage> newMessage;
        } changes;
};

#endif /* PATTERN_HPP_ */
