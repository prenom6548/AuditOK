# Python-docx Sandbox Workaround

## Problem

The `execute_code` sandbox environment does not have `python-docx` pre-installed. Attempting `from docx import Document` in execute_code raises `ModuleNotFoundError`.

## Solution

Write a Python script file to `/tmp/` and run it via `terminal("python3 /tmp/script.py")`. The terminal environment uses the system Python where python-docx IS installed.

### Working pattern

```python
from hermes_tools import terminal, write_file

script = '''
from docx import Document
import os

doc = Document()
doc.add_heading("Title", 0)
doc.add_paragraph("Content here")

output_path = "/path/to/output.docx"
doc.save(output_path)
print(f"Saved: {output_path}")
'''

write_file("/tmp/create_doc.py", script)
result = terminal("python3 /tmp/create_doc.py")
print(result)
```

### Why this works

- `execute_code` runs in an isolated sandbox (e.g., `/tmp/hermes_sandbox_xxx/`) with limited packages
- `terminal` runs in the user's actual shell environment where system packages are available
- python-docx is typically installed system-wide on the host

### Alternative

Use `write_file` to create the script directly, then `terminal("python3 /path/to/script.py")` to run it. This avoids the sandbox limitation entirely.

### Applicable to

- Any python-docx operation (create, read, modify .docx files)
- Other packages not available in the execute_code sandbox
- Any situation where you need to run Python code with system-level dependencies
