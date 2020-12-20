
print( "Test-MsgPack" );

import "io";

type URestaurant =
{
    "_id": string,
    "address": {
        "street": string,
        "zipcode": number,
        "building": string,
        "coord": [ number, number ]
    },
}
/*

*/

//let a = { "_id": "5db47e6d97340000b1003182", "address": { "street": "2 Avenue", "zipcode": 10075, "building": "1480", "coord": [ -73.9557413, 40.7720266 ] }, "borough": "Manhattan", "cuisine": "Italian", "grades": [ { "date": "1970-01-01T00:00:12.323Z", "grade": "A", "score": 11 }, { "date": "1970-01-01T00:02:01.212Z", "grade": "B", "score": 17 } ], "name": "Vella", "restaurant_id": "0" }

function Main( this: void ): void
{
    let mongodb: typeof import ( "iberbar.Mongodb" ) = require( "iberbar.Mongodb" );
    print( mongodb.CreateClient );
    let client = mongodb.CreateClient( "mongodb://localhost:27017" )
    let databse = client.GetDatabase( "test" )
    let collection = databse.GetCollection<URestaurant>( "restaurants" );
    let cursor = collection.Find( {
        "$query": {}
    });
    let res = cursor.First();
    print( res.address.street );
}

Main();