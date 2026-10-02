# Legacy .doc (OLE2) File Extraction

python-docx handles .docx but NOT legacy .doc files (OLE2/Compound Document format). For .doc files, use `olefile` to extract the WordDocument stream and decode as UTF-16LE.

## Setup

```bash
pip install olefile
```

## Extraction Script

```python
import olefile, re

doc_path = 'path/to/file.doc'

ole = olefile.OleFileIO(doc_path)
wd = ole.openstream('WordDocument').read()
ole.close()

# Decode the raw WordDocument stream as UTF-16LE
text = wd.decode('utf-16le', errors='replace')

# Extract meaningful text (Chinese + ASCII)
chunks = re.findall(r'[\u4e00-\u9fff\uff00-\uffef\u3000-\u303f\u0020-\u007e]+', text)
for chunk in chunks:
    chunk = chunk.strip()
    if len(chunk) > 3:
        print(chunk)
```

## How it works

- Legacy .doc files are OLE2 compound documents (same container format as older Excel .xls).
- The actual text content lives in the `WordDocument` stream.
- Word stores text as UTF-16LE inside that stream, interleaved with formatting metadata.
- The regex approach filters out non-text binary data by keeping only CJK characters, fullwidth forms, and ASCII.

## Common Pitfalls

- **olefile** is NOT the same as `olefileio` or `OleFileIO` from `PIL` — it's a standalone package.
- The decoded text will contain garbage characters interspersed with actual content — that's normal, the regex filtering handles it.
- For .docx files, use `python-docx` instead (parses document structure properly).
- Some .doc files may have text in the `1Table` or `0Table` streams if the WordDocument stream fails.
- This method does NOT preserve document structure (headings, tables, formatting) — it only extracts raw text. If structure is needed, consider converting via LibreOffice first.
