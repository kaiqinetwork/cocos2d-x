#ifndef __UIWINDOW_H__
#define __UIWINDOW_H__

#pragma once

#include "ui/UILayout.h"
#include "ui/GUIExport.h"

NS_CC_BEGIN

class Label;
class Button;

namespace ui{

class CC_GUI_DLL Window : public Layout
{
public:
	Window();
	virtual ~Window();

	static Window* create();
	static Window* create(const std::string& uiFileName);

	void setDraggable(bool enable = true);

CC_CONSTRUCTOR_ACCESS:
	virtual bool init() override;
	virtual bool init(const std::string& uiFileName);

protected:
	Widget* _captionRenderer;
	Label* _titleRenderer;
	Button* _minBtnRenderer;
	Button* _maxBtnRenderer;
	Button* _restoreBtnRenderer;
	Button* _closeBtnRenderer;
	
	bool _draggable;
};

}

NS_CC_END

#endif /* defined(__CocoGUI__Window__) */