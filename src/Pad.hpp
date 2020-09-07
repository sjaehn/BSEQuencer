#ifndef PAD_HPP_
#define PAD_HPP_

struct Pad
{
	Pad () : Pad (0, 0, 0, 0, 0, 0, 0, 0, 0, 0) {}

	Pad	(float ch, float pitchNote, float pitchOctave, float velocity, float duration,
		float randGate, float randNote, float randOctave, float randVelocity, float randDuration) :
			ch (ch), pitchNote (pitchNote), pitchOctave (pitchOctave), velocity (velocity), duration (duration),
			randGate (randGate), randNote (randNote), randOctave (randOctave), randVelocity (randVelocity), randDuration (randDuration)
	{}

	bool operator== (Pad& that)
	{
		return
		(
			(ch == that.ch) && (pitchNote == that.pitchNote) && (pitchOctave == that.pitchOctave) &&
			(velocity == that.velocity) && (duration == that.duration) &&
			(randGate == that.randGate) && (randNote == that.randNote) && (randOctave == that.randOctave) &&
			(randVelocity == that.randVelocity) && (randDuration == that.randDuration)
		);
	}

	bool operator!= (Pad& that) {return (!operator== (that));}

	float ch;			// Output channel number (1..4)
	float pitchNote;
	float pitchOctave;
	float velocity;
	float duration;
	float randGate;
	float randNote;
	float randOctave;
	float randVelocity;
	float randDuration;
};

#endif /* PAD_HPP_ */
