# Generates a single amalgamation C header file that includes all public types and functions.
#
# This makes including and linking Celeritas very easy.
import re
import os
from pathlib import Path

categories = {
    "RAL": "src/ral",
    "Render": "src/new_render",
    "Maths": "src/maths"
}

def find_pub_functions_in_folder(folder_path):
    functions = []
    for filename in os.listdir(folder_path):
        filepath = os.path.join(folder_path, filename)
        if os.path.isfile(filepath):
            file_funcs = find_pub_functions_in_file(filepath)
            functions.extend(file_funcs)

    return functions

def find_pub_functions_in_file(file_path):
    pattern = r'PUB\s+(\w+\s+)*(\w+)\s+(\w+)\s*\((.*?)\)'

    with open(file_path, 'r') as file:
        content = file.read()

    matches = re.finditer(pattern, content, re.MULTILINE)

    # Collect all the functions into an array
    functions = []
    for match in matches:
        signature = match.group(0)
        if signature.startswith("PUB "):
            signature = signature[4:]

        print(signature)
        functions.append(signature)

    return functions

def generate_header():
    header_path = "celeritas.h"

    script_dir = Path(__file__).resolve().parent
    grandparent_dir = script_dir.parents[1]

    with open(header_path, 'w') as export_file:
        for category in categories.keys():
            folder = os.path.join(grandparent_dir, categories[category])
            category_funcs = find_pub_functions_in_folder(folder)
            for func in category_funcs:
                export_file.write(func)
                export_file.write(';\n')

if __name__ == "__main__":
    generate_header()
