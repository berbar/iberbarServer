
import * as mongodb from "Mongodb";


type modMongodb = typeof import( "Mongodb" );

function Main()
{
    let mongodb: modMongodb = require( "Mongodb" );
    let server = iberbar.Server.GetServer();
    let dbclient = mongodb.CreateClient( "" );
    let a: import( "Mongodb" ).IClient = null;
    //let database = dbclient.GetDatabase( "" );
}
