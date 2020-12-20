
declare function print( this: void, ...args: any[] ): void;

declare function require( this: void, str: string ): any;

declare function type( this: void, obj: any ): "function" | "table" | "number" | "nil" | "string";

declare function getmetatable( this: void, tbl: any ): any;

declare function collectgarbage( this: void, opt?: "collect", arg?: string ): any

declare function tostring( this: void, arg: any ): string;


declare namespace debug
{
    export function traceback( this: void ): any;
}


declare namespace string
{
    function format( this: void, fmt: string, ...args: any[] ): string;
}

declare interface String
{
    format( ...args: any[] ): string;
    find( str: string ): number;
    sub( start: number, count: number ): string;
}

// declare var package: {
//     path: string;
// }

declare namespace package
{
    var path: string;
}


declare namespace os
{
    function time(): number;
}


declare namespace math
{
    function floor( n: number ): number;
}