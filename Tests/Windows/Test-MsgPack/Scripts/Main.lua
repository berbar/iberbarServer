--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
require("io")
print("Test-MsgPack")
local function Main()
    local msgpack = require("iberbar.MsgPack")
    print(msgpack.Encode)
    print(msgpack.Decode)
    local data = {username = "ladygaga", password = "yoooo"}
    local bytes = msgpack.Encode(data)
    local dataNew = msgpack.Decode(bytes)
    print(
        "username=" .. tostring(dataNew.username)
    )
end
Main()
return ____exports
