#!/bin/lua

cc = "gcc";
flags = "-g -O0";
sources = "main.c common.c preprocessor/preprocessor.c diagnostic.c lexer.c parser.c semantic.c ir.c codegen.c";
target = "bin/seal";

function make()
    local success, exit_type, code = os.execute(cc .. " " .. flags .. " " .. sources .. " -o " .. target);
    print("OUTPUT: " .. target .. "\n");
    print("SUCCESS: " .. tostring(success));
    print("EXIT TYPE: " .. tostring(exit_type));
    print("CODE: " .. tostring(code) .. "\n");
    
    print(cc .. " " .. flags .. " " .. sources .. " -o " .. target);
    os.exit(0);
end

function install()
    os.execute("mv " .. target .. arg[2]);
end

function run()
    os.execute("./" .. target .. " --Save llvm --Compile " .. arg[2]);
end

function clear()
    os.execute("rm " .. target);
end

CMD = arg[1];

if CMD == "install" or CMD == "run" then
    sec_arg = arg[2];

    if sec_arg == " " or nil then
        print("Missing secondary Argument.\n");
        os.exit(1);
    end
end

if arg[1] == nil or arg[1] == " " then
    make();
elseif CMD == "install" then
    install();
elseif CMD == "run" then
    run();
elseif CMD == "clear" then
    clear();
else
    print("Missing primary argument\n");
end
