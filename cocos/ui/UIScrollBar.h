#ifndef __UISCROLLBAR_H__
#define __UISCROLLBAR_H__

#pragma once

#include "ui/UIWidget.h"
#include "ui/GUIExport.h"

NS_CC_BEGIN

class Sprite;

namespace ui {

class Scale9Sprite;

class CC_GUI_DLL ScrollBar : public Widget
{
	DECLARE_CLASS_GUI_INFO

public:
	enum class Direction
	{
		NONE,
		VERTICAL,
		HORIZONTAL,
	};

	enum class EventType
	{
		SCROLL,
		SCROLL_TO_MIN,
		SCROLL_TO_MAX,
	};

	typedef std::function<void(Ref*, ScrollBar::EventType)> ccScrollBarCallback;

	ScrollBar();
	virtual ~ScrollBar();

	static ScrollBar* create();

	Direction getDirection()const;
	float getMin() const;
	float getMax() const;
	float getLineStep() const;
	float getPageStep() const;
	float getScrollPosition() const;
	void setDirection(Direction dir);
	void setMin(float min);
	void setMax(float max);
	void setLineStep(float step);
	void setPageStep(float step);
	void setRanges(float min, float max, float lineStep, float pageStep);

protected:
	Direction _direction;

	Scale9Sprite*  _barRenderer;
	Node* _decreaseRenderer;
	Node* _increaseRenderer;
	Node* _thumbRenderer;
};

}

NS_CC_END

#endif /* defined(__CocoGUI__ScrollBar__) */
