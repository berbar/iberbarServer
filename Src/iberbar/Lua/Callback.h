#pragma once


#include <iberbar/Lua/Headers.h>


namespace iberbar
{
	namespace Lua
	{
		static const char RefId_Mapping_Function[] = "iberbar_refid_mapping_function";
		static const char RefId_Mapping_Function_ExtParams[] = "iberbar_refid_mapping_function_extparams";

		// 将lua函数作为回调函数传到c++
		__iberbarLuaApi__ lua_Integer ScriptCallback_ToFunction( lua_State* pLuaState, int nArg );
		// 将lua函数作为回调函数传到c++的同时，附带的附加参数，原样的作为参数传到函数调用的参数中，可用于self传递
		// 仅用于table传递
		__iberbarLuaApi__ lua_Integer ScriptCallback_ToExtParam( lua_State* pLuaState, int nArg );
		// 推入回调函数，用于调用
		__iberbarLuaApi__ bool ScriptCallback_PushFunction( lua_State* pLuaState, lua_Integer nLuaCallback );
		// 推入回调函数的附加参数
		__iberbarLuaApi__ bool ScriptCallback_PushExtParam( lua_State* pLuaState, lua_Integer nLuaExt );


		// 简易的回调封装
		class __iberbarLuaApi__ CScriptCallbackHandler
			: public iberbar::CRef
		{
		public:
			CScriptCallbackHandler( lua_State* pLuaState, lua_Integer nLuaCallback, lua_Integer nLuaExt = 0 )
				: m_pLuaState( pLuaState )
				, m_nLuaCallback( nLuaCallback )
				, m_nLuaExt( nLuaExt )
			{
			}
			virtual ~CScriptCallbackHandler();

		public:
			bool PushFunction() const { return ScriptCallback_PushFunction( m_pLuaState, m_nLuaCallback ); }
			bool PushExt() const { return ScriptCallback_PushExtParam( m_pLuaState, m_nLuaExt ); }
			bool HasExt() const { return m_nLuaExt > 0; }
			lua_State* GetLuaState() const { return m_pLuaState; }
			lua_Integer GetCallback() const { return m_nLuaCallback; }

		private:
			lua_State* m_pLuaState;
			lua_Integer m_nLuaCallback;
			lua_Integer m_nLuaExt;
		};

		IBERBAR_UNKNOWN_PTR_DECLARE( CScriptCallbackHandler );
	}
}

#define lua_toluacallback( L, n ) iberbar::Lua::ScriptCallback_ToFunction( L, n )
#define lua_toluacallback_extparam( L, n ) iberbar::Lua::ScriptCallback_ToExtParam( L, n )

