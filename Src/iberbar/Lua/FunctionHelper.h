#pragma once


#include <iberbar/Lua/Headers.h>
#include <functional>


namespace iberbar
{
	namespace Lua
	{
		class __iberbarLuaApi__ CFunctionHelper
		{
		public:
			static void sExecuteFunction( lua_State* pLuaState, int nArgs );
			//void ExecuteFunction( int nArgs, int nReturns, std::function<void()> funcGetReturns );
			static void sExecuteGlobalFunction( lua_State* pLuaState, const char* strFunctionName );
		};
	}
}
