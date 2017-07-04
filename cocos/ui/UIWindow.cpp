
#include "UIWindow.h"

NS_CC_BEGIN

namespace ui {

Window::Window()
{
}


Window::~Window()
{
}

bool Window::init()
{
	return Layout::init();
}

bool Window::init(const std::string& uiFileName)
{
	return false;
}

}

NS_CC_END