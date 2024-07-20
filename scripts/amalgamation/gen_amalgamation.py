# Generates a single amalgamation C header file that includes all public types and functions.
#
# This makes including and linking Celeritas very easy.
import re
import sys

def find_pub_functions(filepath):
    pattern = r'PUB\s+(\w+\s+)*(\w+)\s+(\w+)\s*\((.*?)\)'

    with open(filepath, 'r') as file:
        content = file.read()

    matches = re.finditer(pattern, content, re.MULTILINE)

    for match in matches:
        print(match.group(0))

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <path_to_c_file>")
        sys.exit(1)

    file_path = sys.argv[1]
    find_pub_functions(file_path)
