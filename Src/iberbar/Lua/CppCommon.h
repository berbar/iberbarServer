#pragma once

//#include <iberbar/Lua/LuaBase.h>

namespace iberbar
{
	namespace Lua
	{
		// Class �������� CFoo
		static const char uClassReflection_ClassName[] = "__cpp_classname";

		// Class ������ģ�������� iberbar.Test
		static const char uClassReflection_ModuleName[] = "__cpp_modulename";

		// Class ȫ������ iberbar.Test.CFoo
		static const char uClassReflection_FullName[] = "__cpp_fullname";

		// Class ������Դ��1=C++��2=Lua
		static const char uClassReflection_CType[] = "__ctype";

		static const char uWeakTable_ForUserData[] = "__cpp_userdata";

		enum class UClassReflectionCType
		{
			Cpp = 1,
			Lua = 2
		};
	}
}
