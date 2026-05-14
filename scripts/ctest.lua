#!/bin/lua

local target_file = arg[1]
if not target_file then
    print("Missing Argument.")
    os.exit()
end

local file_convert = target_file:gsub("%..-$", "") .. "_converted.c"
local file_read = io.open(target_file,"r")
local file_write = io.open(file_convert, "w")

local types = 
{
    ["i1"]  = "int", 
    ["i8"]  = "int",
    ["i16"] = "int", 
    ["i32"] = "int", 
    ["i64"] = "int" 
}

local scope = 0
for line in file_read:lines() do
	if not line:find("#") then
    	if line:find("{") then
    		scope = scope + 1
    	end

    	for inex, inew in pairs(types) do
    		line = line:gsub("%f[%w]" .. inex .. "%f[%W]", inew) 
    	end

        if scope == 0 then
        	if line:find("int") and line:find(";") and not line:find("=") then
        		line = line:gsub(";", " = 0;")
    		end
		end

		if line:find("}") then 
			scope = scope - 1 
		end

		file_write:write(line .. "\n")
	end
end

file_read:close()
file_write:close()
print(file_convert)
