-- Ensure we can load from the current directory
local info = debug.getinfo(1, "S")
local script_path = info.source:match("@?(.*/)") or "./"
package.path = script_path .. "?.lua;" .. package.path

-- Get protocol name
local this_proto = nil
for file in Dir.open(script_path, ".lua") do
    if file:match("^test.*%.lua$") then
        this_proto = file:gsub("%.lua$", "")
        break
    end
end

local this_test = require(this_proto)
DissectorTable.get("tcp.port"):add(12345, this_test)