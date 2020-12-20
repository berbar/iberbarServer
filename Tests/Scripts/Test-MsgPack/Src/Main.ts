
print( "Test-MsgPack" );

import "io";

function Main( this: void ): void
{
    let msgpack: typeof import ( "iberbar.MsgPack" ) = require( "iberbar.MsgPack" );
    print( msgpack.Encode );
    print( msgpack.Decode );
   
    let data = {
        username: "ladygaga",
        password: "yoooo"
    };
    let bytes = msgpack.Encode( data );
    let dataNew: { username: string } = msgpack.Decode( bytes );
    print( "username=" + dataNew.username );
}

Main();