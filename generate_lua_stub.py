import re
import os

HEADER_DIR = "./include"
OUTPUT_FILE = "./lua_stubs/conx_api.lua"

FUNC_REGEX = re.compile(r'^\s*(?:void|bool|int|float|double|struct\s+\w+|\w+)\s+(\w+)\s*\(([^)]*)\)\s*;')

os.makedirs(os.path.dirname(OUTPUT_FILE), exist_ok=True)

with open(OUTPUT_FILE, 'w') as lua_file:
    lua_file.write("-- Auto-generated ConX Engine API stub for Lua LSP\n")
    lua_file.write("ConX = {}\n\n")

    for header in os.listdir(HEADER_DIR):
        if header.endswith(".h"):
            with open(os.path.join(HEADER_DIR, header), 'r') as f:
                for line in f:
                    match = FUNC_REGEX.match(line)
                    if match:
                        func_name, params = match.groups()
                        param_names = []
                        if params.strip() != "void" and params.strip() != "":
                            for p in params.split(','):
                                pname = p.strip().split()[-1]
                                pname = pname.replace("*", "")
                                param_names.append(pname)
                        param_str = ", ".join(param_names)
                        lua_file.write(f"---@param {param_str} any\n" if param_str else "")
                        lua_file.write(f"function ConX.{func_name}({param_str}) end\n\n")

print(f"Lua stub generated at {OUTPUT_FILE}")
