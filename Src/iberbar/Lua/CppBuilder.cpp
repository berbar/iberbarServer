
#include <iberbar/Lua/CppBuilder.h>
#include <iberbar/lua/CppCommon.h>
#include <iberbar/Utility/String.h>


namespace iberbar
{
	namespace Lua
	{
		void BuildClassMetatable( lua_State* pLuaState, int nMetatable, const char* strClassName, const char* Extends, PHowToBuildClass pHowToAddClass );

		// ����Ϊ����
		// k  ��ʾtable.key��weak�ģ�Ҳ����table��keys���ܹ��������ռ����Զ����ա�
		// v  ��ʾtable.value��weak�ģ�Ҳ����table��values�ܹ��������ռ����Զ����ա�
		// kv �Ƕ��ߵ���ϣ��κ������ֻҪkey��value�е�һ���������ռ����Զ����գ���ôkv��ֵ�Ծͱ��ӱ����Ƴ���
		void CreateWeakTableForInstanceCache( lua_State* pLuaState, int nMetatable, const char* strCacheName, const char* strCacheMode );

		class CScopeGlobal
			: public CScopeBuilder
		{
		public:
			CScopeGlobal( lua_State* pLuaState );

		public:
			virtual void AddClass( const char* className, PHowToBuildClass pHowToAddClass, const char* extends = nullptr ) override;
			virtual void AddEnum( const char* strEnumTypeName, PHowToBuildEnum pHow, int nCount ) override;
			virtual void AddFunctionOne( const char* functionName, lua_CFunction pFunction ) override;
			virtual void AddFunctions( const luaL_Reg* pFunctionRegs ) override;
		};

		class CScopeModule
			: public CScopeBuilder
		{
		public:
			CScopeModule( lua_State* pLuaState, const char* moduleName );

		public:
			virtual void AddClass( const char* className, PHowToBuildClass pHowToAddClass, const char* extends = nullptr ) override;
			virtual void AddEnum( const char* strEnumTypeName, PHowToBuildEnum pHow, int nCount ) override;
			virtual void AddFunctionOne( const char* functionName, lua_CFunction pFunction ) override;
			virtual void AddFunctions( const luaL_Reg* pFunctionRegs ) override;

		public:
			CResult Build( PHowToBuildScope pHowToBuild );

		private:
			int m_table;
		};


		class CScopeDllBuilder
			: public CScopeBuilder
		{
		public:
			CScopeDllBuilder( lua_State* pLuaState, const char* strModuleName );

		public:
			virtual void AddClass( const char* className, PHowToBuildClass pHowToAddClass, const char* extends = nullptr ) override;
			virtual void AddEnum( const char* strEnumTypeName, PHowToBuildEnum pHow, int nCount ) override;
			virtual void AddFunctionOne( const char* functionName, lua_CFunction pFunction ) override;
			virtual void AddFunctions( const luaL_Reg* pFunctionRegs ) override;

		public:
			int Build( PHowToBuildScope pHowToBuild );

		private:
			int m_table;
		};

		class CTableScope
		{
		public:
			CTableScope( lua_State* pLuaState );
		public:
			CResult Next( const char* name );
			void Pop();
			int Table() { return m_table; }
		public:
			lua_State* m_pLuaState;
			int m_table;
			int m_depth;
		};

		// ����ǰ��Ҫ����ֵ���ú����Ὣkey��value����˳����ִ��lua_settable
		void SetTable( lua_State* L, int table_index, const char* key )
		{
			lua_pushstring( L, key );
			lua_insert( L, -2 );  // swap value and key
			lua_settable( L, table_index );
		}
	}
}







void iberbar::Lua::BuildClassMetatable( lua_State* pLuaState, int nMetatable, const char* strClassName, const char* Extends, PHowToBuildClass pHowToAddClass )
{

	// hide metatable from Lua getmetatable()
	// ����meta table��__metatable��
	//lua_pushvalue( L, methods );
	//SetTable( L, metatable, "__metatable" );

	// ���������棬vģʽ
	CreateWeakTableForInstanceCache( pLuaState, nMetatable, uWeakTable_ForUserData, "v" );


	//
	// �̳и���
	if ( StringIsNullOrEmpty( Extends ) == false )
	{
		lua_pushvalue( pLuaState, nMetatable );
		luaL_getmetatable( pLuaState, Extends );  // lookup metatable in Lua registry
		//if ( lua_isnil( L, -1 ) )
		//{
		//	luaL_error( L, "%s missing metatable", classFullName );
		//}
		lua_setmetatable( pLuaState, -2 );
		lua_pop( pLuaState, 1 );
	}

	// ����meta table��__index��
	// ˵����__index�������һ��������Ҳ������һ����
	// ������һ��������ʱ��Lua��table��ȱ�ٵ�����Ϊ
	// �����������������������һ�����ʱ��Lua������
	// �����п��Ƿ���ȱ�ٵ���
	lua_pushvalue( pLuaState, nMetatable );
	SetTable( pLuaState, nMetatable, "__index" );

	// ����Class��ģ����������
	//lua_pushstring( pLuaState, className );
	//SetTable( pLuaState, nMetatable, uClassReflection_ClassName );
	//lua_pushstring( pLuaState, m_name.c_str() );
	//SetTable( pLuaState, nMetatable, uClassReflection_ModuleName );
	lua_pushstring( pLuaState, strClassName );
	SetTable( pLuaState, nMetatable, uClassReflection_FullName );
	lua_pushinteger( pLuaState, (lua_Integer)UClassReflectionCType::Cpp );
	SetTable( pLuaState, nMetatable, uClassReflection_CType );

	// ����class
	CClassBuilder classBuilder( pLuaState, strClassName, strClassName, nMetatable, nMetatable );
	pHowToAddClass( strClassName, &classBuilder );

}


void iberbar::Lua::CreateWeakTableForInstanceCache( lua_State* pLuaState, int nMetatable, const char* strCacheName, const char* strCacheMode )
{
	lua_pushstring( pLuaState, strCacheName );
	lua_gettable( pLuaState, nMetatable );
	if ( lua_isnil( pLuaState, -1 ) )
	{
		lua_pop( pLuaState, 1 );
		lua_checkstack( pLuaState, 3 );

		lua_newtable( pLuaState );
		int nCacheTable = lua_gettop( pLuaState );

		// table is its own metatable
		// ����table��metatable���Լ�
		lua_pushvalue( pLuaState, nCacheTable );
		lua_setmetatable( pLuaState, -2 );

		// �ؼ��������Զ�����Ĺؼ�
		lua_pushstring( pLuaState, "__mode" );
		lua_pushstring( pLuaState, strCacheMode );
		lua_settable( pLuaState, -3 );   // metatable.__mode = mode

		lua_pushstring( pLuaState, strCacheName );
		lua_pushvalue( pLuaState, nCacheTable );
		lua_settable( pLuaState, nMetatable );
	}
	lua_pop( pLuaState, 1 );
}






iberbar::Lua::CScopeBuilder::CScopeBuilder( lua_State* pLuaState, const char* moduleName )
	: m_pLuaState( pLuaState )
	, m_name( moduleName )
{
}


iberbar::Lua::CBuilder::CBuilder( lua_State* pLuaState )
	: m_pLuaState( pLuaState )
{
}

void iberbar::Lua::CBuilder::ResolveScope( PHowToBuildScope pHowToBuildScope, const char* moduleName )
{
	if ( moduleName == nullptr )
	{
		CScopeGlobal scope( m_pLuaState );
		pHowToBuildScope( &scope );
	}
	else
	{
		CScopeModule scope( m_pLuaState, moduleName );
		scope.Build( pHowToBuildScope );
	}
}


int iberbar::Lua::CBuilder::ResolveDllModule( PHowToBuildScope pHowToBuildScope, const char* strModuleName )
{
	CScopeDllBuilder Scope( m_pLuaState, strModuleName );
	return Scope.Build( pHowToBuildScope );
}







iberbar::Lua::CScopeGlobal::CScopeGlobal( lua_State* pLuaState )
	: CScopeBuilder( pLuaState, "" )
{
}

void iberbar::Lua::CScopeGlobal::AddClass( const char* className, PHowToBuildClass pHowToAddClass, const char* extends )
{
	assert( m_pLuaState );
	assert( className );

	lua_State* L = m_pLuaState;
	//int methods;
	int metatable;

	// ����method table������
	//lua_newtable( L );
	//methods = lua_gettop( L );

	// ����metaԪ��
	// ˵����luaL_newmetatable��������һ���±�(�˱�������metatable), ���±��
	// ��ջ�����������registry��T::className����ϵ��
	luaL_newmetatable( L, className );
	metatable = lua_gettop( L );

	// store method table in globals so that
	// scripts can add functions written in Lua.
	// ��method table����ѹ��ջ����Ȼ�����õ�ȫ�������У�����ΪT::className
	lua_pushvalue( L, metatable );
	lua_setglobal( L, className );

	// ����metatable
	BuildClassMetatable( m_pLuaState, metatable, className, extends, pHowToAddClass );

	// ���� metatable and method ����table
	lua_pop( L, 1 );
}


void iberbar::Lua::CScopeGlobal::AddEnum( const char* strName, PHowToBuildEnum pHow, int nCount )
{
	CEnumBuilder EnumBuilder( m_pLuaState, nCount );
	pHow( &EnumBuilder );
	lua_setglobal( m_pLuaState, strName );
}


void iberbar::Lua::CScopeGlobal::AddFunctionOne( const char* functionName, lua_CFunction pFunction )
{
	lua_register( m_pLuaState, functionName, pFunction );
}

void iberbar::Lua::CScopeGlobal::AddFunctions( const luaL_Reg* pFunctionRegs )
{
	const luaL_Reg* pNode = pFunctionRegs;
	while ( pNode->name != nullptr )
	{
		lua_register( m_pLuaState, pNode->name, pNode->func );
		pNode ++;
	}
}









iberbar::Lua::CScopeModule::CScopeModule( lua_State* pLuaState, const char* moduleName )
	: CScopeBuilder( pLuaState, moduleName )
	, m_table( 0 )
{
}

void iberbar::Lua::CScopeModule::AddClass( const char* className, PHowToBuildClass pHowToAddClass, const char* extends )
{
	assert( m_pLuaState );
	assert( className );

	lua_State* L = m_pLuaState;
	//int methods;
	int metatable;

	// ����method table������
	//lua_newtable( L );
	//methods = lua_gettop( L );

	std::string classNameFull = m_name + "." + className;

	// ����metaԪ��
	// ˵����luaL_newmetatable��������һ���±�(�˱�������metatable), ���±��
	// ��ջ�����������registry��T::className����ϵ��
	luaL_newmetatable( L, classNameFull.c_str() );
	metatable = lua_gettop( L );

	// ��methods����ģ���table��
	lua_pushstring( L, className );
	lua_pushvalue( L, metatable );
	lua_settable( L, m_table );

	// ����metatable
	BuildClassMetatable( m_pLuaState, metatable, className, extends, pHowToAddClass );

	// ���� metatable ����table
	lua_pop( L, 1 );
}


void iberbar::Lua::CScopeModule::AddEnum( const char* strName, PHowToBuildEnum pHow, int nCount )
{
	lua_pushstring( m_pLuaState, strName );
	CEnumBuilder EnumBuilder( m_pLuaState, nCount );
	pHow( &EnumBuilder );
	lua_rawset( m_pLuaState, m_table );
}


void iberbar::Lua::CScopeModule::AddFunctionOne( const char* functionName, lua_CFunction pFunction )
{
	lua_pushstring( m_pLuaState, functionName );
	lua_pushcclosure( m_pLuaState, pFunction, 0 );
	lua_settable( m_pLuaState, m_table );
}

void iberbar::Lua::CScopeModule::AddFunctions( const luaL_Reg* pFunctionRegs )
{
	const luaL_Reg* pNode = pFunctionRegs;
	while ( pNode->name != nullptr )
	{
		lua_pushstring( m_pLuaState, pNode->name );
		lua_pushcclosure( m_pLuaState, pNode->func, 0 );
		lua_settable( m_pLuaState, m_table );
		pNode++;
	}
}

iberbar::CResult iberbar::Lua::CScopeModule::Build( PHowToBuildScope pHowToBuildScope )
{
	std::vector<std::string> namespaces;
	static const int s_maxNamespaceLength = 127;
	char name[s_maxNamespaceLength + 1];
	int n = 0;
	const char* ptr_iter = m_name.c_str();
	const char* ptr_last = m_name.c_str();
	while ( true )
	{
		if ( ptr_iter[0] == '.' )
		{
			if ( n == 0 )
				return MakeResult( ResultCode::Bad, "unexpect namesapce<%s>", m_name.c_str() );

			if ( n > s_maxNamespaceLength )
				return MakeResult( ResultCode::Bad, "namesapce<%s> is too long, must be %d", m_name.c_str(), s_maxNamespaceLength );

			memcpy_s( name, s_maxNamespaceLength, ptr_last, n );
			name[n] = 0;

			namespaces.push_back( name );

			n = 0;
			ptr_last = ptr_iter + 1;
		}
		else if ( ptr_iter[0] == 0 )
		{
			if ( n == 0 )
				return MakeResult( ResultCode::Bad, "unexpect namesapce<%s>", m_name.c_str() );

			if ( n > s_maxNamespaceLength )
				return MakeResult( ResultCode::Bad, "namesapce<%s> is too long, must be %d", m_name.c_str(), s_maxNamespaceLength );

			memcpy_s( name, s_maxNamespaceLength, ptr_last, n );
			name[n] = 0;
			namespaces.push_back( name );

			break;
		}
		ptr_iter++;
		n++;
	}

	int top = lua_gettop( m_pLuaState );
	CTableScope tableScope( m_pLuaState );
	auto namespaceIter = namespaces.begin();
	auto namespaceEnd = namespaces.end();
	for ( ; namespaceIter != namespaceEnd; namespaceIter++ )
	{
		tableScope.Next( namespaceIter->c_str() );
	}
	m_table = tableScope.Table();
	top = lua_gettop( m_pLuaState );

	pHowToBuildScope( this );

	top = lua_gettop( m_pLuaState );

	tableScope.Pop();
	top = lua_gettop( m_pLuaState );

	return CResult();
}








iberbar::Lua::CScopeDllBuilder::CScopeDllBuilder( lua_State* pLuaState, const char* strModuleName )
	: CScopeBuilder( pLuaState, strModuleName )
	, m_table( 0 )
{
}

void iberbar::Lua::CScopeDllBuilder::AddClass( const char* className, PHowToBuildClass pHowToAddClass, const char* extends )
{
	assert( m_pLuaState );
	assert( className );

	lua_State* L = m_pLuaState;
	//int methods;
	int metatable;

	// ����method table������
	//lua_newtable( L );
	//methods = lua_gettop( L );

	std::string classNameFull = m_name + "." + className;

	// ����metaԪ��
	// ˵����luaL_newmetatable��������һ���±�(�˱�������metatable), ���±��
	// ��ջ�����������registry��T::className����ϵ��
	luaL_newmetatable( L, classNameFull.c_str() );
	metatable = lua_gettop( L );

	// ��methods����ģ���table��
	lua_pushstring( L, className );
	lua_pushvalue( L, metatable );
	lua_settable( L, m_table );

	// ͬʱ
	//lua_pushvalue( L, metatable );
	//lua_setglobal( L, className );

	// ����metatable
	BuildClassMetatable( m_pLuaState, metatable, className, extends, pHowToAddClass );

	// ���� metatable ����table
	lua_pop( L, 1 );
}


void iberbar::Lua::CScopeDllBuilder::AddEnum( const char* strName, PHowToBuildEnum pHow, int nCount )
{
	lua_pushstring( m_pLuaState, strName );
	CEnumBuilder EnumBuilder( m_pLuaState, nCount );
	pHow( &EnumBuilder );
	lua_rawset( m_pLuaState, m_table );
}


void iberbar::Lua::CScopeDllBuilder::AddFunctionOne( const char* functionName, lua_CFunction pFunction )
{
	lua_pushstring( m_pLuaState, functionName );
	lua_pushcclosure( m_pLuaState, pFunction, 0 );
	lua_settable( m_pLuaState, m_table );
}

void iberbar::Lua::CScopeDllBuilder::AddFunctions( const luaL_Reg* pFunctionRegs )
{
	const luaL_Reg* pNode = pFunctionRegs;
	while ( pNode->name != nullptr )
	{
		lua_pushstring( m_pLuaState, pNode->name );
		lua_pushcclosure( m_pLuaState, pNode->func, 0 );
		lua_settable( m_pLuaState, m_table );
		pNode++;
	}
}

int iberbar::Lua::CScopeDllBuilder::Build( PHowToBuildScope pHowToBuildScope )
{
	lua_createtable( m_pLuaState, 0, 0 );
	m_table = lua_gettop( m_pLuaState );

	pHowToBuildScope( this );

	return 1;
}







iberbar::Lua::CTableScope::CTableScope( lua_State* pLuaState )
	: m_pLuaState( pLuaState )
	, m_table( 0 )
	, m_depth( 0 )
{
}

iberbar::CResult iberbar::Lua::CTableScope::Next( const char* name )
{
	if ( m_depth == 0 )
	{
		lua_getglobal( m_pLuaState, name );
		m_table = lua_gettop( m_pLuaState );
		if ( lua_isnil( m_pLuaState, m_table ) )
		{
			lua_pop( m_pLuaState, 1 );

			lua_newtable( m_pLuaState );
			m_table = lua_gettop( m_pLuaState );
			lua_pushvalue( m_pLuaState, -1 );
			lua_setglobal( m_pLuaState, name );
		}
		else if ( lua_istable( m_pLuaState, -1 ) == false )
		{
			lua_pop( m_pLuaState, 1 );
			return MakeResult( ResultCode::Bad, "%s exist but not module", name );
		}
	}
	else
	{
		int tableParent = m_table;
		lua_pushstring( m_pLuaState, name );
		lua_gettable( m_pLuaState, tableParent );
		m_table = lua_gettop( m_pLuaState );
		if ( lua_isnil( m_pLuaState, m_table ) )
		{
			lua_pop( m_pLuaState, 1 );

			lua_newtable( m_pLuaState );
			m_table = lua_gettop( m_pLuaState );

			lua_pushstring( m_pLuaState, name );
			lua_pushvalue( m_pLuaState, -2 );

			lua_settable( m_pLuaState, tableParent );
		}
		else if ( lua_istable( m_pLuaState, -1 ) == false )
		{
			lua_pop( m_pLuaState, 1 );
			return MakeResult( ResultCode::Bad, "%s exist but not module", name );
		}
	}

	m_depth++;

	return CResult();
}

void iberbar::Lua::CTableScope::Pop()
{
	if ( m_depth > 0 )
	{
		lua_pop( m_pLuaState, m_depth );
	}
}





iberbar::Lua::CClassBuilder::CClassBuilder( lua_State* pLuaState, const char* classNameFull, const char* className, int metatable, int methods )
	: m_pLuaState( pLuaState )
	, m_classNameFull( classNameFull )
	, m_className( className )
	, m_metatable( metatable )
	, m_methods( methods )
{
}

iberbar::Lua::CClassBuilder* iberbar::Lua::CClassBuilder::AddConstructor( lua_CFunction func )
{
	//lua_State* L = m_pLuaState;

	//lua_newtable( L );                // mt for method table
	//lua_pushcfunction( L, func );
	//lua_pushvalue( L, -1 );           // dup new_T function
	//SetTable( L, m_methods, "new" );         // add new_T to method table
	//SetTable( L, -3, "__call" );           // mt.__call = new_T
	//lua_setmetatable( L, m_methods );

	//lua_newtable( m_pLuaState );
	lua_pushstring( m_pLuaState, "new" );
	//lua_pushvalue( m_pLuaState, m_methods );
	lua_pushstring( m_pLuaState, m_classNameFull.c_str() );
	lua_pushcclosure( m_pLuaState, func, 1 );
	lua_settable( m_pLuaState, m_methods );

	return this;
}

iberbar::Lua::CClassBuilder* iberbar::Lua::CClassBuilder::AddDistructor( lua_CFunction func )
{
	AddStandardMethod( "__gc", func );

	return this;
}

iberbar::Lua::CClassBuilder* iberbar::Lua::CClassBuilder::AddMemberMethod( const char* name, lua_CFunction func, UpValue* upvalues, int upvaluesCount )
{
	assert( upvaluesCount >= 0 );

	lua_pushstring( m_pLuaState, name );

	// �հ��ľֲ�����
	if ( upvaluesCount > 0 )
	{
		for ( int i = 0; i < upvaluesCount; i++ )
		{
			if ( upvalues->t == UpValue_Boolean )
			{
				lua_pushboolean( m_pLuaState, upvalues->v.v_b );
			}
			else if ( upvalues->t == UpValue_Integer )
			{
				lua_pushinteger( m_pLuaState, upvalues->v.v_i );
			}
			else if ( upvalues->t == UpValue_Number )
			{
				lua_pushnumber( m_pLuaState, upvalues->v.v_n );
			}
			else if ( upvalues->t == UpValue_String )
			{
				lua_pushstring( m_pLuaState, upvalues->v.v_s );
			}
			else if ( upvalues->t == UpValue_Pointer )
			{
				lua_pushlightuserdata( m_pLuaState, upvalues->v.v_p );
			}
			else
			{
				lua_pushnil( m_pLuaState );
			}
			upvalues++;
		}
	}

	// ����c����thunk������1��upvalue������ջ�ϵ������upvalue
	// �˴���upvalue��lua_pushlightuserdata(L, (void*)l)ѹ���l
	lua_pushcclosure( m_pLuaState, func, upvaluesCount );

	lua_settable( m_pLuaState, m_methods );

	return this;
}

iberbar::Lua::CClassBuilder* iberbar::Lua::CClassBuilder::AddStaticMethod( const char* name, lua_CFunction func )
{
	lua_pushstring( m_pLuaState, name );
	lua_pushstring( m_pLuaState, m_classNameFull.c_str() );
	lua_pushcclosure( m_pLuaState, func, 1 );
	lua_settable( m_pLuaState, m_methods );
	//SetTable( m_pLuaState, m_metatable, name );

	return this;
}

iberbar::Lua::CClassBuilder* iberbar::Lua::CClassBuilder::AddStandardMethod( const char* name, lua_CFunction func )
{
	lua_pushcfunction( m_pLuaState, func );
	SetTable( m_pLuaState, m_metatable, name );

	return this;
}








iberbar::Lua::CEnumBuilder::CEnumBuilder( lua_State* pLuaState, int nValueCount )
	: m_pLuaState( pLuaState )
	, m_nTable( 0 )
{
	lua_createtable( m_pLuaState, 0, nValueCount > 0 ? nValueCount : 0 );
	m_nTable = lua_gettop( m_pLuaState );
}


void iberbar::Lua::CEnumBuilder::AddValueInt( const char* strKey, lua_Integer Value )
{
	lua_pushstring( m_pLuaState, strKey );
	lua_pushinteger( m_pLuaState, Value );
	lua_rawset( m_pLuaState, m_nTable );
}


void iberbar::Lua::CEnumBuilder::AddValueString( const char* strKey, const char* Value )
{
	lua_pushstring( m_pLuaState, strKey );
	lua_pushstring( m_pLuaState, Value );
	lua_rawset( m_pLuaState, m_nTable );
}

