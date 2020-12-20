--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
require("io")
print("Test-MsgPack")
local function Main()
    local mongodb = require("iberbar.Mongodb")
    print(mongodb.CreateClient)
    local client = mongodb.CreateClient("mongodb://localhost:27017")
    local databse = client:GetDatabase("test")
    local collection = databse:GetCollection("restaurants")
    local cursor = collection:Find({["$query"] = {}})
    local res = cursor:First()
    print(res.address.street)
end
Main()
return ____exports
