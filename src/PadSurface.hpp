#ifndef PADSURFACE_HPP_
#define PADSURFACE_HPP_

#include "BWidgets/DrawingSurface.hpp"

class PadSurface : public BWidgets::DrawingSurface
{
public:
	PadSurface (const double x, const double y, const double width, const double height, const std::string& name) :
		DrawingSurface (x, y, width, height, name)
	{
		setDragable (true);
	}

	void onPointerMotionWhileButtonPressed (BEvents::PointerEvent* event)
	{
		cbfunction[BEvents::EventType::POINTER_MOTION_WHILE_BUTTON_PRESSED_EVENT] (event);
	}

};



#endif /* PADSURFACE_HPP_ */
