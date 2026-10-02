---
name: technical-disclosure-drafting
description: "Draft technical disclosures from research materials."
version: 1.0.0
author: KylinAgent
license: MIT
platforms: [linux, macos, windows]
metadata:
  hermes:
    tags: [Patents, Documents, Technical-Writing, Templates, Research]
    related_skills: [ocr-and-documents]
---

# Technical Disclosure Drafting

Draft structured technical disclosure documents (e.g., patent technical disclosure documents) from research manuscript content. The workflow: read source material + read template -> fill template sections -> output.

## Typical Use Case

User provides:
1. A **template file** (.doc or .docx) — e.g., a "技术交底书" (patent disclosure form) with sections like:
   - 发明创造名称
   - 所属技术领域
   - 背景技术及现有技术的缺陷和不足
   - 发明构思描述
   - 优点或有益效果
   - 技术关键点和欲保护点
   - 具体实施例及附图

2. A **research manuscript** (.docx) — containing descriptions of one or more inventions/patents

Output: One or more completed disclosure documents (.docx preferred, matching original template format), each filling the template with one invention's content. The output MUST be a .docx file (Microsoft Word 2007+), NOT a .md file, unless the user explicitly requests otherwise. If the original template is a legacy .doc file, copy it first to the output directory as a blank template, then create the content-filled version as .docx. The user's local WPS opens .docx natively.

CRITICAL: PRESERVE THE ORIGINAL TEMPLATE STRUCTURE. Do NOT replace guide text — keep it in 楷体 font and append filled content after it in 宋体 font. The template/guide text is part of the document and must remain visible.

See also:
- `references/patent-template-structure.md` — complete template structure and writing conventions
- `references/python-docx-cjk-fonts.md` — how to properly set CJK fonts (宋体/楷体) in python-docx
- `references/python-docx-sandbox-workaround.md` — how to run python-docx outside the execute_code sandbox

## Workflow

### Step 1: Read the Template

Use `python-docx` for .docx templates. Use olefile for legacy .doc templates (see `ocr-and-documents` skill's `references/legacy-doc-extraction.md`).

Identify the template structure — look for numbered sections with titles like "1. 发明创造名称", "2. 所属技术领域", etc. Each section has an instruction (e.g., "应简单明确地表明发明的实质内容") that the filled content must follow.

### Step 2: Read the Research Manuscript

```python
from docx import Document
doc = Document('path/to/manuscript.docx')
for para in doc.paragraphs:
    text = para.text.strip()
    if text:
        print(text)
```

Identify:
- How many inventions/patents are described (look for headings like "专利一", "专利二", "Patent 1", etc.)
- For each patent: invention name, background, technical problem, solution description, advantages, embodiments, figures

The manuscript often includes:
- A brief foreground section explaining the general context
- Multiple optional invention names (recommend one)
- Background technology section
- Deficiencies of existing technology
- Technical solution description (often broken into phases/stages)
- Advantages/beneficial effects
- Specific implementation examples (实施例)
- Figure descriptions (附图说明)
- Claim layout suggestions (权利要求布局建议)

### Step 3: Map Content to Template Sections

| Template Section | Manuscript Content to Map |
|---|---|
| 发明创造名称 | The first/main invention name from manuscript |
| 所属技术领域 | Infer from the technical domain of the content |
| 背景技术及缺陷 | The 背景技术 and 现有技术的缺陷和不足 sections |
| 发明构思描述 | The technical solution description (phases, modules, flows) |
| 优点或有益效果 | The 本发明创造的优点 section |
| 技术关键点和欲保护点 | The technical key points + claim layout suggestions |
| 具体实施例及附图 | The 具体实施方式 (embodiments) + 附图说明 sections |

### Step 3b: Copy Original Template First (for .doc format)

If the original template is a .doc file (legacy binary format), do NOT attempt to modify it programmatically. Instead:
1. Copy it to the output directory with the patent name in the filename
2. Create a separate .docx file with the filled content using python-docx
3. The .doc copy preserves the original blank template for the user; the .docx file contains the filled content

### Step 4: Generate .docx with python-docx Preserving Template Style

Use python-docx to create a .docx file that exactly mirrors the template's structure:

```python
from docx import Document
from docx.shared import Pt
from docx.oxml.ns import qn
from docx.oxml import OxmlElement

def set_cell_width(cell, width):
    tc = cell._tc
    tcPr = tc.get_or_add_tcPr()
    tcW = tcPr.find(qn('w:tcW'))
    if tcW is None:
        tcW = OxmlElement('w:tcW')
        tcPr.append(tcW)
    tcW.set(qn('w:w'), str(width))
    tcW.set(qn('w:type'), 'dxa')

def add_run(paragraph, text, bold=False, size=10.5, font_name='宋体'):
    run = paragraph.add_run(text)
    run.bold = bold
    run.font.size = Pt(size)
    run.font.name = font_name
    rPr = run._element.get_or_add_rPr()
    rFonts = OxmlElement('w:rFonts')
    rFonts.set(qn('w:eastAsia'), font_name)
    rFonts.set(qn('w:ascii'), font_name)
    rFonts.set(qn('w:hAnsi'), font_name)
    rPr.insert(0, rFonts)
    return run
```

Template structure to replicate:
- Title "技术交底书" centered and bold (16pt)
- Instruction paragraph in 10pt font
- A 2-column table with borders, each row = one template section
- Left column: section title (bold, 10.5pt 宋体)
- Right column: guide text in 9pt 楷体, followed by filled content in 10.5pt 宋体

IMPORTANT FONT RULES:
- 指导/说明文字 (guide/instruction text): 楷体 (KaiTi), 9pt
- 填写内容 (filled content): 宋体 (SimSun), 10.5pt
- 章节标题 (section titles): 宋体, bold, 10.5pt
- Use OxmlElement to set rFonts for eastAsia/ascii/hAnsi, otherwise python-docx may ignore font name for CJK characters

### Step 4b: Verify the .docx Structure

After generating, verify with python-docx that:
- The table has 8 rows (title row + 7 sections)
- Each row's cell 1 starts with the original guide text
- Filled content follows the guide text
- No guide text was accidentally overwritten

### Step 5: Write in Formal Patent Style

When writing the disclosure, follow these style conventions:

- **名称**: Simple and clear, avoid promotional/advertising wording. Use "方法、装置、设备及介质" suffix.
- **技术领域**: Use the pattern "本发明属于[A]与[B]技术领域，具体涉及...尤其是一种..."
- **背景技术**:
  - Start with a development history of the technical field (eq \o\ac(○,1))
  - Then describe existing technology deficiencies (eq \o\ac(○,2))
  - Be thorough about the deficiencies — describe them in sufficient detail
  - Omit background irrelevant to understanding the invention (eq \o\ac(○,3))
- **发明构思描述**: Describe what each structure/module/phase does. For methods, describe the flow step by step.
- **优点**: Write in numbered "第一，...第二，..." format, each starting with "本发明" or "系统".
- **技术关键点和欲保护点**: Key points are brief; protection points enumerate the core steps/structures.
- **实施例**: Multiple specific examples showing how the invention is implemented. Must be detailed enough for a person skilled in the art to reproduce without creative labor.
- **附图说明**: Figure descriptions using "图1为本发明...示意图" pattern.

### Step 5: Save Output

Create a dedicated folder (e.g., `/data/usershare/paper/专利交底书/`) and save each patent as a separate file.

Naming convention: `专利_[N]_[Invention-Name-in-Chinese].docx` for content-filled files, `技术交底书-[Patent-Name].doc` for blank template copies.

## Pitfalls

- **Legacy .doc files** cannot be read or written with python-docx. Use olefile to extract content. To create output, either: (a) copy the .doc as-is for blank templates, OR (b) create new .docx files with python-docx.
- **Multiple inventions in one manuscript**: Each gets its own disclosure document. They may share the same template but have different content.
- **Patent template sections are often nested** with sub-items (○1, ○2, etc.). Follow the numbering convention.
- **Disambiguate the two patents' boundaries**: When two patents share a theme (e.g., both use phrase structure/syntax), clearly state how they differ. For example: Patent 1 = "language generation control", Patent 2 = "agent planning control".
- **The `doc.paragraphs` iterator in python-docx** may miss content in tables, headers, footers, text boxes. Check `doc.tables` as well.
- **DO NOT remove or overwrite guide text**. The original template's instruction text (e.g., "应简单明确地表明发明的实质内容") is part of the form and must be preserved. Add filled content after it.
- **CJK fonts must be set via OxmlElement**. python-docx's `run.font.name` does NOT reliably set east-Asian font names. Always use the rFonts OxmlElement pattern to set w:eastAsia, w:ascii, and w:hAnsi.
- **Empty initial table row**: When using `doc.add_table(rows=1, cols=2)`, an empty row is created. Either fill it or accept it — it won't affect document readability in WPS.
- **Lock/temp files**: WPS may create hidden `.~` prefixed temp files in the output directory. Delete them before verification to avoid `PackageNotFoundError`.
- **execute_code sandbox lacks python-docx**: The sandbox environment does not include python-docx. Do NOT attempt `from docx import Document` in execute_code — it will fail with `ModuleNotFoundError`. Instead, write a Python script file and run it via `terminal("python3 /tmp/script.py")`. See `references/python-docx-sandbox-workaround.md` for the full pattern.

## Verification

After writing, verify:
- All 7 template sections are present and filled for each patent
- Guide/instruction text from the template is PRESERVED (楷体, 9pt), not removed
- Filled content is in 宋体 (10.5pt), appended after guide text
- The output is saved to the correct directory as specified by the user
- Each .docx file can be opened with python-docx without PackageNotFoundError (remove .~ temp files first)
- Each section has non-trivial content (check char count per section)
- If the user specified .doc format: also copy the original blank template .doc to the output directory
