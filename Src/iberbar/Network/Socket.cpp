#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN // 修正windows上socket头文件重复问题
#endif
#include <iberbar/Network/Socket.h>
#include <iberbar/Network/Protocol.h>
#include <iberbar/Network/SocketLuaApis.h>

#include <iberbar/Lua/Device.h>
#include <iberbar/Lua/CppBuilder.h>
#include <iberbar/Lua/CppInstantiateDef.h>
#include <iberbar/Lua/Callback.h>
#include <iberbar/Lua/Error.h>

#include <iberbar/Utility/String.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
//#include <msgpack.hpp>

#include <assert.h>
#include <vector>
#include <string>
#include <functional>
#include <xmemory>
#include <memory_resource>
#include <csignal>


char TestBuffer[ 2048 ];



// string 使用 polymorphic_allocator 分配器
typedef std::basic_string<char, std::char_traits<char>, std::pmr::polymorphic_allocator<char> > SocketString;


namespace iberbar
{
    namespace Net
    {
        static const int SOCKET_MAX_SINGLE_READ = 1024;

        


        class CEngine;
        class CHostedService;
        class CSocketClient;
        class CSocketConnection;
        class CSocketDelegate_ForLua;
        class CUserEvent;


        IBERBAR_UNKNOWN_PTR_DECLARE( CHostedService );
        IBERBAR_UNKNOWN_PTR_DECLARE( CSocketClient );
        IBERBAR_UNKNOWN_PTR_DECLARE( CSocketConnection );
        IBERBAR_UNKNOWN_PTR_DECLARE( CUserEvent );



        class CAddrInfoHelper
        {
        public:
            CAddrInfoHelper();
            ~CAddrInfoHelper();

            const addrinfo* Set( const char* strNodeName, const char* pServiceName, const addrinfo* pHints );
            const addrinfo* Set_Socket( const std::string& url );
            const addrinfo* Get() const;

        private:
            addrinfo* m_pResult;
        };


        // libevent 核心
        class CEngine
        {
        public:
            CEngine();
            ~CEngine();

        public:
            CResult Initial( Lua::CDevice* pLuaDevice );
            void Run();
            void Shutdown();

            CResult AddEvent( CUserEvent** ppOutUserEvent );

            CResult AddHostedService( CHostedService** ppOutHostedService, int nPort, CProtocolBase* pProtocol );
            void RemoveHostedService( CHostedService* pHostedService );

            CResult AddSocketClient( CSocketClient** ppOutSocketClient, const char* strUrl, CProtocolBase* pProtocol );
            void RemoveSocketClient( CSocketClient* pSocketClient );

            // 内存池的分配器
            template < typename TElement >
            inline std::pmr::polymorphic_allocator<TElement> GetMemoryAllocator()
            {
                return std::pmr::polymorphic_allocator<TElement>( &m_Pool );
            }

        private:
            CResult CallLuaEntry();

        private:
            event_base* m_pEv2_Base;
            event* m_pEv2_Signal_Shutdown;

            Lua::PTR_CDevice m_pLuaDevice;
            Lua::PTR_CScriptCallbackHandler m_pScriptCallback_FileChanged;

            std::list<PTR_CHostedService> m_HostedServiceList;
            std::list<PTR_CSocketClient> m_SocketClientList;
            std::list<CUserEvent*> m_UserEventList;
            // 单线程内存池
            std::pmr::unsynchronized_pool_resource m_Pool;

#ifdef _WINDOWS
            bool m_bInitWindowsWS;
#endif


        private:
            static CEngine* sm_pInstance;
        public:
            static CEngine* sCreateInstance() { assert( sm_pInstance == nullptr ); sm_pInstance = new CEngine(); return sm_pInstance; }
            static void sDeleteInstance() { SAFE_DELETE( sm_pInstance ); }
            static CEngine* sGetInstance() { return sm_pInstance; }
        private:
            static void sOnSignalShutdown( evutil_socket_t, short nEvents, void* pUserData );
        };






        class CHostedService
            : public CRef
        {
        public:
            CHostedService( event_base* pEv2_Base, CProtocolBase* pProtocol );
            ~CHostedService();

        public:
            void SetLuaDevice( Lua::CDevice* pLuaDevice ) { m_pLuaDevice = pLuaDevice; }
            void SetLuaCallback_Accept( Lua::CScriptCallbackHandler* pCallback ) { m_pLuaCallback_Accept = pCallback; }
            CResult Create( int nPort );

        private:
            void Accept( evutil_socket_t nListener, short nEvents );
            void ExecuteLuaCallback_Accept( const char* strErr );
            void ExecuteLuaCallback_Accept( CSocketConnection* pConnection );

        public:
            void* operator new( size_t nSize );
            void operator delete( void* );

        private:
            event_base* m_pEv2_Base;
            event* m_pEv2_Listener;
            Lua::PTR_CDevice m_pLuaDevice;
            Lua::PTR_CScriptCallbackHandler m_pLuaCallback_Accept;
            int m_nPort;
            CProtocolBase* m_pProtocol;

        private:
            static void sOnEv2AcceptCallback( evutil_socket_t listener, short event, void* arg );
        };



        // Socket连接
        class CSocketConnection
            : public CRef
        {
        public:
            CSocketConnection( event_base* pEv2_Base, bufferevent* pEv2_BufferEvent, CProtocolBase* pProtocol );
            ~CSocketConnection();

        public:
            void SetLuaDevice( Lua::CDevice* pLuaDevice );
            void SetCallbackClose( std::function<void ( CSocketConnection* )> CallbackClose ) { m_CallbackClose = CallbackClose; }
            void SetEv2BufferOptions();
            void Close();
            void Send( const char* pData, size_t nDataLen );
            CSocketDelegate_ForLua* GetDelegateForLua() { return m_pDelegate; }

        private:
            void OnConnected();
            void OnRead( const char* pData, size_t nDataLen );
            // Socket Read方法，
            // 协议
            // -----------|-----------------|-----------|-----------
            //   head     |   len (4byte)   |   body    |    tail
            //
            // len = sizeof(body)
            void Read_1();

        public:
            void* operator new( size_t nSize );
            void operator delete( void* );

        private:
            event_base* m_pEv2_Base;
            bufferevent* m_pEv2_BufferEvent;
            intptr_t m_nFd; //socket描述符

            CProtocolBase* m_pProtocol;

            CSocketDelegate_ForLua* m_pDelegate;

            std::function<void ( CSocketConnection* )> m_CallbackClose;


        private:
            static void sOnEv2ReadCallback_0( struct bufferevent* bev, void* arg );
            static void sOnEv2ReadCallback_1( struct bufferevent* bev, void* arg );
            static void sOnEv2WriteCallback( bufferevent* pBufferEvent, void* pArg );
            static void sOnEv2ErrorCallback( struct bufferevent* bev, short event, void* arg );
        };




        class CSocketClient
            : public CRef
        {
        public:
            CSocketClient( event_base* pEv2_Base, const char* strUrl, CProtocolBase* pProtocol );
            virtual ~CSocketClient();

        public:
            void SetLuaDevice( Lua::CDevice* pLuaDevice ) { m_pLuaDevice = pLuaDevice; }
            CResult Create();
            CSocketConnection* GetConnection() { return m_pConnection; }

        private:
            void OnConnectionClose( CSocketConnection* pConnection );

        private:
            event_base* m_pEv2_Base;
            std::string m_strUrl;
            CProtocolBase* m_pProtocol;
            Lua::PTR_CDevice m_pLuaDevice;
            PTR_CSocketConnection m_pConnection;
        };




        class CUserEvent
            : public CRef
        {
        public:
            CUserEvent( event_base* pEv2_Base );
            ~CUserEvent();

            void Initial();
            void Destroy();
            void SetLuaCallback( Lua::PTR_CScriptCallbackHandler pLuaCallback ) { m_pLuaCallback = pLuaCallback; }
            void Active();

        private:
            event_base* m_pEv2_Base;
            event* m_pEv2_Event;
            Lua::PTR_CScriptCallbackHandler m_pLuaCallback;
            //std::function<void()> m_Executor;

        private:
            static void sEv2Callback( intptr_t, short nEvents, void* pUserData );
        };







        void RegisterSocketLuaApis_UseLibevent( lua_State* pLuaState );

        int LuaCFunction_CreateProtocol( lua_State* pLuaState );
        int LuaCFunction_AddEvent( lua_State* pLuaState );
        int LuaCFunction_DelEvent( lua_State* pLuaState );
        int LuaCFunction_CreateHostedService( lua_State* pLuaState );
        int LuaCFunction_CreateSocketClient( lua_State* pLuaState );
        int LuaCFunction_RemoveSocketClient( lua_State* pLuaState );

        int LuaCppFunction_HostedService_SetCallbackAccept( lua_State* pLuaState );
        int LuaCppFunction_SocketClient_GetConnection( lua_State* pLuaState );
        //int LuaCppFunction_HostedService_SetCallbackFileChanged( lua_State* pLuaState );
        //int LuaCppFunction_SocketServer_AddEvent( lua_State* pLuaState );
        //int LuaCppFunction_SocketServer_DelEvent( lua_State* pLuaState );
        //int LuaCppFunction_SocketServer_AddTimer( lua_State* pLuaState );
        //int LuaCppFunction_SocketServer_DelTimer( lua_State* pLuaState );
        int LuaCppFunction_SocketConnection_Send( lua_State* pLuaState );
        int LuaCppFunction_SocketConnection_Listen( lua_State* pLuaState );
        int LuaCppFunction_Event_SetCallback( lua_State* pLuaState );
        int LuaCppFunction_Event_Active( lua_State* pLuaState );

        void LuaCppCallbackExecute_Event( Lua::PTR_CScriptCallbackHandler pLuaCallback );


        const char s_ClassName_Protocol[] = "CProtocol";
        const char s_ClassName_Protocol_FullName[] = "iberbar.Net.CProtocol";

        const char s_ClassName_HostedService[] = "CHostedService";
        const char s_ClassName_HostedService_FullName[] = "iberbar.Net.CHostedService";

        const char s_ClassName_SocketClient[] = "CSocketClient";
        const char s_ClassName_SocketClient_FullName[] = "iberbar.Net.CSocketClient";

        const char s_ClassName_SocketConnection[] = "CSocketConnection";
        const char s_ClassName_SocketConnection_FullName[] = "iberbar.Net.CSocketConnection";

        const char s_ClassName_Event[] = "CEvent";
        const char s_ClassName_Event_FullName[] = "iberbar.Net.CEvent";
    }
}









inline iberbar::Net::CAddrInfoHelper::CAddrInfoHelper()
    : m_pResult( nullptr )
{
}


inline iberbar::Net::CAddrInfoHelper::~CAddrInfoHelper()
{
    if ( m_pResult != nullptr )
    {
        freeaddrinfo( m_pResult );
        m_pResult = nullptr;
    }
}


inline const addrinfo* iberbar::Net::CAddrInfoHelper::Set( const char* strNodeName, const char* pServiceName, const addrinfo* pHints )
{
    if ( m_pResult != nullptr )
    {
        freeaddrinfo( m_pResult );
        m_pResult = nullptr;
    }

    int c = getaddrinfo( strNodeName, pServiceName, pHints, &m_pResult );
    if ( c != 0 )
    {

    }

    return m_pResult;
}


inline const addrinfo* iberbar::Net::CAddrInfoHelper::Set_Socket( const std::string& url )
{
    size_t p = url.find( ':' );
    if ( p == std::string::npos || p == ( url.length() - 1 ) || p == 0 )
        return nullptr;
    std::string host = url.substr( 0, p );
    std::string port = url.substr( p + 1 );

    struct addrinfo hints;
    //struct addrinfo* res, * cur;
    memset( &hints, 0, sizeof( struct addrinfo ) );
    hints.ai_family = AF_INET;     /* Allow IPv4 */
    hints.ai_flags = AI_ALL;
    hints.ai_protocol = 0;         /* Any protocol */
    hints.ai_socktype = SOCK_STREAM;

    return Set( host.c_str(), port.c_str(), &hints );
}


inline const addrinfo* iberbar::Net::CAddrInfoHelper::Get() const
{
    return m_pResult;
}







iberbar::Net::CEngine* iberbar::Net::CEngine::sm_pInstance = nullptr;



//iberbar::Net::ISocketServer* iberbar::Net::CreateSocketServer_UseLibevent( const USocketServerOptions& Options )
//{
//    if ( Options.nPort == 0 )
//        return nullptr;
//
//    if ( Options.Head.size() == 0 || Options.Tail.size() == 0 )
//        return nullptr;
//
//    CSocketServer_UseLibevent* pServer = new CSocketServer_UseLibevent( Options );
//
//    RegisterSocketLuaApis_UseLibevent( Options.pLuaState );
//
//    pServer->CallLua();
//
//    return pServer;
//}


iberbar::CResult iberbar::Net::NetRun( Lua::CDevice* pLuaDevice )
{
    RegisterSocketLuaApis_UseLibevent( pLuaDevice->GetLuaState() );

    CResult Ret = CEngine::sCreateInstance()->Initial( pLuaDevice );
    if ( Ret.IsOK() == false )
    {
        CEngine::sDeleteInstance();
        return Ret;
    }

    CEngine::sGetInstance()->Run();

    CEngine::sDeleteInstance();

    printf_s( "exit NetRun" );

    return CResult();
}


void iberbar::Net::NetShutdown()
{
    CEngine::sGetInstance()->Shutdown();
    CEngine::sDeleteInstance();
}









iberbar::Net::CEngine::CEngine()
    : m_pEv2_Base( nullptr )
    , m_pEv2_Signal_Shutdown( nullptr )
    , m_pLuaDevice( nullptr )
    , m_pScriptCallback_FileChanged( nullptr )
    , m_UserEventList()
    , m_Pool()
    , m_bInitWindowsWS( false )
{

}


iberbar::Net::CEngine::~CEngine()
{
    m_HostedServiceList.clear();
    m_SocketClientList.clear();

    if ( m_pEv2_Base )
    {
        event_base_free( m_pEv2_Base );
        m_pEv2_Base = nullptr;
    }

    if ( m_UserEventList.empty() == false )
    {
        auto iter = m_UserEventList.begin();
        auto end = m_UserEventList.end();
        for ( ; iter != end; iter++ )
        {
            SAFE_DELETE( *iter );
        }
    }

#ifdef _WINDOWS
    if ( m_bInitWindowsWS == true )
    {
        WSACleanup();
    }
#endif
}


iberbar::CResult iberbar::Net::CEngine::Initial( Lua::CDevice* pLuaDevice )
{
    m_pLuaDevice = pLuaDevice;

#ifdef _WINDOWS
    WSADATA  Ws;
    //Init Windows Socket
    if ( WSAStartup( MAKEWORD( 2, 2 ), &Ws ) != 0 )
    {
        return MakeResult( ResultCode::Bad, "Failed to call WSAStartup" );
    }
#endif
    m_bInitWindowsWS = true;

    m_pEv2_Base = event_base_new();
    assert( m_pEv2_Base != NULL );

    m_pEv2_Signal_Shutdown = evsignal_new( m_pEv2_Base, SIGINT, sOnSignalShutdown, this );
    if ( !m_pEv2_Signal_Shutdown || event_add( m_pEv2_Signal_Shutdown, NULL ) < 0 )
    {
        return MakeResult( ResultCode::Bad, "Could not create/add a signal event!\n" );
    }

    // Lua 自定义入口
    CResult Ret = CallLuaEntry();
    if ( Ret.IsOK() == false )
    {
        return Ret;
    }

    return CResult();
}


void iberbar::Net::CEngine::Run()
{
    event_base_dispatch( m_pEv2_Base );
}


void iberbar::Net::CEngine::Shutdown()
{
    struct timeval delay = { 2, 0 };
    event_base_loopbreak( m_pEv2_Base );
    event_base_loopexit( m_pEv2_Base, &delay );
}


iberbar::CResult iberbar::Net::CEngine::AddEvent( CUserEvent** ppOutUserEvent )
{
    PTR_CUserEvent pUserEvent = PTR_CUserEvent::_sNew( m_pEv2_Base );
    pUserEvent->Initial();
    m_UserEventList.push_back( pUserEvent );

    ( *ppOutUserEvent ) = pUserEvent;
    ( *ppOutUserEvent )->AddRef();

    return CResult();
}


iberbar::CResult iberbar::Net::CEngine::AddHostedService( CHostedService** ppOutHostedService, int nPort, CProtocolBase* pProtocol )
{
    PTR_CHostedService pHostedService = PTR_CHostedService::_sNew( m_pEv2_Base, pProtocol );
    pHostedService->SetLuaDevice( m_pLuaDevice );
    CResult Ret = pHostedService->Create( nPort );
    if ( Ret.IsOK() == false )
    {
        return Ret;
    }

    m_HostedServiceList.push_back( pHostedService );

    ( *ppOutHostedService ) = pHostedService;
    ( *ppOutHostedService )->AddRef();

    return CResult();
}


void iberbar::Net::CEngine::RemoveHostedService( CHostedService* pHostedService )
{
    auto iter = m_HostedServiceList.begin();
    auto end = m_HostedServiceList.end();
    for ( ; iter != end; iter++ )
    {
        if ( iter->Equal( pHostedService ) )
        {
            m_HostedServiceList.erase( iter );
            break;
        }
    }
}


iberbar::CResult iberbar::Net::CEngine::AddSocketClient( CSocketClient** ppOutSocketClient, const char* strUrl, CProtocolBase* pProtocol )
{
    PTR_CSocketClient pClient = PTR_CSocketClient::_sNew( m_pEv2_Base, strUrl, pProtocol );
    pClient->SetLuaDevice( m_pLuaDevice );
    CResult Ret = pClient->Create();
    if ( Ret.IsOK() == false )
    {
        return Ret;
    }

    m_SocketClientList.push_back( pClient );

    ( *ppOutSocketClient ) = pClient;
    ( *ppOutSocketClient )->AddRef();

    return CResult();
}


void iberbar::Net::CEngine::RemoveSocketClient( CSocketClient* pClient )
{
    auto iter = m_SocketClientList.begin();
    auto end = m_SocketClientList.end();
    for ( ; iter != end; iter++ )
    {
        if ( iter->Equal( pClient ) )
        {
            m_SocketClientList.erase( iter );
            break;
        }
    }
}


iberbar::CResult iberbar::Net::CEngine::CallLuaEntry()
{
    lua_State* pLuaState = m_pLuaDevice->GetLuaState();
    lua_getglobal( pLuaState, "NetMain" );
    if ( lua_isfunction( pLuaState, -1 ) == 0 )
    {
        lua_pop( pLuaState, 1 );
        return MakeResult( ResultCode::Bad, "No net entry: NetMain" );
    }
    else
    {
        int nRet = lua_pcallk( pLuaState, 0, 0, 0, 0, nullptr );
        if ( nRet != 0 )
        {
            int type = lua_type( pLuaState, -1 );
            const char* err = lua_tostring( pLuaState, -1 );
            lua_pop( pLuaState, 1 );
            return MakeResult( ResultCode::Bad, err );
        }
    }

    return CResult();
}


void iberbar::Net::CEngine::sOnSignalShutdown( evutil_socket_t, short nEvents, void* pUserData )
{
    CEngine* pEngine = (CEngine*)pUserData;

    printf_s( "Caught an interrupt signal; exiting cleanly in two seconds.\n" );

    pEngine->Shutdown();
}









iberbar::Net::CHostedService::CHostedService( event_base* pEv2_Base, CProtocolBase* pProtocol )
    : m_pEv2_Base( pEv2_Base )
    , m_pEv2_Listener( nullptr )
    , m_pLuaCallback_Accept( nullptr )
    , m_nPort( 0 )
    , m_pProtocol( pProtocol )
{
    m_pProtocol->AddRef();
}


iberbar::Net::CHostedService::~CHostedService()
{
    if ( m_pEv2_Listener )
    {
        event_free( m_pEv2_Listener );
        m_pEv2_Listener = nullptr;
    }
    
    UNKNOWN_SAFE_RELEASE_NULL( m_pProtocol );
}


iberbar::CResult iberbar::Net::CHostedService::Create( int nPort )
{
    evutil_socket_t listener;
    listener = socket( AF_INET, SOCK_STREAM, 0 );
    assert( listener > 0 );
    evutil_make_listen_socket_reuseable( listener );
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons( (u_short)nPort );
    if ( bind( listener, (struct sockaddr*)&sin, sizeof( sin ) ) < 0 ) {
        perror( "bind" );
        return MakeResult( ResultCode::Bad, "" );
    }
    if ( listen( listener, 1000 ) < 0 ) {
        perror( "listen" );
        return MakeResult( ResultCode::Bad, "" );
    }

    printf_s( "Listening Port=%d...\n", nPort );

    evutil_make_socket_nonblocking( listener );
    m_pEv2_Listener = event_new( m_pEv2_Base, listener, EV_READ | EV_PERSIST, sOnEv2AcceptCallback, this );
    event_add( m_pEv2_Listener, NULL );

    m_nPort = nPort;

    return CResult();
}


void iberbar::Net::CHostedService::Accept( evutil_socket_t nListener, short nEvents )
{
    //UProtocol Protocol{ m_pHead, m_nHeadLen, m_pTail, m_nTailLen };

    evutil_socket_t fd;
    //声明地址
    struct sockaddr_in sin;
    //地址长度声明
    socklen_t slen = sizeof( sin );
    //接收客户端
    fd = accept( nListener, (struct sockaddr*)&sin, &slen );
    if ( fd < 0 )
    {
        ExecuteLuaCallback_Accept( "Error accept" );
        return;
    }
    printf_s( "ACCEPT: fd = %u\n", fd );

    //注册一个bufferevent_socket_new事件
    bufferevent* pBufferEvent = bufferevent_socket_new( m_pEv2_Base, fd, BEV_OPT_CLOSE_ON_FREE );

    // 创建虚拟的connection对象
    CSocketConnection* pSocketConn = new CSocketConnection( m_pEv2_Base, pBufferEvent, m_pProtocol );
    pSocketConn->SetLuaDevice( m_pLuaDevice );
    pSocketConn->SetEv2BufferOptions();
    
    ExecuteLuaCallback_Accept( pSocketConn );
}


void iberbar::Net::CHostedService::ExecuteLuaCallback_Accept( const char* strErr )
{
    lua_State* pLuaState = m_pLuaCallback_Accept->GetLuaState();
    if ( m_pLuaCallback_Accept != nullptr && m_pLuaCallback_Accept->PushFunction() )
    {
        int nArgCount = 1;

        if ( m_pLuaCallback_Accept->HasExt() )
        {
            m_pLuaCallback_Accept->PushExt();
            nArgCount++;
        }

        lua_pushstring( pLuaState, strErr );

        int nRet = lua_pcallk( pLuaState, nArgCount, 0, 0, 0, nullptr );
        if ( nRet != 0 )
        {
            int type = lua_type( pLuaState, -1 );
            const char* err = lua_tostring( pLuaState, -1 );
            lua_pop( pLuaState, 1 );
            printf_s( err );
        }
    }
}


void iberbar::Net::CHostedService::ExecuteLuaCallback_Accept( CSocketConnection* pConnection )
{
    lua_State* pLuaState = m_pLuaCallback_Accept->GetLuaState();
    if ( m_pLuaCallback_Accept != nullptr && m_pLuaCallback_Accept->PushFunction() )
    {
        int nArgCount = 1;

        if ( m_pLuaCallback_Accept->HasExt() )
        {
            m_pLuaCallback_Accept->PushExt();
            nArgCount++;
        }

        lua_pushcppobject( pLuaState, s_ClassName_SocketConnection_FullName, pConnection );

        int nRet = lua_pcallk( pLuaState, nArgCount, 0, 0, 0, nullptr );
        if ( nRet != 0 )
        {
            int type = lua_type( pLuaState, -1 );
            const char* err = lua_tostring( pLuaState, -1 );
            lua_pop( pLuaState, 1 );
            printf_s( err );
        }
    }
}


void* iberbar::Net::CHostedService::operator new( size_t n )
{
    assert( n > 0 );
    return CEngine::sGetInstance()->GetMemoryAllocator<CHostedService>().allocate( 1 );
}


void iberbar::Net::CHostedService::operator delete( void* p )
{
    CEngine::sGetInstance()->GetMemoryAllocator<CHostedService>().deallocate( (CHostedService*)p, 1 );
}


//accept回掉函数
void iberbar::Net::CHostedService::sOnEv2AcceptCallback( evutil_socket_t nListener, short nEvents, void* arg )
{
    ( (CHostedService*)arg )->Accept( nListener, nEvents );
}








iberbar::Net::CSocketClient::CSocketClient( event_base* pEv2_Base, const char* strUrl, CProtocolBase* pProtocol )
    : m_pEv2_Base( pEv2_Base )
    , m_strUrl( strUrl )
    , m_pProtocol( pProtocol )
    , m_pLuaDevice( nullptr )
    , m_pConnection( nullptr )
{
}


iberbar::Net::CSocketClient::~CSocketClient()
{
    UNKNOWN_SAFE_RELEASE_NULL( m_pProtocol );
}


iberbar::CResult iberbar::Net::CSocketClient::Create()
{
    CAddrInfoHelper AddrInfo;
    if ( AddrInfo.Set_Socket( m_strUrl ) == nullptr )
    {
        return MakeResult( ResultCode::Bad, "getaddrinfo failed" );
    }

    int opts = BEV_OPT_CLOSE_ON_FREE;
    //opts |= BEV_OPT_THREADSAFE;
    bufferevent* pEv2_BufferEvent = bufferevent_socket_new( m_pEv2_Base, -1, opts );
    if ( pEv2_BufferEvent == NULL )
    {
        return MakeResult( ResultCode::Bad, "socket init failed" );
    }

    //连接服务端
    int nConnRet = bufferevent_socket_connect( pEv2_BufferEvent, AddrInfo.Get()->ai_addr, sizeof( sockaddr ) );
    if ( -1 == nConnRet )
    {
        bufferevent_free( pEv2_BufferEvent );
        return MakeResult( ResultCode::Bad, "connect failed" );
    }

    m_pConnection = PTR_CSocketConnection::_sNew( m_pEv2_Base, pEv2_BufferEvent, m_pProtocol );
    m_pConnection->SetLuaDevice( m_pLuaDevice );
    m_pConnection->SetCallbackClose( std::bind( &CSocketClient::OnConnectionClose, this, std::placeholders::_1 ) );
    m_pConnection->SetEv2BufferOptions();

    return CResult();
}


void iberbar::Net::CSocketClient::OnConnectionClose( CSocketConnection* pConnection )
{
    if ( pConnection == m_pConnection )
    {
        m_pConnection = nullptr;
    }
}











iberbar::Net::CSocketConnection::CSocketConnection( event_base* pEv2_Base, bufferevent* pEv2_BufferEvent, CProtocolBase* pProtocol )
    : m_pEv2_Base( pEv2_Base )
    , m_pEv2_BufferEvent( pEv2_BufferEvent )
    , m_nFd( 0 )
    , m_pProtocol( pProtocol )
    , m_pDelegate( nullptr )
{
    m_pProtocol->AddRef();
}


iberbar::Net::CSocketConnection::~CSocketConnection()
{
    if ( m_pEv2_BufferEvent != nullptr )
    {
        bufferevent_free( m_pEv2_BufferEvent );
        m_pEv2_BufferEvent = nullptr;
    }

    UNKNOWN_SAFE_RELEASE_NULL( m_pProtocol );

    SAFE_DELETE( m_pDelegate );
}


void iberbar::Net::CSocketConnection::SetLuaDevice( Lua::CDevice* pDevice )
{
    m_pDelegate = new CSocketDelegate_ForLua( pDevice->GetLuaState() );
}


void iberbar::Net::CSocketConnection::SetEv2BufferOptions()
{
    bufferevent_data_cb pOnEv2ReadCallback = nullptr;
    if ( m_pProtocol->GetType() == UProtocolType::HEAD_AND_TAIL )
    {
        pOnEv2ReadCallback = sOnEv2ReadCallback_1;
    }
    //设置回掉函数
    bufferevent_setcb( m_pEv2_BufferEvent, pOnEv2ReadCallback, sOnEv2WriteCallback, sOnEv2ErrorCallback, this );
    //设置该事件的属性
    bufferevent_enable( m_pEv2_BufferEvent, EV_READ | EV_WRITE | EV_PERSIST );
    // 设置单次read的字节数
    bufferevent_set_max_single_read( m_pEv2_BufferEvent, SOCKET_MAX_SINGLE_READ );
    // 关联write到内存上的缓存区
    //evbuffer_add_reference( bufferevent_get_output( pBufferEvent ), nullptr, 2048, nullptr, nullptr );
}


void iberbar::Net::CSocketConnection::Close()
{
    if ( m_pEv2_BufferEvent != nullptr )
    {
        bufferevent_free( m_pEv2_BufferEvent );
        m_pEv2_BufferEvent = nullptr;
    }

    m_pDelegate->ExecuteClose();

    if ( m_CallbackClose )
    {
        m_CallbackClose( this );
    }
}


void iberbar::Net::CSocketConnection::Send( const char* pData, size_t nDataLen )
{
    
    if ( m_pProtocol->GetType() == UProtocolType::HEAD_AND_TAIL )
    {
        CProtocol_HeadAndTail* pProtocol = (CProtocol_HeadAndTail*)m_pProtocol;

        const char* pHead = pProtocol->Head();
        const char* pTail = pProtocol->Tail();
        size_t nHeadLen = pProtocol->HeadLen();
        size_t nTailLen = pProtocol->TailLen();

        size_t nBufferSize = nHeadLen + nDataLen + sizeof( uint32 ) + nTailLen;
        SocketString message( CEngine::sGetInstance()->GetMemoryAllocator<char>() );
        message.resize( nBufferSize );
        char* ptr = &message.front();

        memcpy_s( ptr, nBufferSize, pHead, nHeadLen );
        ptr += nHeadLen;

        ( *( (uint32*)ptr ) ) = (uint32)nDataLen;
        ptr += sizeof( uint32 );

        memcpy_s( ptr, nDataLen, pData, nDataLen );
        ptr += nDataLen;

        memcpy_s( ptr, nTailLen, pTail, nTailLen );

        int nRet = bufferevent_write( m_pEv2_BufferEvent, message.c_str(), nBufferSize );
    }

}


void iberbar::Net::CSocketConnection::OnConnected()
{
    m_pDelegate->ExecuteConnected();
}


void iberbar::Net::CSocketConnection::OnRead( const char* pData, size_t nDataLen )
{
    m_pDelegate->ExecuteRead( pData, nDataLen );
}


void iberbar::Net::CSocketConnection::Read_1()
{
    evbuffer* buf = bufferevent_get_input( m_pEv2_BufferEvent );
    int len = evbuffer_get_length( buf );
    //printf( "len=%d\n", len );

    CProtocol_HeadAndTail* pProtocol = (CProtocol_HeadAndTail*)m_pProtocol;

    const char* pHead = pProtocol->Head();
    const char* pTail = pProtocol->Tail();
    size_t nHeadLen = pProtocol->HeadLen();
    size_t nTailLen = pProtocol->TailLen();

    //evbuffer_iovec iovec[2];
    //int nChainCount = evbuffer_peek( buf, -1, nullptr, iovec, 2 );
    //printf( "len=%d,chain=%d\n", (int)iovec->iov_len, nChainCount );
    //if ( nChainCount == 2 )
    //{
    //}

    //const char* pFd = (const char*)iovec[ 0 ].iov_base;
    //size_t nFdLen = iovec[ 0 ].iov_len;

    //
    // 由于peek出来的chain可能不止一条，暂时粗暴的copyout，迟点可能改成peek配合部分copyout
    //
    SocketString MemoryBuffer( CEngine::sGetInstance()->GetMemoryAllocator<char>() );
    MemoryBuffer.resize( len );
    evbuffer_copyout( buf, &MemoryBuffer.front(), len );

    const char* pFd = MemoryBuffer.c_str();
    size_t nFdLen = MemoryBuffer.size();
    int nHeadPos, nHeadFindStart = 0, nMaybe;
    int nTempLen;
    unsigned int nBodyLen;
    size_t nDrain = 0;
    while ( ( nHeadPos = SocketProtocol_FindHead( pFd, nFdLen, nHeadFindStart, pHead, nHeadLen, nMaybe ) ) >= 0 )
    {
        nBodyLen = *( (unsigned int*)( pFd + nHeadPos + nHeadLen ) );

        nTempLen = nHeadPos + nHeadLen + 4;
        // len不完整
        if ( nTempLen > nFdLen )
        {
            nMaybe = nHeadPos;
            break;
        }

        nTempLen += nBodyLen;
        // body不完整
        if ( nTempLen > nFdLen )
        {
            nMaybe = nHeadPos;
            break;
        }

        nTempLen += nTailLen;
        // tail不完整
        if ( nTempLen > nFdLen )
        {
            nMaybe = nHeadPos;
            break;
        }

        if ( memcmp( pFd + ( nTempLen - nTailLen ), pTail, nTailLen ) == 0 )
        {
            OnRead( pFd + ( nTempLen - nTailLen - nBodyLen ), nBodyLen );
        }

        nHeadFindStart += nTempLen;
    }

    if ( nMaybe < 0 )
    {
        evbuffer_drain( buf, nFdLen );
    }
    else if ( nMaybe > 0 )
    {
        evbuffer_drain( buf, nMaybe );
    }
}


void* iberbar::Net::CSocketConnection::operator new( size_t n )
{
    assert( n > 0 );
    return CEngine::sGetInstance()->GetMemoryAllocator<CSocketConnection>().allocate( 1 );
}


void iberbar::Net::CSocketConnection::operator delete( void* p )
{
    CEngine::sGetInstance()->GetMemoryAllocator<CSocketConnection>().deallocate( (CSocketConnection*)p, 1 );
}


////read 回调函数
void iberbar::Net::CSocketConnection::sOnEv2ReadCallback_1( struct bufferevent* bev, void* arg )
{
    ( (CSocketConnection*)arg )->Read_1();    
}


void iberbar::Net::CSocketConnection::sOnEv2WriteCallback( bufferevent* pBufferEvent, void* pArg )
{
    //int fd = bufferevent_getfd( pBufferEvent );
    //printf_s( "write_cb:%d\n", fd );
}


////error回调函数
void iberbar::Net::CSocketConnection::sOnEv2ErrorCallback( struct bufferevent* bev, short event, void* arg )
{
    //通过传入参数bev找到socket fd
    evutil_socket_t fd = bufferevent_getfd( bev );
    //cout << "fd = " << fd << endl;
    if ( event & BEV_EVENT_TIMEOUT )
    {
        printf_s( "Timed out\n" ); //if bufferevent_set_timeouts() called
        ( (CSocketConnection*)arg )->Close();
    }
    else if ( event & BEV_EVENT_EOF )
    {
        printf_s( "connection closed\n" );
        ( (CSocketConnection*)arg )->Close();
    }
    else if ( event & BEV_EVENT_ERROR )
    {
        printf_s( "some other error\n" );
        ( (CSocketConnection*)arg )->Close();
    }
    else if ( event & BEV_EVENT_CONNECTED )
    {
        printf_s( "connection connected\n" );
        ( (CSocketConnection*)arg )->OnConnected();
    }
}









iberbar::Net::CUserEvent::CUserEvent( event_base* pEv2_Base )
    : m_pEv2_Base( pEv2_Base )
    , m_pEv2_Event( nullptr )
    , m_pLuaCallback( nullptr )
{
}


iberbar::Net::CUserEvent::~CUserEvent()
{
    Destroy();
}


void iberbar::Net::CUserEvent::Initial()
{
    m_pEv2_Event = event_new( m_pEv2_Base, -1, EV_PERSIST, sEv2Callback, this );
    event_add( m_pEv2_Event, nullptr );
}


void iberbar::Net::CUserEvent::Destroy()
{
    if ( m_pEv2_Event != nullptr )
    {
        event_free( m_pEv2_Event );
        m_pEv2_Event = nullptr;
    }
}


void iberbar::Net::CUserEvent::Active()
{
    event_active( m_pEv2_Event, EV_SIGNAL, 0 );
}


void iberbar::Net::CUserEvent::sEv2Callback( intptr_t, short nEvents, void* pUserData )
{
    CUserEvent* pEvent = (CUserEvent*)pUserData;
    LuaCppCallbackExecute_Event( pEvent->m_pLuaCallback );
}









void iberbar::Net::RegisterSocketLuaApis_UseLibevent( lua_State* pLuaState )
{
    Lua::CBuilder builder( pLuaState );

    builder.ResolveScope(
        []( Lua::CScopeBuilder* pScope )
        {
            pScope->AddFunctionOne( "CreateProtocol", &LuaCFunction_CreateProtocol );
            pScope->AddFunctionOne( "AddEvent", &LuaCFunction_AddEvent );
            pScope->AddFunctionOne( "CreateHostedService", &LuaCFunction_CreateHostedService );
            pScope->AddFunctionOne( "CreateSocketClient", &LuaCFunction_CreateSocketClient );
            pScope->AddFunctionOne( "RemoveSocketClient", &LuaCFunction_RemoveSocketClient );

            pScope->AddClass( s_ClassName_Protocol,
                []( const char*, Lua::CClassBuilder* pClass )
                {
                    pClass->AddDistructor( &Lua::Class_Unknown_Distructor_Release<CProtocolBase> );
                } );

            pScope->AddClass( s_ClassName_HostedService,
                []( const char*, Lua::CClassBuilder* pClass )
                {
                    pClass->AddDistructor( &Lua::Class_Unknown_Distructor_Release<CHostedService> );
                    pClass->AddMemberMethod( "SetCallbackAccept", &LuaCppFunction_HostedService_SetCallbackAccept );
                }
            );

            pScope->AddClass( s_ClassName_SocketClient,
                []( const char*, Lua::CClassBuilder* pClass )
                {
                    pClass->AddDistructor( &Lua::Class_Unknown_Distructor_Release<CSocketClient> );
                    pClass->AddMemberMethod( "GetConnection", &LuaCppFunction_SocketClient_GetConnection );
                } );

            pScope->AddClass( s_ClassName_SocketConnection,
                []( const char*, Lua::CClassBuilder* pClass )
                {
                    pClass->AddDistructor( &Lua::Class_Unknown_Distructor_Release<CSocketConnection> );
                    pClass->AddMemberMethod( "Send", &LuaCppFunction_SocketConnection_Send );
                    pClass->AddMemberMethod( "Listen", &LuaCppFunction_SocketConnection_Listen );
                }
            );

            pScope->AddClass( s_ClassName_Event,
                []( const char*, Lua::CClassBuilder* pClass )
                {
                    pClass->AddDistructor( &Lua::Class_Unknown_Distructor_Release<CUserEvent> );
                    pClass->AddMemberMethod( "SetCallback", &LuaCppFunction_Event_SetCallback );
                    pClass->AddMemberMethod( "Active", &LuaCppFunction_Event_Active );
                } );

        }, "iberbar.Net"
    );
}







int iberbar::Net::LuaCppFunction_HostedService_SetCallbackAccept( lua_State* pLuaState )
{
    int top = lua_gettop( pLuaState );
    if ( top != 2 && top != 3 )
        return iberbar_LuaError_ArgumentsCountIsNotMatch2( pLuaState, 2, 3 );

    CHostedService* pHostedService = lua_tocppobject( pLuaState, 1, CHostedService );
    if ( pHostedService == nullptr )
        return 0;

    lua_Integer nFunc = lua_toluacallback( pLuaState, 2 );
    lua_Integer nExt = 0;
    if ( top == 3 )
    {
        nExt = lua_toluacallback_extparam( pLuaState, 3 );
    }
    Lua::PTR_CScriptCallbackHandler pCallback = Lua::PTR_CScriptCallbackHandler::_sNew( pLuaState, nFunc, nExt );
    pHostedService->SetLuaCallback_Accept( pCallback );

    return 0;
}










int iberbar::Net::LuaCppFunction_SocketClient_GetConnection( lua_State* pLuaState )
{
    int nTop = lua_gettop( pLuaState );
    iberbar_LuaCheckArguments( pLuaState, nTop, 1 );

    CSocketConnection* pConnection = nullptr;
    CSocketClient* pClient = lua_tocppobject( pLuaState, 1, CSocketClient );
    if ( pClient == nullptr || ( pConnection = pClient->GetConnection() ) == nullptr )
    {
        lua_pushnil( pLuaState );
        return 1;
    }

    lua_pushcppref( pLuaState, s_ClassName_SocketConnection_FullName, pConnection );

    return 1;
}








//int iberbar::Net::LuaCppFunction_SocketServer_SetCallbackFileChanged( lua_State* pLuaState )
//{
//    int top = lua_gettop( pLuaState );
//    if ( top != 2 && top != 3 )
//        return iberbar_LuaError_ArgumentsCountIsNotMatch2( pLuaState, 2, 3 );
//
//    CSocketServer_UseLibevent* pServer = lua_tocppobject( pLuaState, 1, CSocketServer_UseLibevent );
//    if ( pServer == nullptr )
//        return 0;
//
//    lua_Integer nFunc = lua_toluacallback( pLuaState, 2 );
//    lua_Integer nExt = 0;
//    if ( top == 3 )
//    {
//        nExt = lua_toluacallback_extparam( pLuaState, 3 );
//    }
//    Lua::PTR_CScriptCallbackHandler pCallback = Lua::PTR_CScriptCallbackHandler::_sNew( pLuaState, nFunc, nExt );
//    pServer->SetLuaCallback_FileChanged( pCallback );
//
//    return 0;
//}


int iberbar::Net::LuaCFunction_CreateProtocol( lua_State* pLuaState )
{
    int nTop = lua_gettop( pLuaState );
    UProtocolType nType = (UProtocolType)lua_tointeger( pLuaState, 1 );
    TSmartRefPtr<CProtocolBase> pProtocol = nullptr;
    if ( nType == UProtocolType::HEAD_AND_TAIL )
    {
        iberbar_LuaCheckArguments( pLuaState, nTop, 3 );
        const char* pHead = lua_tostring( pLuaState, 2 );
        const char* pTail = lua_tostring( pLuaState, 3 );
        if ( StringIsNullOrEmpty( pHead ) || StringIsNullOrEmpty( pTail ) )
        {
            lua_pushnil( pLuaState );
            return 1;
        }

        pProtocol = new CProtocol_HeadAndTail( pHead, strlen( pHead ), pTail, strlen( pTail ) );
    }

    if ( pProtocol == nullptr )
    {
        lua_pushnil( pLuaState );
        return 1;
    }

    lua_pushcppref( pLuaState, s_ClassName_Protocol_FullName, pProtocol );

    return 1;
}


int iberbar::Net::LuaCFunction_AddEvent( lua_State* pLuaState )
{
    PTR_CUserEvent pUserEvent;
    CResult Ret = CEngine::sGetInstance()->AddEvent( &pUserEvent );
    if ( Ret.IsOK() == false )
    {
        lua_pushstring( pLuaState, Ret.data.c_str() );
    }
    else
    {
        lua_pushcppref( pLuaState, s_ClassName_Event_FullName, pUserEvent );
    }

    return 1;
}


int iberbar::Net::LuaCFunction_DelEvent( lua_State* pLuaState )
{
    //int top = lua_gettop( pLuaState );
    //iberbar_LuaCheckArguments( pLuaState, top, 2 );

    //CSocketServer_UseLibevent* pServer = lua_tocppobject( pLuaState, 1, CSocketServer_UseLibevent );
    //if ( pServer == nullptr )
    //{
    //    return 0;
    //}

    return 0;
}


int iberbar::Net::LuaCFunction_CreateHostedService( lua_State* pLuaState )
{
    int top = lua_gettop( pLuaState );
    iberbar_LuaCheckArguments( pLuaState, top, 2 );

    int nPort = lua_tointeger( pLuaState, 1 );
    CProtocolBase* pProtocol = lua_tocppobject( pLuaState, 2, CProtocolBase );
    if ( pProtocol == nullptr )
    {
        lua_pushstring( pLuaState, "empty protocol" );
        return 1;
    }

    PTR_CHostedService pHostedService = nullptr;
    CResult Ret = CEngine::sGetInstance()->AddHostedService( &pHostedService, nPort, pProtocol );
    if ( Ret.IsOK() == false )
    {
        lua_pushstring( pLuaState, Ret.data.c_str() );
    }
    else
    {
        lua_pushcppref( pLuaState, s_ClassName_HostedService_FullName, pHostedService );
    }
    
    return 1;
}


int iberbar::Net::LuaCFunction_CreateSocketClient( lua_State* pLuaState )
{
    int top = lua_gettop( pLuaState );
    iberbar_LuaCheckArguments( pLuaState, top, 2 );

    const char* strUrl = lua_tostring( pLuaState, 1 );
    if ( StringIsNullOrEmpty( strUrl ) )
    {
        lua_pushstring( pLuaState, "empty url" );
        return 1;
    }
    CProtocolBase* pProtocol = lua_tocppobject( pLuaState, 2, CProtocolBase );
    if ( pProtocol == nullptr )
    {
        lua_pushstring( pLuaState, "empty protocol" );
        return 1;
    }

    PTR_CSocketClient pSocketClient = nullptr;
    CResult Ret = CEngine::sGetInstance()->AddSocketClient( &pSocketClient, strUrl, pProtocol );
    if ( Ret.IsOK() == false )
    {
        lua_pushstring( pLuaState, Ret.data.c_str() );
    }
    else
    {
        lua_pushcppref( pLuaState, s_ClassName_SocketClient_FullName, pSocketClient );
    }

    return 1;
}


int iberbar::Net::LuaCFunction_RemoveSocketClient( lua_State* pLuaState )
{
    int nTop = lua_gettop( pLuaState );
    iberbar_LuaCheckArguments( pLuaState, nTop, 1 );

    CSocketClient* pClient = lua_tocppobject( pLuaState, 1, CSocketClient );
    if ( pClient == nullptr )
    {
        return 0;
    }

    CEngine::sGetInstance()->RemoveSocketClient( pClient );

    return 0;
}


int iberbar::Net::LuaCppFunction_SocketConnection_Send( lua_State* pLuaState )
{
    int top = lua_gettop( pLuaState );
    if ( top != 2 )
        return iberbar_LuaError_ArgumentsCountIsNotMatch( pLuaState, 2 );

    CSocketConnection* pConnection = lua_tocppobject( pLuaState, 1, CSocketConnection );
    if ( pConnection == nullptr )
        return 0;

    size_t nDataLen = 0;
    const char* pData = lua_tolstring( pLuaState, 2, &nDataLen );
    if ( pData == nullptr || nDataLen == 0 )
        return 0;

    pConnection->Send( pData, nDataLen );

    return 0;
}


int iberbar::Net::LuaCppFunction_SocketConnection_Listen( lua_State* pLuaState )
{
    int top = lua_gettop( pLuaState );
    if ( top != 3 && top != 4 )
        return iberbar_LuaError_ArgumentsCountIsNotMatch2( pLuaState, 3, 4 );

    CSocketConnection* pConnection = lua_tocppobject( pLuaState, 1, CSocketConnection );
    if ( pConnection == nullptr )
        return 0;

    const char* strName = lua_tostring( pLuaState, 2 );
    if ( strName == nullptr || strName[ 0 ] == 0 )
        return 0;

    lua_Integer nFunc = lua_toluacallback( pLuaState, 3 );
    lua_Integer nExt = 0;
    if ( top == 4 )
    {
        nExt = lua_toluacallback_extparam( pLuaState, 4 );
    }
    Lua::PTR_CScriptCallbackHandler pCallback = Lua::PTR_CScriptCallbackHandler::_sNew( pConnection->GetDelegateForLua()->GetLuaThread(), nFunc, nExt );

    if ( strcmp( strName, "message" ) == 0 )
    {
        pConnection->GetDelegateForLua()->SetCallbackRead( pCallback );
    }
    else if ( strcmp( strName, "close" ) == 0 )
    {
        pConnection->GetDelegateForLua()->SetCallbackClose( pCallback );
    }
    else if ( strcmp( strName, "connected" ) == 0 )
    {
        pConnection->GetDelegateForLua()->SetCallbackConnected( pCallback );
    }

    return 0;
}


int iberbar::Net::LuaCppFunction_Event_SetCallback( lua_State* pLuaState )
{
    int top = lua_gettop( pLuaState );
    if ( top != 2 && top != 3 )
        return iberbar_LuaError_ArgumentsCountIsNotMatch2( pLuaState, 2, 3 );

    CUserEvent* pEvent = lua_tocppobject( pLuaState, 1, CUserEvent );
    if ( pEvent == nullptr )
        return 0;

    lua_Integer nFunc = lua_toluacallback( pLuaState, 2 );
    lua_Integer nExt = 0;
    if ( top == 3 )
    {
        nExt = lua_toluacallback_extparam( pLuaState, 3 );
    }
    Lua::PTR_CScriptCallbackHandler pCallback = Lua::PTR_CScriptCallbackHandler::_sNew( pLuaState, nFunc, nExt );
    pEvent->SetLuaCallback( pCallback );

    return 0;
}


int iberbar::Net::LuaCppFunction_Event_Active( lua_State* pLuaState )
{
    int top = lua_gettop( pLuaState );
    iberbar_LuaCheckArguments( pLuaState, top, 1 )

    CUserEvent* pEvent = lua_tocppobject( pLuaState, 1, CUserEvent );
    if ( pEvent == nullptr )
        return 0;

    pEvent->Active();

    return 0;
}


void iberbar::Net::LuaCppCallbackExecute_Event( Lua::PTR_CScriptCallbackHandler pLuaCallback )
{
    if ( pLuaCallback != nullptr && pLuaCallback->PushFunction() )
    {
        lua_State* pLuaState = pLuaCallback->GetLuaState();
        int nArg = 0;
        if ( pLuaCallback->HasExt() )
        {
            pLuaCallback->PushExt();
            nArg++;
        }
        int nRet = lua_pcallk( pLuaState, nArg, 0, 0, 0, nullptr );
        if ( nRet != 0 )
        {
            int type = lua_type( pLuaState, -1 );
            const char* err = lua_tostring( pLuaState, -1 );
            lua_pop( pLuaState, 1 );
            printf_s( err );
        }
    }
}

