#ifndef PAD_HPP_
#define PAD_HPP_

struct Pad
{
	Pad () : Pad (0, 0, 0, 0) {}
	Pad (float ch, float pitchOctave, float velocity, float duration) :
		ch (ch),pitchOctave (pitchOctave), velocity (velocity), duration (duration) {}
	bool operator== (Pad& that) {return ((ch == that.ch) && (pitchOctave == that.pitchOctave) && (velocity == that.velocity) && (duration == that.duration));}

	float ch;			// Output channel number (1..4)
	float pitchOctave;
	float velocity;
	float duration;
};

#endif /* PAD_HPP_ */
