/* B.Scale
 * Basic music note operation tools
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

#ifndef BSCALE_HPP_
#define BSCALE_HPP_

#include <cstdint>
#include <array>
#include <string>
#include <cmath>

#define ENOTE -128

#define CROMATICSCALE 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
#define MAJORSCALE 0, 2, 4, 5, 7, 9, 11, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE
#define MINORSCALE 0, 2, 3, 5, 7, 8, 10, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE
#define HARMONICMAJORSCALE 0, 2, 4, 5, 7, 8, 11, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE
#define HARMONICMINORSCALE 0, 2, 3, 5, 7, 8, 11, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE
#define MELODICMINORSCALE 0, 2, 3, 5, 7, 9, 10, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE
#define DORIANSCALE 0, 2, 3, 5, 7, 9, 10, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE
#define PHRYGIANSCALE 0, 1, 3, 5, 7, 8, 10, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE
#define LYDIANSCALE 0, 2, 4, 6, 7, 9, 11, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE
#define MIXOLYDIANSCALE 0, 2, 4, 5, 7, 9, 10, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE
#define LOKRIANSCALE 0, 1, 3, 5, 6, 8, 10, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE
#define HUNGARIANMINORSCALE 0, 2, 3, 6, 7, 8, 11, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE
#define MAJORPENTATONICSCALE 0, 2, 4, 7, 9, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE
#define MINORPENTATONICSCALE 0, 3, 5, 7, 10, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE, ENOTE

typedef std::array<int, 12> BScaleNotes;

typedef enum {
	FLAT			= -1,
	NATURAL			= 0,
	SHARP			= 1
} SignatureIndex;

const BScaleNotes defaultScale = {CROMATICSCALE};

const char noteSymbols[12] = {'C', 0, 'D', 0, 'E', 'F', 0, 'G', 0, 'A', 0, 'B'};

class BScale {
public:
	BScale (const int root, const BScaleNotes& elementarray);
	BScale (const int root, const SignatureIndex signature, const BScaleNotes& elementarray);
	void setRoot (int root);
	int getRoot ();
	void setScale (BScaleNotes& elementarray);
	BScaleNotes getScale ();
	int getMIDInote (int element);
	int getElement (int midiNote);
	int getSize ();
	std::string getSymbol (int element);

protected:
	void createSymbols ();
	BScaleNotes scale;
	int rootNote;
	SignatureIndex signature;
	char symbols[12][6];
};

BScale::BScale (const int root, const BScaleNotes& elementarray) : BScale (root, NATURAL, elementarray) {}
BScale::BScale (const int root, const SignatureIndex signature, const BScaleNotes& elementarray) :
rootNote (root), signature (signature), scale (elementarray)
{
	memset (symbols, 0, sizeof symbols);
	createSymbols ();
}

void BScale::createSymbols ()
{
	// Build a flat scale and a sharp scale
	char flatSymbols[12][6];
	char sharpSymbols[12][6];
	memset (flatSymbols, 0, sizeof flatSymbols);
	memset (sharpSymbols, 0, sizeof sharpSymbols);
	for (int i = 0; (i < 12) && (scale[i] != ENOTE); ++i)
	{
		int midiNote = getMIDInote (i);
		if ((midiNote >= 0) && (midiNote <= 127))
		{
			// Note without signature => take this symbol
			if (noteSymbols[midiNote % 12])
			{
				flatSymbols[i][0] = noteSymbols[midiNote % 12];
				sharpSymbols[i][0] = noteSymbols[midiNote % 12];
			}

			// Or with signature => build from neighbor
			else
			{
				flatSymbols[i][0] = noteSymbols[(midiNote + 1) % 12];
				strcat (flatSymbols[i], "b");
				sharpSymbols[i][0] = noteSymbols[(midiNote + 11) % 12];
				strcat (sharpSymbols[i], "#");
			}
		}

		// Note out of range => break
		else break;
	}

	switch (signature)
	{
		case FLAT:	memcpy (symbols, flatSymbols, sizeof symbols);
					break;

		case SHARP:	memcpy (symbols, sharpSymbols, sizeof symbols);
					break;

		default:	{
						// Count redundant symbols
						int flatRedunds = 0;
						int sharpRedunds = 0;
						for (int i = 1; (i < 12) && (flatSymbols[i][0]); ++i)
						{
							if (flatSymbols[i][0] == flatSymbols[i - 1][0]) ++flatRedunds;
							if (sharpSymbols[i][0] == sharpSymbols[i - 1][0]) ++sharpRedunds;
						}

						// Store the more relevant scale
						if (flatRedunds < sharpRedunds) memcpy (symbols, flatSymbols, sizeof symbols);
						else memcpy (symbols, sharpSymbols, sizeof symbols);
					}
	}
}

void BScale::setRoot (int root)
{
	rootNote = root;
	createSymbols ();
}

int BScale::getRoot () {return rootNote;}

void BScale::setScale (BScaleNotes& elementarray)
{
	int i = 0;
	for (; (i < 12) && (elementarray[i] != ENOTE); ++i) scale[i] = elementarray[i] % 12;
	for (; i < 12; ++i) scale[i] = ENOTE;
	createSymbols ();
}

BScaleNotes BScale::getScale () {return scale;}

/* Calculates a MIDI note for an element within a BScale
 * @param element: note position relative to root note in number of (scale-specific) notes,
 *                 e.g. note F in C major will be element 3
 * Returns MIDI note (00 .. 7F) or ENOTE if out of range
 */
int BScale::getMIDInote (int element)
{
	int size = getSize ();
	int octave = (int) floor ((float) element / size);
	int midiNote = octave * 12 + rootNote + scale [element - octave * size];
	if ((midiNote >=0) && (midiNote <= 127)) return midiNote;
	else return ENOTE;
}

/* Calculates the number of an element (note) within a BScale
 * @param midiNote: MIDI note (00 .. 7F)
 * Returns the element (note) relative to the root of the scale, e. g.
 *         returns 3 for the note F4  in a C(4) major scale
 */
int BScale::getElement (int midiNote)
{
	int ssize = getSize ();
	int octDiff = (int) floor (((float)midiNote - rootNote) / 12);
	int noteDiff = (midiNote - rootNote) - octDiff * 12;

	for (int i = 0; i < ssize; ++i)
	{
		if (scale[i] == noteDiff) return i + octDiff * ssize;
	}

	return ENOTE;
}

/* Returns number of elements of the BScale object, max. 12
 *
 */
int BScale::getSize()
{
	for (int8_t i = 0; i < 12; ++i)
	{
		if (scale[i] == ENOTE) return i;
	}
	return 12;

}

/* Returns the note symbol for an element (note) within a BScale.
 * !!! Don't use this method in a realtime process !!!
 * @param element: note position relative to root note in number of (scale-specific) notes,
 *                 e.g. note F in C major will be element 3
 * @return	Note symbol string
 *
 */
std::string BScale::getSymbol (int element)
{
	if (element < 0) return "";

	int ssize = getSize ();
	return std::string (symbols[element % ssize]);
}


#endif /* BSCALE_HPP_ */
