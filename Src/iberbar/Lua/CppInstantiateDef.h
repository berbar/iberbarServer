#pragma once

#include <iberbar/Lua/CppInstantiate.h>


// 将C++对象压入lua栈
#define lua_pushcppobject( L, n, o ) iberbar::Lua::Class_Default_PushObjectWithName( L, n, o );

// 将C++ CRef对象压入lua栈
#define lua_pushcppref( L, n, o ) iberbar::Lua::Class_Ref_PushObjectWithName( L, n, o );

// 从lua栈获取C++对象
#define lua_tocppobject( L, arg, type )  iberbar::Lua::ToClassObject<type>( (L), (arg) )

#define lua_tocppobject_safe( L, arg, type, clsname )  iberbar::Lua::ToClassObject<type>( (L), (arg), (clsname) )

// 从lua栈将self转化为C++对象
#define lua_self_tocppobject( L, type ) iberbar::Lua::ToClassObject<type>( L, 1 );
