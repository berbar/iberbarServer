#pragma once

#include <iberbar/Lua/Headers.h>
#include <iberbar/Utility/Result.h>


namespace iberbar
{
	namespace Lua
	{
		class __iberbarLuaApi__ CDevice
			: public CRef
		{
		public:
			CDevice();
			~CDevice();



		public:
			void Initial();
			void Shutdown();
			void AddLuaPath( const char* strPath );
			void AddCLuaPath( const char* strPath );
			CResult ExecuteFile( const char* strFile );
			CResult ExecuteSource( const char* strSource );
			lua_State* GetLuaState() { return m_pLuaState; }
			

		private:
			lua_State* m_pLuaState;
			bool m_bShutdown;
		};


		IBERBAR_UNKNOWN_PTR_DECLARE( CDevice );

		CDevice* GetLuaDevice( lua_State* pLuaState );
	}
}

