#!/bin/lua
local target_file = arg[1]

if not target_file then
     print("Missing Argument.")
     os.exit()
end

local file_convert = target_file:gsub("%..-$", "") .. ".c"
local file_read = io.open(target_file,"r")
local file_write = io.open(file_convert, "w")
local scope = 0

for line in file_read:lines() do

    --Multiple line comment
    line = line:gsub("/~", "/*")
    line = line:gsub("~/", "*/")
    --Single line check
    if not (line:find("/%*") or line:find("%*/")) then
        line = line:gsub("~", "//")
    end

	if line:find("#") then
        if line:find("%(") and line:find("%)") then
            line = line:gsub("#", "")
        else
            line = ""
        end
    end

    --Jump to if (condition given)
    line = line:gsub("%f[%w]jump%s*(%b())%s*(%w+);", "if %1 goto %2;")
    --Jump to if (no condition)
    line = line:gsub("%f[%w]jump%s+(%w+);", "goto %1;")
    -- :mirror to mirror:
    line = line:gsub(":(%w+)", "%1:")
    -- i[number] to int conversion
    line = line:gsub("%f[%w]i%d+%f[%W]", "int")

    local brace_open  = line:find("{")
    local brace_close = line:find("}")
    --bracket check
    if scope == 0 and not brace_open then
        if line:find("int") and line:find(";") and not line:find("=") then
        	line = line:gsub(";", " = 0;")
        end
    end

    if brace_open then scope = scope + 1 end
    if brace_close then scope = scope - 1 end
    file_write:write(line .. "\n")

end

file_read:close()
file_write:close()
print(file_convert)


