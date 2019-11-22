/*  B.SEQuencer
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

#ifndef SCALEMAP_HPP_
#define SCALEMAP_HPP_

#include "definitions.h"
#include <cstring>
#include <string>
#include <array>
#include "BScale.hpp"

struct ScaleMap;

struct RTScaleMap
{
	int iD;
	char name[64];
	int elements[ROWS];
	char altSymbols[ROWS][16];
	BScaleNotes scaleNotes;

        RTScaleMap& operator= (const ScaleMap& scaleMap);
};

struct ScaleMap
{
	int iD;
	std::string name;
        std::array<int, ROWS> elements;
        std::array<std::string, ROWS> altSymbols;
	BScaleNotes scaleNotes;

        ScaleMap& operator= (const RTScaleMap& scaleMap);
};

RTScaleMap& RTScaleMap::operator= (const ScaleMap& scaleMap)
{
	iD = scaleMap.iD;
        strncpy (name, scaleMap.name.c_str (), 63);
        for (size_t i = 0; i < ROWS; ++i)
        {
                elements[i] = scaleMap.elements[i];
                strncpy (altSymbols[i], scaleMap.altSymbols[i].c_str (), 15);
        }
	scaleNotes = scaleMap.scaleNotes;
        return *this;
}

ScaleMap& ScaleMap::operator= (const RTScaleMap& rtScaleMap)
{
	iD = rtScaleMap.iD;
        name = std::string (rtScaleMap.name);
        for (size_t i = 0; i < ROWS; ++i)
        {
                elements[i] = rtScaleMap.elements[i];
                altSymbols[i] = rtScaleMap.altSymbols[i];
        }
	scaleNotes = rtScaleMap.scaleNotes;
        return *this;
}


#define ALLROWS 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
#define NOALTSYMBOLS "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""

const std::array<ScaleMap, NR_SYSTEM_SCALES + NR_USER_SCALES> defaultScaleMaps =
{{
	{0, "Chromatic", {ALLROWS}, {NOALTSYMBOLS}, {CROMATICSCALE}},
	{1, "Major", {ALLROWS}, {NOALTSYMBOLS}, {MAJORSCALE}},
	{2, "Minor", {ALLROWS}, {NOALTSYMBOLS}, {MINORSCALE}},
	{3, "Harmonic major", {ALLROWS}, {NOALTSYMBOLS}, {HARMONICMAJORSCALE}},
	{4, "Harmonic minor", {ALLROWS}, {NOALTSYMBOLS}, {HARMONICMINORSCALE}},
	{5, "Melodic minor", {ALLROWS}, {NOALTSYMBOLS}, {MELODICMINORSCALE}},
	{6, "Dorian", {ALLROWS}, {NOALTSYMBOLS}, {DORIANSCALE}},
	{7, "Phrygian", {ALLROWS}, {NOALTSYMBOLS}, {PHRYGIANSCALE}},
	{8, "Lydian", {ALLROWS}, {NOALTSYMBOLS}, {LYDIANSCALE}},
	{9, "Mixolydian", {ALLROWS}, {NOALTSYMBOLS}, {MIXOLYDIANSCALE}},
	{10, "Lokrian", {ALLROWS}, {NOALTSYMBOLS}, {LOKRIANSCALE}},
	{11, "Hungarian minor", {ALLROWS}, {NOALTSYMBOLS}, {HUNGARIANMINORSCALE}},
	{12, "Major pentatonic", {ALLROWS}, {NOALTSYMBOLS}, {MAJORPENTATONICSCALE}},
	{13, "Minor pentatonic", {ALLROWS}, {NOALTSYMBOLS}, {MINORPENTATONICSCALE}},
	{
		18, "GM drumkit 1 (36-51)",
		{292, 294, 296, 297, 299, 301, 303, 304, 306, 293, 295, 298, 300, 302, 305, 307},
		{"Kick", "Snare 1", "Snare2", "Low F Tom", "Hi F Tom", "Low Tom", "Low M Tom", "Hi M Tom", "Hi Tom", "Side stick", "Clap", "Closed HH", "Pedal HH", "Open HH", "Crash", "Ride"},
		{CROMATICSCALE}
	},
	{
		19, "GM drumkit 2 (60-75)",
		{316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331},
		{"Hi Bongo", "Low Bongo", "Mute Hi Conga", "Open Hi Conga", "Low Conga", "Hi Timbale", "Low Timbale", "Hi Agogo", "Low Agogo", "Cabasa", "Maracas", "Short Whistle", "Long Whistle", "Short Guiro", "Long Guiro", "Claves"},
		{CROMATICSCALE}
	},
	/*********************** User scales **************************/
	{14, "User scale 1", {ALLROWS}, {NOALTSYMBOLS}, {CROMATICSCALE}},
	{15, "User scale 2", {ALLROWS}, {NOALTSYMBOLS}, {CROMATICSCALE}},
	{16, "User scale 3", {ALLROWS}, {NOALTSYMBOLS}, {CROMATICSCALE}},
	{17, "User scale 4", {ALLROWS}, {NOALTSYMBOLS}, {CROMATICSCALE}}
}};

#endif /* SCALEMAP_HPP_ */
