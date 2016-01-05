#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32

#include "CCInput-win32.h"
#include "platform/CCCommon.h"

NS_CC_BEGIN

Input* Input::getInstance()
{
	if (s_sharedInput == nullptr)
	{
		s_sharedInput = new InputWin32();
		if (!s_sharedInput->init())
		{
			delete s_sharedInput;
			s_sharedInput = nullptr;
			CCLOG("ERROR: Could not init InputWin32");
		}
	}
	return s_sharedInput;
}

InputWin32::InputWin32()
{

}

bool InputWin32::init()
{
	pushCursor(CURSOR_ARROW);
	return Input::init();
}

static struct { int32_t id; LPTSTR resourceId; } cursorShapeMap[] =
{
	{ Input::CURSOR_ARROW, IDC_ARROW },
	{ Input::CURSOR_WAIT, IDC_WAIT },
	{ Input::CURSOR_PLUS, IDC_CROSS },
	{ Input::CURSOR_RESIZE_VERT, IDC_SIZEWE },
	{ Input::CURSOR_RESIZE_HORZ, IDC_SIZENS },
	{ Input::CURSOR_RESIZE_ALL, IDC_SIZEALL },
	{ Input::CURSOR_IBEAM, IDC_IBEAM },
	{ Input::CURSOR_RESIZE_NESW, IDC_SIZENESW },
	{ Input::CURSOR_RESIZE_NWSE, IDC_SIZENWSE },
	{ 0, 0 },
};

void InputWin32::setCursorShape(int32_t cursorId)
{
	LPTSTR resourceId = NULL;

	for (int32_t i = 0; cursorShapeMap[i].resourceId != NULL; ++i)
	{
		if (cursorId == cursorShapeMap[i].id)
		{
			resourceId = cursorShapeMap[i].resourceId;
			break;
		}
	}

	if (resourceId == NULL)
		return;

	HCURSOR cur = LoadCursor(NULL, resourceId);
	if (cur)
		SetCursor(cur);
}

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_WIN32