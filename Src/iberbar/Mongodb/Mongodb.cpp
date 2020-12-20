
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN // 修正windows上socket头文件重复问题
#endif
#include "Mongodb.h"
#include <iberbar/Lua/CppBuilder.h>
#include <iberbar/Lua/CppInstantiateDef.h>
#include <iberbar/Lua/Error.h>
#include <iberbar/Utility/String.h>
#include <mongoc.h>





namespace iberbar
{

#define BSON_REAL 1
#define BSON_STRING 2
#define BSON_DOCUMENT 3
#define BSON_ARRAY 4
#define BSON_BINARY 5
#define BSON_UNDEFINED 6
#define BSON_OBJECTID 7
#define BSON_BOOLEAN 8
#define BSON_DATE 9
#define BSON_NULL 10
#define BSON_REGEX 11
#define BSON_DBPOINTER 12
#define BSON_JSCODE 13
#define BSON_SYMBOL 14
#define BSON_CODEWS 15
#define BSON_INT32 16
#define BSON_TIMESTAMP 17
#define BSON_INT64 18
#define BSON_MINKEY 255
#define BSON_MAXKEY 127





	//int LuaCppFunction_Bson_ToString( lua_State* pLuaState );


	namespace Mongodb
	{
		void Lua_Bson_UnpackDict( lua_State* pLuaState, bson_iter_t* bi, bool bIsArray );
		void Lua_Bson_Pack( lua_State* pLuaState, bson_t* pBson, int nTable, int nLen );
		void Lua_Bson_PackNode( lua_State* pLuaState, bson_t* pBson, int nArg, const char* strKey, int nKeyLen );
		void Lua_Bson_MakeObject( lua_State* L, int type, const void* ptr, size_t len );

		//int LuaCFunction_Bson_Decode( lua_State* pLuaState );
		//int LuaCFunction_Bson_Encode( lua_State* pLuaState );
		int LuaCFunction_Bson_Date( lua_State* pLuaState );
		int LuaCFunction_Bson_Timestamp( lua_State* pLuaState );
		int LuaCFunction_Bson_ObjectId( lua_State* pLuaState );

		int LuaCFunction_CreateClient( lua_State* pLuaState );

		int LuaCppFunction_Client_Destory( lua_State* pLuaState );
		int LuaCppFunction_Client_GetDatabase( lua_State* pLuaState );
		int LuaCppFunction_Client_GetCollection( lua_State* pLuaState );

		int LuaCppFunction_Database_Destory( lua_State* pLuaState );
		int LuaCppFunction_Database_GetCollection( lua_State* pLuaState );

		int LuaCppFunction_Collection_Destory( lua_State* pLuaState );
		int LuaCppFunction_Collection_Find( lua_State* pLuaState );
		int LuaCppFunction_Collection_InsertOne( lua_State* pLuaState );
		int LuaCppFunction_Collection_Delete( lua_State* pLuaState );
		int LuaCppFunction_Collection_DeleteOne( lua_State* pLuaState );

		int LuaCppFunction_Cursor_Destory( lua_State* pLuaState );
		int LuaCppFunction_Cursor_ToList( lua_State* pLuaState );
		int LuaCppFunction_Cursor_First( lua_State* pLuaState );
		int LuaCppFunction_Cursor_HasRecord( lua_State* pLuaState );

		extern const char s_ClassName_Client[] = "IClient";
		extern const char s_ClassName_Client_FullName[] = "iberbar.Mongodb.IClient";

		extern const char s_ClassName_Database[] = "IDatabase";
		extern const char s_ClassName_Database_FullName[] = "iberbar.Mongodb.IDatabase";

		extern const char s_ClassName_Collection[] = "ICollection";
		extern const char s_ClassName_Collection_FullName[] = "iberbar.Mongodb.ICollection";

		extern const char s_ClassName_Cursor[] = "ICursor";
		extern const char s_ClassName_Cursor_FullName[] = "iberbar.Mongodb.ICursor";
	}






}








//
//
//void iberbar::RegisterMongoLuaApis( lua_State* pLuaState )
//{
//	mongoc_init();
//
//	LuaCpp::CBuilder builder( pLuaState );
//	builder.ResolveScope(
//		[]( LuaCpp::CScopeBuilder* pScope )
//		{
//			pScope->AddClass( s_ClassName_MongoClient,
//				[]( const char*, LuaCpp::CClassBuilder* pClass )
//				{
//					pClass->AddConstructor( &LuaCpp::Class_Unknown_Constructor_New<CMongoClientRef> );
//					pClass->AddDistructor( &LuaCpp::Class_Unknown_Distructor_Release<CMongoClientRef> );
//					pClass->AddMemberMethod( "Connect", &LuaCppFunction_Client_Connect );
//					pClass->AddMemberMethod( "GetDatabase", &LuaCppFunction_Client_GetDatabase );
//					pClass->AddMemberMethod( "GetCollection", &LuaCppFunction_Client_GetCollection );
//				}
//			);
//
//
//			pScope->AddClass( s_ClassName_MongoDatabase,
//				[]( const char*, LuaCpp::CClassBuilder* pClass )
//				{
//					pClass->AddDistructor( &LuaCpp::Class_Unknown_Distructor_Release<CMongoDatabaseRef> );
//					pClass->AddMemberMethod( "GetCollection", &LuaCppFunction_Database_GetCollection );
//				}
//			);
//
//
//			pScope->AddClass( s_ClassName_MongoCollection,
//				[]( const char*, LuaCpp::CClassBuilder* pClass )
//				{
//					pClass->AddDistructor( &LuaCpp::Class_Unknown_Distructor_Release<CMongoCollectionRef> );
//					pClass->AddMemberMethod( "Find", &LuaCppFunction_Collection_Find );
//					pClass->AddMemberMethod( "InsertOne", &LuaCppFunction_Collection_InsertOne );
//					pClass->AddMemberMethod( "DeleteMany",&LuaCppFunction_Collection_Delete );
//					pClass->AddMemberMethod( "DeleteOne",&LuaCppFunction_Collection_DeleteOne );
//				}
//			);
//
//			pScope->AddClass( s_ClassName_MongoCursor,
//				[]( const char*, LuaCpp::CClassBuilder* pClass )
//				{
//					pClass->AddDistructor( &LuaCpp::Class_Default_Distructor_Delete<CMongoCursorRef> );
//					pClass->AddMemberMethod( "ToList", &LuaCppFunction_Cursor_ToList );
//					pClass->AddMemberMethod( "First", &LuaCppFunction_Cursor_First );
//					//pClass->AddMemberMethod( "HasRecord", &LuaCppFunction_Cursor_HasRecord );
//					//pClass->AddMemberMethod( "GetCount", &LuaCppFunction_Cursor_GetCount );
//				}
//			);
//
//		}, "iberbar.Mongo"
//	);
//
//	builder.ResolveScope(
//		[]( LuaCpp::CScopeBuilder* pScope )
//		{
//			pScope->AddFunctionOne( "Encode", LuaCFunction_Bson_Encode );
//			pScope->AddFunctionOne( "Decode", LuaCFunction_Bson_Decode );
//			pScope->AddFunctionOne( "Date", LuaCFunction_Bson_Date );
//			pScope->AddFunctionOne( "Timestamp", LuaCFunction_Bson_Timestamp );
//			pScope->AddFunctionOne( "ObjectId", LuaCFunction_Bson_ObjectId );
//
//			pScope->AddClass( s_ClassName_Bson,
//				[]( const char*, LuaCpp::CClassBuilder* pClass )
//				{
//					pClass->AddDistructor( &LuaCpp::Class_Unknown_Distructor_Release<CBsonRef> );
//					pClass->AddStandardMethod( "__tostring", &LuaCppFunction_Bson_ToString );
//				}
//			);
//		}, s_ModuleName_Bson
//	);
//}




//
//
//iberbar::CMongoClientRef::CMongoClientRef()
//	: m_pClient( nullptr )
//{
//}
//
//
//iberbar::CMongoClientRef::~CMongoClientRef()
//{
//	if ( m_pClient != nullptr )
//	{
//		mongoc_client_destroy( m_pClient );
//		m_pClient = nullptr;
//	}
//}
//
//
//void iberbar::CMongoClientRef::Connect( const char* strUri )
//{
//	m_pClient = mongoc_client_new( strUri );
//}
//
//
//
//
//
//
//
//iberbar::CMongoDatabaseRef::CMongoDatabaseRef( mongoc_database_t* pDatabase )
//	: m_pDatabase( pDatabase )
//{
//}
//
//
//iberbar::CMongoDatabaseRef::~CMongoDatabaseRef()
//{
//	if ( m_pDatabase != nullptr )
//	{
//		mongoc_database_destroy( m_pDatabase );
//		m_pDatabase = nullptr;
//	}
//}
//
//
//
//
//
//iberbar::CMongoCollectionRef::CMongoCollectionRef( mongoc_collection_t* pCollection )
//	: m_pCollection( pCollection )
//{
//}
//
//
//iberbar::CMongoCollectionRef::~CMongoCollectionRef()
//{
//	if ( m_pCollection != nullptr )
//	{
//		mongoc_collection_destroy( m_pCollection );
//		m_pCollection = nullptr;
//	}
//}
//
//
//
//
//
//
//iberbar::CMongoCursorRef::CMongoCursorRef( mongoc_cursor_t* pCursor )
//	: m_pCursor( pCursor )
//{
//}
//
//
//iberbar::CMongoCursorRef::~CMongoCursorRef()
//{
//	if ( m_pCursor != nullptr )
//	{
//		mongoc_cursor_destroy( m_pCursor );
//		m_pCursor = nullptr;
//	}
//}








//iberbar::CBsonRef::CBsonRef()
//{
//	bson_init( &m_Bson );
//}
//
//
//iberbar::CBsonRef::~CBsonRef()
//{
//	if ( m_Bson.len > 0 )
//	{
//		bson_destroy( &m_Bson );
//	}
//}












int iberbar::Mongodb::LuaCFunction_CreateClient( lua_State* pLuaState )
{
	int top = lua_gettop( pLuaState );
	if ( top != 1 )
		return iberbar_LuaError_ArgumentsCountIsNotMatch( pLuaState, 1 );

	const char* strUri = lua_tostring( pLuaState, 1 );
	if ( StringIsNullOrEmpty( strUri ) )
		return 0;

	mongoc_client_t* pClient = mongoc_client_new( strUri );

	if ( pClient == nullptr )
	{
		lua_pushnil( pLuaState );
		return 1;
	}

	lua_pushcppobject( pLuaState, s_ClassName_Client_FullName, pClient );

	return 1;
}






int iberbar::Mongodb::LuaCppFunction_Client_Destory( lua_State* L )
{
	int n = lua_gettop( L );

	if ( lua_isuserdata( L, 1 ) == false )
		return 0;

	Lua::UClassObjectType* s = (Lua::UClassObjectType*)lua_touserdata( L, 1 );
	if ( s == nullptr )
		return 0;

	if ( s->data != nullptr )
	{
		mongoc_client_destroy( (mongoc_client_t*)s->data );
		s->data = nullptr;
	}

	return 0;
}


int iberbar::Mongodb::LuaCppFunction_Client_GetDatabase( lua_State* pLuaState )
{
	int top = lua_gettop( pLuaState );
	if ( top != 2 )
		return iberbar_LuaError_ArgumentsCountIsNotMatch( pLuaState, 2 );

	mongoc_client_t* pClient = lua_tocppobject( pLuaState, 1, mongoc_client_t );
	if ( pClient == nullptr )
	{
		lua_pushnil( pLuaState );
		return 1;
	}

	const char* strName = lua_tostring( pLuaState, 2 );
	if ( StringIsNullOrEmpty( strName ) )
	{
		lua_pushnil( pLuaState );
		return 1;
	}

	mongoc_database_t* pDatabase = mongoc_client_get_database( pClient, strName );

	//TSmartRefPtr<CMongoDatabaseRef> pDatabaseRef = TSmartRefPtr<CMongoDatabaseRef>::_sNew( pDatabase );

	lua_pushcppobject( pLuaState, s_ClassName_Database_FullName, pDatabase );

	return 1;
}


int iberbar::Mongodb::LuaCppFunction_Client_GetCollection( lua_State* pLuaState )
{
	int top = lua_gettop( pLuaState );
	if ( top != 3 )
		return iberbar_LuaError_ArgumentsCountIsNotMatch( pLuaState, 3 );

	mongoc_client_t* pClient = lua_tocppobject( pLuaState, 1, mongoc_client_t );
	if (pClient == nullptr )
	{
		lua_pushnil( pLuaState );
		return 1;
	}

	const char* strDbName = lua_tostring( pLuaState, 2 );
	if ( StringIsNullOrEmpty( strDbName ) )
	{
		lua_pushnil( pLuaState );
		return 1;
	}

	const char* strCollectionName = lua_tostring( pLuaState, 3 );
	if ( StringIsNullOrEmpty( strCollectionName ) )
	{
		lua_pushnil( pLuaState );
		return 1;
	}

	mongoc_collection_t* pCollection = mongoc_client_get_collection( pClient, strDbName, strCollectionName );

	lua_pushcppobject( pLuaState, s_ClassName_Collection_FullName, pCollection );

	return 1;
}








int iberbar::Mongodb::LuaCppFunction_Database_Destory( lua_State* L )
{
	int n = lua_gettop( L );

	if ( lua_isuserdata( L, 1 ) == false )
		return 0;

	Lua::UClassObjectType* s = (Lua::UClassObjectType*)lua_touserdata( L, 1 );
	if ( s == nullptr )
		return 0;

	if ( s->data != nullptr )
	{
		mongoc_database_destroy( (mongoc_database_t*)s->data );
		s->data = nullptr;
	}

	return 0;
}


int iberbar::Mongodb::LuaCppFunction_Database_GetCollection( lua_State* pLuaState )
{
	int top = lua_gettop( pLuaState );
	if ( top != 2 )
		return iberbar_LuaError_ArgumentsCountIsNotMatch( pLuaState, 2 );

	mongoc_database_t* pDatabase = lua_tocppobject( pLuaState, 1, mongoc_database_t );
	if (pDatabase == nullptr )
	{
		lua_pushnil( pLuaState );
		return 1;
	}

	const char* strName = lua_tostring( pLuaState, 2 );
	if ( StringIsNullOrEmpty( strName ) )
	{
		lua_pushnil( pLuaState );
		return 1;
	}

	mongoc_collection_t* pCollection = mongoc_database_get_collection( pDatabase, strName );

	lua_pushcppobject( pLuaState, s_ClassName_Collection_FullName, pCollection );

	return 1;
}









int iberbar::Mongodb::LuaCppFunction_Collection_Destory( lua_State* L )
{
	int n = lua_gettop( L );

	if ( lua_isuserdata( L, 1 ) == false )
		return 0;

	Lua::UClassObjectType* s = (Lua::UClassObjectType*)lua_touserdata( L, 1 );
	if ( s == nullptr )
		return 0;

	if ( s->data != nullptr )
	{
		mongoc_collection_destroy( (mongoc_collection_t*)s->data );
		s->data = nullptr;
	}

	return 0;
}


int iberbar::Mongodb::LuaCppFunction_Collection_Find( lua_State* pLuaState )
{
	int top = lua_gettop( pLuaState );
	iberbar_LuaCheckArguments5( pLuaState, top, 1, 2, 3, 4, 5 );

	mongoc_collection_t* pCollection = lua_tocppobject( pLuaState, 1, mongoc_collection_t );
	if ( pCollection == nullptr )
	{
		lua_pushnil( pLuaState );
		return 1;
	}

	if ( lua_isnil( pLuaState, 2 ) && lua_istable( pLuaState, 2 ) )
	{
		lua_pushnil( pLuaState );
		return 1;
	}

	uint32 nSkip = 0;
	uint32 nLimit = 0;
	bson_t* pBsonQuery = nullptr;
	bson_t* pBsonFields = nullptr;
	bool bBsonQueryDestroy = false;
	bool bBsonFieldsDestroy = false;

	if ( lua_isnoneornil( pLuaState, 2 ) == false )
	{
		if ( lua_isuserdata( pLuaState, 2 ) == true )
		{
			luaL_error( pLuaState, "no CBsonRef" );
			//CBsonRef* pBsonRef = lua_tocppobject( pLuaState, 2, CBsonRef );
			//pBsonQuery = pBsonRef->Get();
		}
		else if ( lua_istable( pLuaState, 2 ) == true )
		{
			pBsonQuery = bson_new();
			bBsonQueryDestroy = true;
			Lua_Bson_Pack( pLuaState, pBsonQuery, 2, 0 );
		}
	}
	else
	{
		pBsonQuery = bson_new();
		bBsonQueryDestroy = true;
	}

	if ( lua_isnoneornil( pLuaState, 3 ) == false )
	{
		nSkip = (uint32)lua_tointeger( pLuaState, 3 );
	}

	if ( lua_isnoneornil( pLuaState, 4 ) == false )
	{
		nLimit = (uint32)lua_tointeger( pLuaState, 4 );
	}

	if ( lua_isnoneornil( pLuaState, 5 ) == false )
	{
		if ( lua_isuserdata( pLuaState, 5 ) == true )
		{
			luaL_error( pLuaState, "no CBsonRef" );
			/*CBsonRef* pBsonRef = lua_tocppobject( pLuaState, 2, CBsonRef );
			pBsonFields = pBsonRef->Get();*/
		}
		else if ( lua_istable( pLuaState, 5 ) == true )
		{
			pBsonFields = bson_new();
			bBsonFieldsDestroy = true;
			Lua_Bson_Pack( pLuaState, pBsonFields, 5, 0 );
		}
	}

	mongoc_cursor_t* pCursor = mongoc_collection_find( pCollection, MONGOC_QUERY_NONE, nSkip, nLimit, 0, pBsonQuery, pBsonFields, 0 );

	if ( bBsonQueryDestroy == true )
		bson_destroy( pBsonQuery );
	if ( bBsonFieldsDestroy == true )
		bson_destroy( pBsonFields );

	lua_pushcppobject( pLuaState, s_ClassName_Cursor_FullName, pCursor );

	return 1;
}


int iberbar::Mongodb::LuaCppFunction_Collection_InsertOne( lua_State* pLuaState )
{
	int top = lua_gettop( pLuaState );
	iberbar_LuaCheckArguments( pLuaState, top, 2 );

	mongoc_collection_t* pCollection = lua_tocppobject( pLuaState, 1, mongoc_collection_t );
	if ( pCollection == nullptr )
	{
		lua_pushboolean( pLuaState, false );
		return 1;
	}

	bool nRetInsert = false;
	if ( lua_isuserdata( pLuaState, 2 ) == true )
	{
		luaL_error( pLuaState, "no CBsonRef" );
		//CBsonRef* pBsonRef = lua_tocppobject( pLuaState, 2, CBsonRef );
		//if ( pBsonRef == nullptr )
		//{
		//	nRetInsert = false;
		//}
		//else
		//{
		//	nRetInsert = mongoc_collection_insert_one( pCollection, pBsonRef->Get(), nullptr, nullptr, nullptr );
		//}
	}
	else if ( lua_istable( pLuaState, 2 ) == true )
	{
		bson_t BsonDoc;
		bson_init( &BsonDoc );
		Lua_Bson_Pack( pLuaState, &BsonDoc, 2, 0 );

		nRetInsert = mongoc_collection_insert_one( pCollection, &BsonDoc, nullptr, nullptr, nullptr );

		bson_destroy( &BsonDoc );
	}
	else
	{
	}

	lua_pushboolean( pLuaState, nRetInsert );

	return 1;
}


int iberbar::Mongodb::LuaCppFunction_Collection_Delete( lua_State* pLuaState )
{
	int top = lua_gettop( pLuaState );
	iberbar_LuaCheckArguments( pLuaState, top, 2 );

	mongoc_collection_t* pCollection = lua_tocppobject( pLuaState, 1, mongoc_collection_t );
	if ( pCollection == nullptr )
	{
		lua_pushboolean( pLuaState, false );
		return 1;
	}

	bool nRet = false;
	if ( lua_isuserdata( pLuaState, 2 ) == true )
	{
		luaL_error( pLuaState, "no CBsonRef" );
		//CBsonRef* pBsonRef = lua_tocppobject( pLuaState, 2, CBsonRef );
		//if ( pBsonRef == nullptr )
		//{
		//	nRet = false;
		//}
		//else
		//{
		//	nRet = mongoc_collection_delete_many( pCollection, pBsonRef->Get(), nullptr, nullptr, nullptr );
		//}
	}
	else if ( lua_istable( pLuaState, 2 ) == true )
	{
		bson_t bsonSelector;
		bson_init( &bsonSelector );
		Lua_Bson_Pack( pLuaState, &bsonSelector, 2, 0 );

		nRet = mongoc_collection_delete_many( pCollection, &bsonSelector, nullptr, nullptr, nullptr );

		bson_destroy( &bsonSelector );
	}
	else
	{
	}

	lua_pushboolean( pLuaState, nRet );

	return 1;
}


int iberbar::Mongodb::LuaCppFunction_Collection_DeleteOne( lua_State* pLuaState )
{
	int top = lua_gettop( pLuaState );
	iberbar_LuaCheckArguments( pLuaState, top, 2 );

	mongoc_collection_t* pCollection = lua_tocppobject( pLuaState, 1, mongoc_collection_t );
	if ( pCollection == nullptr )
	{
		lua_pushboolean( pLuaState, false );
		return 1;
	}

	bool nRet = false;
	if ( lua_isuserdata( pLuaState, 2 ) == true )
	{
		luaL_error( pLuaState, "no CBsonRef" );
		//CBsonRef* pBsonRef = lua_tocppobject( pLuaState, 2, CBsonRef );
		//if ( pBsonRef == nullptr )
		//{
		//	nRet = false;
		//}
		//else
		//{
		//	nRet = mongoc_collection_delete_one( pCollection, pBsonRef->Get(), nullptr, nullptr, nullptr );
		//}
	}
	else if ( lua_istable( pLuaState, 2 ) == true )
	{
		bson_t BsonSelector;
		bson_init( &BsonSelector );
		Lua_Bson_Pack( pLuaState, &BsonSelector, 2, 0 );

		nRet = mongoc_collection_delete_one( pCollection, &BsonSelector, nullptr, nullptr, nullptr );

		bson_destroy( &BsonSelector );
	}
	else
	{
	}

	lua_pushboolean( pLuaState, nRet );

	return 1;
}









int iberbar::Mongodb::LuaCppFunction_Cursor_Destory( lua_State* L )
{
	int n = lua_gettop( L );

	if ( lua_isuserdata( L, 1 ) == false )
		return 0;

	Lua::UClassObjectType* s = (Lua::UClassObjectType*)lua_touserdata( L, 1 );
	if ( s == nullptr )
		return 0;

	if ( s->data != nullptr )
	{
		mongoc_cursor_destroy( (mongoc_cursor_t*)s->data );
		s->data = nullptr;
	}

	return 0;
}


int iberbar::Mongodb::LuaCppFunction_Cursor_ToList( lua_State* pLuaState )
{
	int top = lua_gettop( pLuaState );
	iberbar_LuaCheckArguments( pLuaState, top, 1 );

	mongoc_cursor_t* pCursor = lua_tocppobject( pLuaState, 1, mongoc_cursor_t );
	if ( pCursor == nullptr )
	{
		lua_pushnil( pLuaState );
		return 1;
	}

	const bson_t* pBsonDoc;
	bson_iter_t bi;
	lua_newtable( pLuaState );
	int nIndex = 0;
	while ( mongoc_cursor_next( pCursor, &pBsonDoc ) )
	{
		nIndex++;
		lua_pushinteger( pLuaState, nIndex );
		if ( bson_iter_init( &bi, pBsonDoc ) )
		{
			Lua_Bson_UnpackDict( pLuaState, &bi, false );
		}
		else
		{
			lua_pop( pLuaState, 1 );
		}
		lua_rawset( pLuaState, -3 );
	}

	return 1;
}


int iberbar::Mongodb::LuaCppFunction_Cursor_First( lua_State* pLuaState )
{
	int top = lua_gettop( pLuaState );
	iberbar_LuaCheckArguments( pLuaState, top, 1 );

	mongoc_cursor_t* pCursor = lua_tocppobject( pLuaState, 1, mongoc_cursor_t );
	if ( pCursor == nullptr )
	{
		lua_pushnil( pLuaState );
		return 1;
	}

	const bson_t* pBsonDoc;
	bson_iter_t bi;
	if ( mongoc_cursor_next( pCursor, &pBsonDoc ) == false )
	{
		lua_pushnil( pLuaState );
		return 1;
	}

	if ( bson_iter_init( &bi, pBsonDoc ) == false )
	{
		lua_pushnil( pLuaState );
		return 1;
	}

	Lua_Bson_UnpackDict( pLuaState, &bi, false );

	return 1;
}


int iberbar::Mongodb::LuaCppFunction_Cursor_HasRecord( lua_State* pLuaState )
{
	int top = lua_gettop( pLuaState );
	iberbar_LuaCheckArguments( pLuaState, top, 1 );

	mongoc_cursor_t* pCursor = lua_tocppobject( pLuaState, 1, mongoc_cursor_t );
	if ( pCursor == nullptr )
	{
		lua_pushboolean( pLuaState, 0 );
		return 1;
	}

	if ( mongoc_cursor_next( pCursor, nullptr ) == false )
	{
		lua_pushboolean( pLuaState, 0 );
		return 1;
	}

	lua_pushboolean( pLuaState, 1 );
	return 1;
}








void iberbar::Mongodb::Lua_Bson_UnpackDict( lua_State* pLuaState, bson_iter_t* bi, bool bIsArray )
{
	//bson_iter_t bi;
	bson_type_t bt;
	const char* strKey;
	lua_newtable( pLuaState );
	int nLuaTable = lua_gettop( pLuaState );
	while ( bson_iter_next( bi ) )
	{
		strKey = bson_iter_key( bi );
		bt = bson_iter_type( bi );
		if ( bIsArray == true )
			lua_pushinteger( pLuaState, (lua_Integer)strtol( strKey, nullptr, 10 ) + 1 );
		else
			lua_pushstring( pLuaState, strKey );

		switch ( bt )
		{
			case bson_type_t::BSON_TYPE_INT32:
			{
				lua_pushinteger( pLuaState, (lua_Integer)bson_iter_int32( bi ) );
				break;
			}

			case bson_type_t::BSON_TYPE_INT64:
			{
				lua_pushinteger( pLuaState, (lua_Integer)bson_iter_int64( bi ) );
				break;
			}

			case bson_type_t::BSON_TYPE_DOUBLE:
			{
				lua_pushnumber( pLuaState, (lua_Number)bson_iter_double( bi ) );
				break;
			}

			case bson_type_t::BSON_TYPE_BOOL:
			{
				lua_pushboolean( pLuaState, bson_iter_bool( bi ) );
				break;
			}

			case bson_type_t::BSON_TYPE_UTF8:
			{
				lua_pushstring( pLuaState, bson_iter_utf8( bi, nullptr ) );
				break;
			}

			case bson_type_t::BSON_TYPE_DATE_TIME:
			{
				int64_t date = bson_iter_date_time( bi );
				uint32_t v = date / 1000;
				Lua_Bson_MakeObject( pLuaState, BSON_DATE, &v, 4 );
				break;
			}

			case bson_type_t::BSON_TYPE_ARRAY:
			{
				bson_iter_t bi_child;
				if ( bson_iter_recurse( bi, &bi_child ) == true )
				{
					Lua_Bson_UnpackDict( pLuaState, &bi_child, true );
				}
				break;
			}

			case bson_type_t::BSON_TYPE_DOCUMENT:
			{
				bson_iter_t bi_child;
				if ( bson_iter_recurse( bi, &bi_child ) == true )
				{
					Lua_Bson_UnpackDict( pLuaState, &bi_child, false );
				}
				break;
			}

			default:
			{
				//luaL_error( pLuaState, "unsupport bson type: %d", (int)bt );
				lua_pop( pLuaState, 1 );
				continue;
			}
		}

		lua_rawset( pLuaState, -3 );
	}
}


void iberbar::Mongodb::Lua_Bson_Pack( lua_State* pLuaState, bson_t* pBson, int nTable, int nLen )
{
	if ( nLen > 0 )
	{
		char strKey[ 10 ];
		for ( int i = 1; i <= nLen; i++ )
		{
			lua_rawgeti( pLuaState, nTable, i );
			sprintf_s( strKey, "%d", i-1 );
			Lua_Bson_PackNode( pLuaState, pBson, -1, strKey, strlen(strKey) );
			lua_pop( pLuaState, 1 );
		}
	}
	else
	{
		lua_pushnil( pLuaState );
		const char* strKey;
		size_t nKeyLen;
		while ( lua_next( pLuaState, nTable ) )
		{

			// 现在的栈：-1 => value; -2 => key; index => table
			// 拷贝一份 key 到栈顶，然后对它做 lua_tostring 就不会改变原始的 key 值了
			lua_pushvalue( pLuaState, -2 );

			// 现在的栈：-1 => key; -2 => value; -3 => key; index => table
			strKey = lua_tolstring( pLuaState, -1, &nKeyLen );

			// 打包
			Lua_Bson_PackNode( pLuaState, pBson, -2, strKey, nKeyLen );

			// 弹出 value 和拷贝的 key，留下原始的 key 作为下一次 lua_next 的参数
			lua_pop( pLuaState, 2 );

			// 现在的栈：-1 => key; index => table
		}
	}
}


void iberbar::Mongodb::Lua_Bson_PackNode( lua_State* pLuaState, bson_t* pBson, int nArg, const char* strKey, int nKeyLen )
{
	int nLuaType = lua_type( pLuaState, nArg );
	switch ( nLuaType )
	{
		case LUA_TNUMBER:
		{
			if ( lua_isinteger( pLuaState, nArg ) == 1 )
			{
				bson_append_int64( pBson, strKey, nKeyLen, lua_tointeger( pLuaState, nArg ) );
			}
			else
			{
				bson_append_double( pBson, strKey, nKeyLen, lua_tonumber( pLuaState, nArg ) );
			}
			break;
		}

		case LUA_TBOOLEAN:
		{
			bson_append_bool( pBson, strKey, nKeyLen, ( lua_toboolean( pLuaState, nArg ) == 0 ) ? false : true );
			break;
		}

		case LUA_TSTRING:
		{
			size_t nLen = 0;
			const char* pData = lua_tolstring( pLuaState, nArg, &nLen );
			if ( nLen > 1 && pData[ 0 ] == 0 )
			{
				int nSubType = pData[ 1 ];
				switch ( nSubType )
				{
					case BSON_DATE:
					{
						int32* pVal = (int32*)(pData + 2);
						bson_append_date_time( pBson, strKey, nKeyLen, ((int64)(*pVal)) * 1000 );
						break;
					}
					default:
						break;
				}
			}
			else
			{
				bson_append_utf8( pBson, strKey, nKeyLen, pData, nLen );
			}
			break;
		}

		case LUA_TTABLE:
		{
			if ( nArg < 0 )
			{
				nArg = lua_gettop( pLuaState ) + nArg + 1;
			}

			lua_len( pLuaState, nArg );
			int nLen = lua_tointeger( pLuaState, -1 );
			lua_pop( pLuaState, 1 );
			if ( nLen > 0 )
			{
				bson_t BsonChild;
				if ( bson_append_array_begin( pBson, strKey, nKeyLen, &BsonChild ) == true )
				{
					Lua_Bson_Pack( pLuaState, &BsonChild, nArg, nLen );
					bson_append_array_end( pBson, &BsonChild );
				}
			}
			else
			{
				bson_t BsonChild;
				if ( bson_append_document_begin( pBson, strKey, nKeyLen, &BsonChild ) == true )
				{
					Lua_Bson_Pack( pLuaState, &BsonChild, nArg, 0 );
					bson_append_document_end( pBson, &BsonChild );
				}
			}
			break;
		}

		default:
			break;
	}
}


FORCEINLINE void iberbar::Mongodb::Lua_Bson_MakeObject( lua_State* pLuaState, int nType, const void* pData, size_t nDataLen )
{
	luaL_Buffer Buffer;
	luaL_buffinit( pLuaState, &Buffer );
	luaL_addchar( &Buffer, 0 );
	luaL_addchar( &Buffer, nType );
	luaL_addlstring( &Buffer, (const char*)pData, nDataLen );
	luaL_pushresult( &Buffer );
}

//
//int iberbar::LuaCFunction_Bson_Decode( lua_State* pLuaState )
//{
//	int top = lua_gettop( pLuaState );
//	iberbar_LuaCheckArguments( pLuaState, top, 1 );
//
//	CBsonRef* pBsonRef = lua_tocppobject( pLuaState, 1, CBsonRef );
//	if ( pBsonRef == nullptr )
//	{
//		lua_pushstring( pLuaState, "" );
//		return 1;
//	}
//
//	bson_iter_t bi;
//	if ( bson_iter_init( &bi, pBsonRef->Get() ) == true )
//	{
//		Lua_Bson_UnpackDict( pLuaState, &bi, false );
//	}
//	else
//	{
//		lua_pushnil( pLuaState );
//	}
//
//	return 1;
//}
//
//
//int iberbar::LuaCFunction_Bson_Encode( lua_State* pLuaState )
//{
//	int top = lua_gettop( pLuaState );
//	iberbar_LuaCheckArguments( pLuaState, top, 1 );
//
//	if ( lua_istable( pLuaState, 1 ) == false )
//	{
//		lua_pushnil( pLuaState );
//		return 1;
//	}
//
//	TSmartRefPtr<CBsonRef> pBsonRef = TSmartRefPtr<CBsonRef>::_sNew();
//	Lua_Bson_Pack( pLuaState, pBsonRef->Get(), 1, 0 );
//
//	lua_pushcppref( pLuaState, s_ClassName_Bson_FullName, pBsonRef );
//
//	return 1;
//}


int iberbar::Mongodb::LuaCFunction_Bson_Date( lua_State* pLuaState )
{
	int top = lua_gettop( pLuaState );
	iberbar_LuaCheckArguments( pLuaState, top, 1 );

	int nTimestamp = (int)lua_tointeger( pLuaState, 1 );
	luaL_Buffer b;
	luaL_buffinit( pLuaState, &b );
	luaL_addchar( &b, 0 );
	luaL_addchar( &b, BSON_DATE );
	luaL_addlstring( &b, (const char*)&nTimestamp, sizeof( nTimestamp ) );
	luaL_pushresult( &b );

	return 1;
}


int iberbar::Mongodb::LuaCFunction_Bson_Timestamp( lua_State* pLuaState )
{
	int nTimestamp = luaL_checkinteger( pLuaState, 1 );
	luaL_Buffer b;
	luaL_buffinit( pLuaState, &b );
	luaL_addchar( &b, 0 );
	luaL_addchar( &b, BSON_TIMESTAMP );
	if ( lua_isnoneornil( pLuaState, 2 ) ) {
		static uint32_t inc = 0;
		luaL_addlstring( &b, (const char*)&inc, sizeof( inc ) );
		++inc;
	}
	else {
		uint32_t i = (uint32_t)lua_tointeger( pLuaState, 2 );
		luaL_addlstring( &b, (const char*)&i, sizeof( i ) );
	}
	luaL_addlstring( &b, (const char*)&nTimestamp, sizeof( nTimestamp ) );
	luaL_pushresult( &b );

	return 1;
}


int iberbar::Mongodb::LuaCFunction_Bson_ObjectId( lua_State* pLuaState )
{
	return 0;
}

//
//int iberbar::Mongodb::LuaCppFunction_Bson_ToString( lua_State* pLuaState )
//{
//	int top = lua_gettop( pLuaState );
//	iberbar_LuaCheckArguments( pLuaState, top, 1 );
//
//	CBsonRef* pBsonRef = lua_tocppobject( pLuaState, 1, CBsonRef );
//	if ( pBsonRef == nullptr )
//	{
//		lua_pushstring( pLuaState, "" );
//		return 1;
//	}
//
//	char* str = bson_as_json( pBsonRef->Get(), nullptr );
//	lua_pushstring( pLuaState, str );
//	bson_free( str );
//
//	return 1;
//}




extern "C" int
#ifdef _WINDOWS
__declspec( dllexport )
#else
LUA_API
#endif
	luaopen_iberbar_Mongodb( lua_State* pLuaState )
	{
		using namespace iberbar::Mongodb;
		using namespace iberbar::Lua;

		mongoc_init();

		CBuilder builder( pLuaState );
		return builder.ResolveDllModule(
			[]( CScopeBuilder* pScope )
			{
				
				pScope->AddFunctionOne( "BsonDate", LuaCFunction_Bson_Date );
				pScope->AddFunctionOne( "BsonTimestamp", LuaCFunction_Bson_Timestamp );

				pScope->AddFunctionOne( "CreateClient", LuaCFunction_CreateClient );
				pScope->AddClass( s_ClassName_Client, []( const char*, CClassBuilder* pClass )
					{
						pClass->AddMemberMethod( "GetDatabase", &LuaCppFunction_Client_GetDatabase );
						pClass->AddMemberMethod( "GetCollection", &LuaCppFunction_Client_GetCollection );
						pClass->AddDistructor( &LuaCppFunction_Client_Destory );
					} );
				pScope->AddClass( s_ClassName_Database, []( const char*, CClassBuilder* pClass )
					{
						pClass->AddMemberMethod( "GetCollection", &LuaCppFunction_Database_GetCollection );
						pClass->AddDistructor( &LuaCppFunction_Database_Destory );
					} );
				pScope->AddClass( s_ClassName_Collection, []( const char*, CClassBuilder* pClass )
					{
						pClass->AddMemberMethod( "Find", &LuaCppFunction_Collection_Find );
						pClass->AddMemberMethod( "InsertOne", &LuaCppFunction_Collection_InsertOne );
						pClass->AddMemberMethod( "DeleteMany", &LuaCppFunction_Collection_Delete );
						pClass->AddMemberMethod( "DeleteOne", &LuaCppFunction_Collection_DeleteOne );
						pClass->AddDistructor( &LuaCppFunction_Collection_Destory );
					} );
				pScope->AddClass( s_ClassName_Cursor, []( const char*, CClassBuilder* pClass )
					{
						pClass->AddMemberMethod( "ToList", &LuaCppFunction_Cursor_ToList );
						pClass->AddMemberMethod( "First", &LuaCppFunction_Cursor_First );
						pClass->AddDistructor( &LuaCppFunction_Cursor_Destory );
					} );
			}, "iberbar.Mongodb"
		);
	}





