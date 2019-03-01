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
#include <cstring>
#include <array>
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

const BScaleNotes defaultScale = {CROMATICSCALE};

typedef enum {
	FLAT			= -1,
	NATURAL			= 0,
	SHARP			= 1
} SignatureIndex;

const char flatSymbol[] = "♭";
const char sharpSymbol[] = "♯";
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
	void getSymbol (char* symbol, int element);

protected:
	BScaleNotes scale;
	int rootNote;
	SignatureIndex signature;
};

BScale::BScale (const int root, const BScaleNotes& elementarray) : BScale (root, NATURAL, elementarray) {}
BScale::BScale (const int root, const SignatureIndex signature, const BScaleNotes& elementarray) :
	rootNote (root), signature (signature), scale (elementarray) {}


void BScale::setRoot (int root) {rootNote = root;}

int BScale::getRoot () {return rootNote;}

void BScale::setScale (BScaleNotes& elementarray) {scale = elementarray;}

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

/* Composes the note symbol for an element (note) within a BScale
 * @param symbol: Returned symbol as a cstring (at least char[3])
 * @param element: note position relative to root note in number of (scale-specific) notes,
 *                 e.g. note F in C major will be element 3
 *
 * TODO Sometimes a natural note need to be forced to the next flat or sharp note (e.g., E -> F♭ in C minor)
 *
 */
void BScale::getSymbol (char* symbol, int element)
{
	symbol[0] = 0; symbol[1] = 0;
	int midiNote = getMIDInote (element);

	if ((midiNote >= 0) && (midiNote <= 127))
	{
		int8_t note = midiNote % 12;
		if (noteSymbols[note] == 0)
		{
			if (signature == FLAT)
			{
				symbol[0] = noteSymbols[(midiNote + 1) % 12];
				strcat (symbol, flatSymbol);
			}
			else
			{
				symbol[0] = noteSymbols[(midiNote - 1) % 12];
				strcat (symbol, sharpSymbol);
			}
		}
		else symbol[0] = noteSymbols[note];
	}
}


#endif /* BSCALE_HPP_ */

