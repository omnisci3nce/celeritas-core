# Generates a static webpage for the public C-API of `celeritas-core`
import re
import os
from pathlib import Path

# --- HTML Fragments
page_start = """
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="description" content="">
    <link rel="stylesheet" href="doc_styles.css">
    <!-- <link rel="stylesheet" href="prism.css"> -->
    <!-- <script src="prism.js"></script> -->

    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/default.min.css">
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js"></script>
    <!-- and it's easy to individually load additional languages -->
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/go.min.js"></script>
    <script>hljs.highlightAll();</script>

    <title>Celeritas core API</title>
</head>
<body>
<main>
"""

page_header = """
<header>
    <h1>CELERITAS CORE DOCS</h1>
</header>
"""

page_footer = """
<footer>
</footer>
"""

page_end = """
</main>
</body>
</html>
"""

def emit_function_sig(signature: str) -> str:
    return f"""
    <li class="signature">
    <pre><code class="language-c">{signature}</code></pre>
    </li>
    """

categories = {
    "RAL": "src/ral",
    "Render": "src/new_render"
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

def generate_html():
    html_filepath = "index.html"

    script_dir = Path(__file__).resolve().parent
    grandparent_dir = script_dir.parents[1]

    with open(html_filepath, 'w') as export_file:
        export_file.write(page_start)
        export_file.write(page_header)
        # TODO: make the actual content
        for category in categories.keys():
            folder = os.path.join(grandparent_dir, categories[category])
            category_funcs = find_pub_functions_in_folder(folder)
            export_file.write(f"<h3>{category}</h3>")
            export_file.write("<ul class=\"category-list\">")
            for func in category_funcs:
                export_file.write(emit_function_sig(func))
            export_file.write("</ul>")
        export_file.write(page_end)

if __name__ == "__main__":
    generate_html()
