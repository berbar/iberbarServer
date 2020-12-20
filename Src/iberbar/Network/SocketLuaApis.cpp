
#include <iberbar/Network/SocketLuaApis.h>




iberbar::Net::CSocketDelegate_ForLua::CSocketDelegate_ForLua( lua_State* pLuaState )
	: m_pLuaThread( nullptr )
	, m_pLuaCallbackConnected( nullptr )
	, m_pLuaCallbackRead( nullptr )
	, m_pLuaCallbackClose( nullptr )
{
	//m_pLuaThread = lua_newthread( pLuaState );
	m_pLuaThread = pLuaState;
}


iberbar::Net::CSocketDelegate_ForLua::~CSocketDelegate_ForLua()
{
	m_pLuaCallbackConnected = nullptr;
	m_pLuaCallbackRead = nullptr;
	m_pLuaCallbackClose = nullptr;

/*	if ( m_pLuaThread != nullptr )
	{
		lua_close( m_pLuaThread );
		m_pLuaThread = nullptr;
	}*/	
}


void iberbar::Net::CSocketDelegate_ForLua::ExecuteConnected()
{
	if ( m_pLuaCallbackConnected == nullptr )
		return;

	if ( m_pLuaCallbackConnected->PushFunction() == true )
	{
		int nArgCount = 0;
		if ( m_pLuaCallbackConnected->HasExt() == true )
		{
			m_pLuaCallbackConnected->PushExt();
			nArgCount++;
		}

		int nRet = lua_pcallk( m_pLuaThread, nArgCount, 0, 0, 0, nullptr );
		if ( nRet != 0 )
		{
			int type = lua_type( m_pLuaThread, -1 );
			const char* err = lua_tostring( m_pLuaThread, -1 );
			lua_pop( m_pLuaThread, 1 );
			printf_s( err );
		}
	}
}


void iberbar::Net::CSocketDelegate_ForLua::ExecuteRead( const char* pData, size_t nDataLen )
{
	if ( m_pLuaCallbackRead == nullptr )
		return;

	if ( m_pLuaCallbackRead->PushFunction() == true )
	{
		int nArgCount = 1;
		if ( m_pLuaCallbackRead->HasExt() == true )
		{
			m_pLuaCallbackRead->PushExt();
			nArgCount++;
		}

		lua_pushlstring( m_pLuaThread, pData, nDataLen );

		int nRet = lua_pcallk( m_pLuaThread, nArgCount, 0, 0, 0, nullptr );
		if ( nRet != 0 )
		{
			int type = lua_type( m_pLuaThread, -1 );
			const char* err = lua_tostring( m_pLuaThread, -1 );
			lua_pop( m_pLuaThread, 1 );
			printf_s( err );
		}
	}
}



void iberbar::Net::CSocketDelegate_ForLua::ExecuteClose()
{
	if ( m_pLuaCallbackClose == nullptr )
		return;

	if ( m_pLuaCallbackClose->PushFunction() == true )
	{
		int nArgCount = 0;
		if ( m_pLuaCallbackClose->HasExt() == true )
		{
			m_pLuaCallbackClose->PushExt();
			nArgCount++;
		}

		int nRet = lua_pcallk( m_pLuaThread, nArgCount, 0, 0, 0, nullptr );
		if ( nRet != 0 )
		{
			int type = lua_type( m_pLuaThread, -1 );
			const char* err = lua_tostring( m_pLuaThread, -1 );
			lua_pop( m_pLuaThread, 1 );
			printf_s( err );
		}
	}
}




