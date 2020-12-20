#pragma once

#include <iberbar/Network/Headers.h>
#include <iberbar/Utility/Result.h>
#include <string>


namespace iberbar
{
	namespace Lua
	{
		class CDevice;
	}

	namespace Net
	{
		//class __iberbarNetworkApis__ ISocketServer abstract
		//{
		//public:
		//	virtual ~ISocketServer() {}
		//	virtual int Run() = 0;
		//};


		////class __iberbarServerApis__ ISocketDelegate abstract
		////{
		////public:
		////	virtual void OnRead( const char* pData, size_t nDataLen ) = 0;
		////	virtual void Send( const char* pData, size_t nDataLen ) = 0;

		////};

		//struct USocketServerOptions
		//{
		//	int nPort;
		//	std::string Head;
		//	std::string Tail;
		//	lua_State* pLuaState;
		//};

		//__iberbarNetworkApis__ ISocketServer* CreateSocketServer_UseLibevent( const USocketServerOptions& Options );


		__iberbarNetworkApis__ CResult NetRun( Lua::CDevice* pLuaDevice );
		__iberbarNetworkApis__ void NetShutdown();
	}
}

