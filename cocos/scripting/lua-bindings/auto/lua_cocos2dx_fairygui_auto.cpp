#include "scripting/lua-bindings/auto/lua_cocos2dx_fairygui_auto.hpp"
#include "editor-support/libfairygui/Classes/FairyGUI.h"
#include "editor-support/libfairygui/Classes/FairyGUIMacros.h"
#include "scripting/lua-bindings/manual/tolua_fix.h"
#include "scripting/lua-bindings/manual/LuaBasicConversions.h"
TOLUA_API int register_all_cocos2dx_fairygui(lua_State* tolua_S)
{
	tolua_open(tolua_S);
	
	tolua_module(tolua_S,"fgui",0);
	tolua_beginmodule(tolua_S,"fgui");


	tolua_endmodule(tolua_S);
	return 1;
}

