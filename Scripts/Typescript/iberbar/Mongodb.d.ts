
export function CreateClient( this: void, uri: string ): IClient;

export type UFindQuery = {
    $query: any;
    $orderby?: any;
}

export interface IClient
{
    GetDatabase( dbName: string ): IDatabase;
    GetCollection<T=any>( dbName: string, collectionName: string ): ICollection<T>;
}

declare interface IDatabase
{
    GetCollection< T = any >( collectionName: string ): ICollection< T >;
}

declare interface ICollection< T = any >
{
    Find( query?: string | UFindQuery, skip?: number, limit?: number, fields?: any ): ICursor<T>;
    InsertOne( record: Partial< T > ): boolean;
    DeleteMany( query: any ): boolean;
    DeleteOne( query: any ): boolean;
}

declare interface ICursor< T = any >
{
    First(): T;
    ToList(): T[];
}


export function BsonDate( timestamp: number ): string;
export function BsonTimestamp( timestamp: number ): string;
