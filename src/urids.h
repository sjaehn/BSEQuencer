#ifndef URIDS_H_
#define URIDS_H_

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/time/time.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/state/state.h>

typedef struct
{
	LV2_URID atom_Sequence;
	LV2_URID atom_Float;
	LV2_URID atom_Double;
	LV2_URID atom_Int;
	LV2_URID atom_Object;
	LV2_URID atom_Blank;
	LV2_URID atom_eventTransfer;
	LV2_URID atom_Vector;
	LV2_URID atom_Long;
	LV2_URID midi_Event;
	LV2_URID time_Position;
	LV2_URID time_bar;
	LV2_URID time_barBeat;
	LV2_URID time_beatsPerMinute;
	LV2_URID time_beatsPerBar;
	LV2_URID time_beatUnit;
	LV2_URID time_speed;
	LV2_URID ui_on;
	LV2_URID ui_off;
	LV2_URID state_pad;
	LV2_URID notify_Event;
	LV2_URID notify_pad;
	LV2_URID notify_cursors;
	LV2_URID notify_notes;
	LV2_URID notify_channels;
}  BSEQuencerURIs;

void getURIs (LV2_URID_Map* m, BSEQuencerURIs* uris)
{
	uris->atom_Sequence = m->map(m->handle, LV2_ATOM__Sequence);
	uris->atom_Float = m->map(m->handle, LV2_ATOM__Float);
	uris->atom_Double = m->map(m->handle, LV2_ATOM__Double);
	uris->atom_Int = m->map(m->handle, LV2_ATOM__Int);
	uris->atom_Object = m->map(m->handle, LV2_ATOM__Object);
	uris->atom_Blank = m->map(m->handle, LV2_ATOM__Blank);
	uris->atom_eventTransfer = m->map(m->handle, LV2_ATOM__eventTransfer);
	uris->atom_Vector = m->map(m->handle, LV2_ATOM__Vector);
	uris->atom_Long = m->map (m->handle, LV2_ATOM__Long);
	uris->midi_Event = m->map(m->handle, LV2_MIDI__MidiEvent);
	uris->time_Position = m->map(m->handle, LV2_TIME__Position);
	uris->time_bar = m->map(m->handle, LV2_TIME__bar);
	uris->time_barBeat = m->map(m->handle, LV2_TIME__barBeat);
	uris->time_beatsPerMinute = m->map(m->handle, LV2_TIME__beatsPerMinute);
	uris->time_beatUnit = m->map(m->handle, LV2_TIME__beatUnit);
	uris->time_beatsPerBar = m->map(m->handle, LV2_TIME__beatsPerBar);
	uris->time_speed = m->map(m->handle, LV2_TIME__speed);
	uris->ui_on = m->map(m->handle, BSEQUENCER_URI "#UIon");
	uris->ui_off = m->map(m->handle, BSEQUENCER_URI "#UIoff");
	uris->state_pad = m->map(m->handle, BSEQUENCER_URI "#STATEpad");
	uris->notify_Event = m->map(m->handle, BSEQUENCER_URI "#NOTIFYev");
	uris->notify_pad = m->map(m->handle, BSEQUENCER_URI "#NOTIFYpad");
	uris->notify_cursors = m->map(m->handle, BSEQUENCER_URI "#NOTIFYcursors");
	uris->notify_notes = m->map(m->handle, BSEQUENCER_URI "#NOTIFYnotes");
	uris->notify_channels = m->map(m->handle, BSEQUENCER_URI "#NOTIFYchannels");
}

#endif /* URIDS_H_ */
