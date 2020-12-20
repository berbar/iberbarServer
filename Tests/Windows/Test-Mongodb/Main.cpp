
#include <iberbar/Lua/Device.h>

int main()
{
	iberbar::Lua::CDevice* pLuaDevice = new iberbar::Lua::CDevice();
	pLuaDevice->Initial();
	pLuaDevice->AddLuaPath( "Scripts/?.lua" );
	//std::string st = IBERBAR_LUA_LIB_DIR;
	//st += "\\?.dll";
	//pLuaDevice->AddCLuaPath( st.c_str() );
	pLuaDevice->ExecuteFile( "Scripts/Main.lua" );
	pLuaDevice->Release();

	return 0;
}
