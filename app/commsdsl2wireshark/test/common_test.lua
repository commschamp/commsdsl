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

local success, result = pcall(require, this_proto)
if not success then
    -- If 'success' is false, 'result' contains the error message
    io.stderr:write("FATAL ERROR loading '" .. tostring(this_proto) .. "':\n" .. tostring(result) .. "\n")
    os.exit(1)
end

-- If we get here, require succeeded!
-- 'result' now contains whatever the required file returned prot object
local this_test = result
DissectorTable.get("tcp.port"):add(12345, this_test)