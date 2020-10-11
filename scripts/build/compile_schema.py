#!/bin/python
# Copyright by Potato Engine contributors.  See accompanying License.txt for
# copyright details.
import sys
import json
import argparse
import os
from os import path
from datetime import datetime
import type_info

class Context:
    """Complete context used by a generator"""
    def __init__(self, db, output, input_name):
        self.__db = db
        self.__output = output
        self.__input_name = input_name

    @property
    def db(self):
        return self.__db

    @property
    def input_name(self):
        return self.__input_name

    @property
    def output(self):
        return self.__output

    def print(self, *args):
        self.__output.write(''.join(*args))
        print(*args, end='')

def generate_header(ctx: Context):
    """Executor for the brunt of code generation"""

    ctx.print(f"""
// --- GENERATED FILE ----
// - Do not edit this file
// - Generated on {datetime.utcnow()} UTC
// - Generated from {path.basename(ctx.input_name)}
#pragma once
#include "potato/spud/string.h"
#include "potato/reflex/reflect.h"
namespace up {{
    inline namespace schema {{
""")

    for name, type in ctx.db.exports.items():
        ctx.print(f"        struct {type.cxxname} {{\n")
        for field in type.fields_ordered:
            ctx.print(f"            {field.type.cxxname} {field.cxxname};\n")
        ctx.print("        };\n")
        ctx.print(f"""
        UP_REFLECT_TYPE({type.cxxname}) {{
            reflect("{field.name}", &Type::{field.cxxname});
        }}

""")

    ctx.print(f"""
    }} // namespace up::schema
}} // namespace up
""")

def main(argv):
    """Main entrypoint for the program"""
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', type=argparse.FileType(mode='r', encoding='utf-8'), required=True, help="Input schema json")
    parser.add_argument('-o', '--output', type=str, help="Target output file")
    args = parser.parse_args(argv[1:])

    doc = json.load(args.input)
    args.input.close()

    db = type_info.TypeDatabase()
    db.load_from_json(doc)

    output = sys.stdout

    generate = generate_header

    if args.output is not None:
        parent_path = path.dirname(args.output)
        os.makedirs(name=parent_path, exist_ok=True)
        output = open(args.output, 'wt', encoding='utf-8')

    ctx = Context(db=db, output=output, input_name=args.input.name)

    rs = generate(ctx)
    if rs is None or rs == 0:
        return 0
    else:
        return 1

if __name__ == '__main__':
    sys.exit(main(sys.argv))
