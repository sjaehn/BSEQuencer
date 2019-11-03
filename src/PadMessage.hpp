#ifndef PADMESSAGE_HPP_
#define PADMESSAGE_HPP_

#include "Pad.hpp"

#define ENDPADMESSAGE -1.0, 0.0, 0.0, 0.0, 0.0, 0.0

struct PadMessage : Pad
{
	PadMessage () : PadMessage (ENDPADMESSAGE) {}
	PadMessage (float step, float row, Pad pad) :
		Pad (pad), step (step), row (row) {}
	PadMessage (float step, float row, float ch, float pitchOctave, float velocity, float duration) :
		Pad (ch, pitchOctave, velocity, duration), step (step), row (row) {}
	bool operator== (PadMessage& that) {return (Pad::operator== (that) && (step == that.step) && (row == that.row));}

	float step;
	float row;
};



#endif /* PADMESSAGE_HPP_ */
