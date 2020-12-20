

#pragma once

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <iberbar/Utility/Ref.h>


#ifndef __iberbarLuaApi__
#	if defined ( _WINDOWS )
#		if defined ( __iberbarLuaApiExport__ )
#			define __iberbarLuaApi__ __declspec( dllexport )
#		else
#			define __iberbarLuaApi__ __declspec(dllimport)
#		endif
#	else
#		define __iberbarLuaApi__
#	endif
#endif


#ifndef LUA_TRUE
#define LUA_TRUE 1
#endif


#ifndef LUA_FALSE
#define LUA_FALSE 0
#endif
