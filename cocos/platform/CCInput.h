#ifndef __CC_INPUT_H__
#define __CC_INPUT_H__

#include <string>
#include <vector>
#include <unordered_map>

#include "platform/CCPlatformMacros.h"
#include "base/ccTypes.h"

NS_CC_BEGIN

class CC_DLL Input
{
public:
	/**
	*  Gets the instance of Input.
	*/
	static Input* getInstance();

	/**
	*  Destroys the instance of Input.
	*/
	static void destroyInstance();

	/** @deprecated Use getInstance() instead */
	CC_DEPRECATED_ATTRIBUTE static Input* sharedInput() { return getInstance(); }

	/** @deprecated Use destroyInstance() instead */
	CC_DEPRECATED_ATTRIBUTE static void purgeInput() { destroyInstance(); }

	/**
	*  The destructor of Input.
	* @js NA
	* @lua NA
	*/
	virtual ~Input();

	enum
	{
		CURSOR_ARROW = 0,	///< Default Cursor
		CURSOR_WAIT,		///< Hourglass Cursor
		CURSOR_PLUS,		///< Arrow Plus
		CURSOR_RESIZE_VERT, ///< Resize Vertical
		CURSOR_RESIZE_HORZ, ///< Resize Horizontal
		CURSOR_RESIZE_ALL,  ///< Resize All
		CURSOR_IBEAM,		///< IBeam Used for Text Entry
		CURSOR_RESIZE_NESW, ///< Resize NESW
		CURSOR_RESIZE_NWSE,	///< Resize NWSE
	};

	virtual void setCursorShape(int32_t cursorId);

	void pushCursor(int32_t cursorId);
	void popCursor();
	void refreshCursor();

protected:
	/**
	*  The default constructor.
	*/
	Input();

	virtual bool init();

	void changeCursorShape(int32_t cursorId);///< Change the Current Cursor Shape

	std::vector<int32_t> _cursors;

	static Input* s_sharedInput;
};

NS_CC_END

#endif