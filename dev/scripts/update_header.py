#!/usr/bin/env python3

import os
import argparse
from datetime import datetime


DEFAULT_CONFIG = {
    "author": "coshz <fsinhx@gmail.com>",
    "project_name": "Cube",
    "homepage": "https://github.com/coshz/cube",
    "license": "MIT",
    "version": "0.1.0.0",
    "year": str(datetime.now().year), 
    "date": datetime.now().strftime("%Y-%m-%d"),
    "extensions": (".cpp", ".hpp", ".h", ".hh", ".c"),
    "targets": ["."]
}


def get_header_template(config):
    """Generate the file header template based on the current configuration."""
    return f"""/*
 * File: {{filename}}
 * Project: {config["project_name"]}
 * Author: {config["author"]}
 * Version: {config["version"]}
 * Date: {config["date"]}
 * Homepage: {config["homepage"]}
 * License: {config["license"]}
 *
 * Copyright (c) {config["year"]} {config["author"]}. All rights reserved.
 */

"""


def process_file(file_path, config, header_template):
    filename = os.path.basename(file_path)
    
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            content = f.read()

        if f"Copyright (c) {config['year']} {config['author']}" in content:
            print(f"[SKIP] Header already exists: {file_path}")
            return

        new_header = header_template.format(filename=filename)
        new_content = new_header + content

        with open(file_path, "w", encoding="utf-8") as f:
            f.write(new_content)

        print(f"[ADDED] Header added to: {file_path}")

    except Exception as e:
        print(f"[ERROR] Failed to process {file_path}: {e}")


def make_parser(): 
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description="Batch insert or update file header comments in C/C++ source files."
    )
    
    # Target positions and flags
    parser.add_argument(
        "targets", 
        nargs="*", 
        help="Target files or directories to process (default: current directory '.')"
    )
    parser.add_argument("--author", type=str, help=f"Author info (default: '{DEFAULT_CONFIG['author']}')")
    parser.add_argument("--project-name", type=str, help=f"Project name (default: '{DEFAULT_CONFIG['project_name']}')")
    parser.add_argument("--homepage", type=str, help=f"Project homepage URL (default: '{DEFAULT_CONFIG['homepage']}')")
    parser.add_argument("--license", type=str, help=f"License identifier (default: '{DEFAULT_CONFIG['license']}')")
    parser.add_argument("--version", type=str, help=f"Version string (default: '{DEFAULT_CONFIG['version']}')")
    parser.add_argument("--year", type=str, help=f"Copyright year (default: '{DEFAULT_CONFIG['year']}')")
    parser.add_argument("--date", type=str, help=f"Date string (default: '{DEFAULT_CONFIG['date']}')")
    parser.add_argument(
        "--extensions", 
        type=str, 
        help="Comma-separated file extensions to target (e.g., '.cpp,.h,.c')"
    )
    return parser


def build_config(args):
    """Merge CLI arguments into the default configuration."""
    config = DEFAULT_CONFIG.copy()

    if args.targets:
        config["targets"] = args.targets
    if args.author:
        config["author"] = args.author
    if args.project_name:
        config["project_name"] = args.project_name
    if args.homepage:
        config["homepage"] = args.homepage
    if args.license:
        config["license"] = args.license
    if args.version:
        config["version"] = args.version
    if args.year:
        config["year"] = args.year
    if args.date:
        config["date"] = args.date
    if args.extensions:
        exts = [
            e.strip() if e.strip().startswith(".") else f".{e.strip()}" 
            for e in args.extensions.split(",")
        ]
        config["extensions"] = tuple(exts)

    return config


def main():
    args = make_parser().parse_args()
    config = build_config(args)
    header_template = get_header_template(config)

    print("=== Starting Batch Header Insertion ===")
    print(f"Project : {config['project_name']} (v{config['version']})")
    print(f"Author  : {config['author']}")
    print(f"Exts    : {', '.join(config['extensions'])}\n")

    extensions = config["extensions"]
    for target in config["targets"]:
        if os.path.isfile(target):
            if target.endswith(extensions):
                process_file(target, config, header_template)
        elif os.path.isdir(target):
            for root, _, files in os.walk(target):
                for file in files:
                    if file.endswith(extensions):
                        full_path = os.path.join(root, file)
                        process_file(full_path, config, header_template)
        else:
            print(f"[WARN] Target path does not exist: {target}")
            
    print("\n=== Complete! ===")


if __name__ == "__main__":
    main()