#!/usr/bin/env python3

import argparse
import re 
import os
from typing import Tuple, Optional


def make_parser():
    parser = argparse.ArgumentParser(
        description="Expand user C++ files into a single amalgamated source file."
    )
    parser.add_argument(
        "sources", nargs='+', help="C++ source files")
    parser.add_argument(
        '-I', '--include-dirs', nargs='+', default=[], help="Include directories")
    parser.add_argument(
        '-c', '--compacted', action='store_true', help="Compacted output")
    parser.add_argument(
        '-o', '--output', required=True, help="Output file path")
    return parser


def resolve_header_path(
    header_name: str, current_dir, include_dirs: list[str]
) -> Optional[str]: 
    cand = os.path.abspath(os.path.join(current_dir, header_name))
    if os.path.isfile(cand): 
        return cand
    for d in include_dirs:
        cand = os.path.abspath(os.path.join(d, header_name))
        if os.path.isfile(cand):
            return cand
    return None


def remove_comments(text: str) -> str:
    pattern = re.compile(
        r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
        re.DOTALL | re.MULTILINE
    )
    def replacer(match): 
        s = match.group(0)
        return "" if s.startswith("/") else s
    return re.sub(pattern, replacer, text)


def expand_cpp_file(
    path: str, include_dirs: list[str], visited_headers: set[str]
) -> Tuple[list[str], str]: 
    """
    Expand a c++ header or source file

    Return: 
        a list of public header files, the remaining content
    """
    includes = []
    content = ""

    inc_pat = re.compile(r'^\s*#\s*include\s*["<]([^">]+)[">]')
    pragma_once_pat = re.compile(r"^\s*#\s*pragma\s+once")

    abs_path = os.path.abspath(path)
    if abs_path in visited_headers: return [], ""
    visited_headers.add(abs_path)
    current_dir = os.path.dirname(abs_path)

    with open(abs_path, 'r') as f:
        for line in f:
            if pragma_once_pat.match(line): continue
            m = inc_pat.match(line)
            if m:
                header_name = m.group(1)
                header_path = resolve_header_path(header_name, current_dir, include_dirs)
                if header_path: 
                    incl, cont = expand_cpp_file(header_path, include_dirs, visited_headers)
                    includes.extend(incl)
                    content += cont
                else:
                    includes.append(header_name)
            else:
                content += line
    return includes, content


def expand_cpp_sources(
    sources: list[str], output_file: str, include_dirs: list[str], compacted: bool
):
    includes = []
    content = ""
    visited_headers = set()
    for src in sources:
        includes_, content_ = expand_cpp_file(src, include_dirs, visited_headers)
        includes.extend(includes_)
        content += content_

    # clean duplicated headers 
    includes = list(dict.fromkeys(includes))

    if compacted:
        # make output compacted: clean comments and duplicated newlines
        content = remove_comments(content)
        content = re.sub(r"[ \t]+$", "", content, flags=re.MULTILINE)
        content = re.sub(r"\n{2,}", "\n", content)
    else:
        # clean continuous newlines
        content = re.sub(r"[ \t]+$", "", content, flags=re.MULTILINE)
        content = re.sub(r"\n{3,}", "\n\n", content)

    out_dir = os.path.dirname(os.path.abspath(output_file))
    if out_dir: os.makedirs(out_dir, exist_ok=True)
    with open(output_file, 'w') as f:
        for h in includes:
            f.write(f"#include <{h}>\n")
        f.write("\n")
        content = content.lstrip('\n')
        f.write(content)


if __name__ == '__main__':
    parser = make_parser()
    args = parser.parse_args()
    expand_cpp_sources(
        args.sources, args.output, args.include_dirs, args.compacted
    )
