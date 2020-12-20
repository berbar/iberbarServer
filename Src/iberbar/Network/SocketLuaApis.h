#pragma once

#include <iberbar/Lua/Callback.h>


namespace iberbar
{
	namespace Net
	{
        // Socket∏®◊Ù¿‡£¨luaΩ”»Î
        class CSocketDelegate_ForLua
        {
        public:
            CSocketDelegate_ForLua( lua_State* pLuaState );
            ~CSocketDelegate_ForLua();

        public:
            void SetCallbackConnected( iberbar::Lua::CScriptCallbackHandler* pCallback ) { m_pLuaCallbackConnected = pCallback; }
            void SetCallbackRead( iberbar::Lua::CScriptCallbackHandler* pCallback ) { m_pLuaCallbackRead = pCallback; }
            void SetCallbackClose( iberbar::Lua::CScriptCallbackHandler* pCallback ) { m_pLuaCallbackClose = pCallback; }

            void ExecuteConnected();
            void ExecuteRead( const char* pData, size_t nDataLen );
            void ExecuteClose();

            lua_State* GetLuaThread() { return m_pLuaThread; }

        private:
            lua_State* m_pLuaThread;
            iberbar::Lua::PTR_CScriptCallbackHandler m_pLuaCallbackConnected;
            iberbar::Lua::PTR_CScriptCallbackHandler m_pLuaCallbackRead;
            iberbar::Lua::PTR_CScriptCallbackHandler m_pLuaCallbackClose;
        };
	}
}
