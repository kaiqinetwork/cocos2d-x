#ifndef __CC_INPUT_WIN32_H__
#define __CC_INPUT_WIN32_H__

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32

#include "platform/CCInput.h"
#include "platform/CCPlatformMacros.h"
#include "base/ccTypes.h"
#include <string>
#include <vector>

NS_CC_BEGIN

class CC_DLL InputWin32 : public Input
{
	friend class Input;
	InputWin32();
public:
	/* override funtions */
	bool init();

	virtual void setCursorShape(int32_t cursorId) override;
};

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_WIN32

#endif    // __CC_INPUT_WIN32_H__