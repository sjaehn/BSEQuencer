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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#define NR_SYSTEM_SCALES 14
#define NR_USER_SCALES 4
#define MAXSTEPS 32
#define ROWS 16
#define NR_SEQUENCER_CHS 4
#define NR_CTRL_BUTTONS 9
#define NR_MIDI_KEYS 128
#define AUTOPLAY_KEY 128
#define ALL_CH 0xFF
#define HALT_STEP 1000
#define BSEQUENCER_URI "https://www.jahnichen.de/plugins/lv2/BSEQuencer"
#define BSEQUENCER_GUI_URI "https://www.jahnichen.de/plugins/lv2/BSEQuencer#gui"

#ifndef LIMIT
#define LIMIT(val, min, max) ((val) > (max) ? (max) : ((val) < (min) ? (min) : (val)))
#endif /* LIMIT */

typedef enum {
	PER_BEAT		= 1,
	PER_BAR			= 2
} Steps;

typedef enum {
	AUTOPLAY		= 1,
	HOST_CONTROLLED	= 2
} ModeIndex;

typedef enum {
	Chromatic		= 1,
	Major			= 2,
	Minor			= 3
} ScaleIndex;

typedef enum {
	NO_CTRL			= 0x00,
	CTRL_PLAY_FWD	= 0x10,
	CTRL_PLAY_REW	= 0x20,
	CTRL_ALL_MARK	= 0x30,
	CTRL_MARK		= 0x40,
	CTRL_JUMP_FWD	= 0x50,
	CTRL_JUMP_BACK	= 0x60,
	CTRL_SKIP		= 0x70,
	CTRL_STOP		= 0x80
} CtrlButtons;


typedef struct {
	char name[64];
	int elements[ROWS];
	char altSymbols[ROWS][16];
} ScaleMap;


#endif /* DEFINITIONS_H_ */
