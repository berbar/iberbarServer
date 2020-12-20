#pragma once


#include <iberbar/Lua/Headers.h>


namespace iberbar
{
	namespace Lua
	{
		static const char RefId_Mapping_Function[] = "iberbar_refid_mapping_function";
		static const char RefId_Mapping_Function_ExtParams[] = "iberbar_refid_mapping_function_extparams";

		// ��lua������Ϊ�ص���������c++
		__iberbarLuaApi__ lua_Integer ScriptCallback_ToFunction( lua_State* pLuaState, int nArg );
		// ��lua������Ϊ�ص���������c++��ͬʱ�������ĸ��Ӳ�����ԭ������Ϊ���������������õĲ����У�������self����
		// ������table����
		__iberbarLuaApi__ lua_Integer ScriptCallback_ToExtParam( lua_State* pLuaState, int nArg );
		// ����ص����������ڵ���
		__iberbarLuaApi__ bool ScriptCallback_PushFunction( lua_State* pLuaState, lua_Integer nLuaCallback );
		// ����ص������ĸ��Ӳ���
		__iberbarLuaApi__ bool ScriptCallback_PushExtParam( lua_State* pLuaState, lua_Integer nLuaExt );


		// ���׵Ļص���װ
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

