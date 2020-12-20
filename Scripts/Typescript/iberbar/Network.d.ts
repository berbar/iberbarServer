

declare namespace iberbar.Net
{
    //function GetServer( this: void ): any;

    type UEventCallback_1 = ( this: void ) => void;
    type UEventCallback_2<TExt> = ( this: TExt ) => void;
    class CEvent
    {
        protected constructor();
        public SetCallback( func: UEventCallback_1 ): void;
        public SetCallback<TExt>( func: UEventCallback_2<TExt>, ext: TExt ): void;
        public Active(): void;
    }


    type USocketConnCallbackConnected_1 = ( this: void ) => void;
    type USocketConnCallbackConnected_2<TExt> = ( this: TExt ) => void;
    type USocketConnCallbackMessage_1 = ( this: void, bytes: string ) => void;
    type USocketConnCallbackMessage_2<TExt> = ( this: TExt, bytes: string ) => void;
    type USocketConnCallbackClose_1 = ( this: void ) => void;
    type USocketConnCallbackClose_2<TExt> = ( this: TExt ) => void;

    class CSocketConnection
    {
        protected constructor();
        
        public Listen( name: "connected", func: USocketConnCallbackConnected_1 ): void;
        public Listen<TExt>( name: "connected", func: USocketConnCallbackConnected_2<TExt>, ext: TExt ): void;

        public Listen( name: "message", func: USocketConnCallbackMessage_1 ): void;
        public Listen<TExt>( name: "message", func: USocketConnCallbackMessage_2<TExt>, ext: TExt ): void;

        public Listen( name: "close", func: USocketConnCallbackClose_1 ): void;
        public Listen<TExt>( name: "close", func: USocketConnCallbackClose_2<TExt>, ext: TExt ): void;

        public Send( bytes: string ): void;
    }


    // type UServerCallbackAccept_1 = ( this: void, socketConn: CSocketConnection ) => void;
    // type UServerCallbackAccept_2<TExt> = ( this: TExt, socketConn: CSocketConnection ) => void;

    // class CServer
    // {
    //     protected constructor();
    //     public SetCallbackAccept( func: UServerCallbackAccept_1 ): void;
    //     public SetCallbackAccept<TExt>( func: UServerCallbackAccept_2<TExt>, ext: TExt ): void;
    //     public AddEvent(): CEvent;
    // }


    type UHostedServiceCallbackAccept_1 = ( this: void, socketConn: CSocketConnection ) => void;
    type UHostedServiceCallbackAccept_2<TExt> = ( this: TExt, socketConn: CSocketConnection ) => void;
    class CHostedService
    {
        protected constructor();
        public SetCallbackAccept( func: UHostedServiceCallbackAccept_1 ): void;
        public SetCallbackAccept<TExt>( func: UHostedServiceCallbackAccept_2<TExt>, ext: TExt ): void;
    }


    class CSocketClient
    {
        protected constructor();
        public GetConnection(): CSocketConnection;
    }


    class CProtocol
    {
        protected constructor();
    }


    function CreateProtocol( this: void, ptype: 0 ): CProtocol;
    function CreateProtocol( this: void, ptype: 1, head: string, tail: string ): CProtocol;
    function AddEvent( this: void ): CEvent;
    function CreateHostedService( this: void, port: number, protocol: CProtocol ): CHostedService | string;
    function CreateSocketClient( this: void, url: string, protocol: CProtocol ): CSocketClient | string;
    function RemoveSocketClient( this: void ): void;
}