#include "CCInput.h"

#include "base/ccMacros.h"
#include "base/ccUtils.h"

NS_CC_BEGIN

Input* Input::s_sharedInput = nullptr;

void Input::destroyInstance()
{
	CC_SAFE_DELETE(s_sharedInput);
}

Input::Input()
{
}

Input::~Input()
{
}

bool Input::init()
{
	return true;
}

void Input::pushCursor(int32_t cursorId)
{
	if (std::find(_cursors.begin(), _cursors.end(), cursorId) != _cursors.end())
		return;

	_cursors.push_back(cursorId);

	changeCursorShape(cursorId);
}

void Input::popCursor()
{
	if (_cursors.size() <= 1)
	{
		return;
	}

	_cursors.pop_back();

	changeCursorShape(_cursors.back());
}

void Input::refreshCursor()
{
	changeCursorShape(_cursors.back());	
}

void Input::changeCursorShape(int32_t cursorId)
{
	if (cursorId >= 0)
	{
		setCursorShape(cursorId);
	}	
}

void Input::setCursorShape(int32_t cursorId)
{

}

NS_CC_END