# python-docx CJK Font Handling

python-docx's `run.font.name` does NOT reliably set font names for CJK (Chinese/Japanese/Korean) characters. To properly set 宋体 (SimSun), 楷体 (KaiTi), or 黑体 (SimHei), you must use OxmlElement to set the rFonts element.

## The Technique

```python
from docx.oxml.ns import qn
from docx.oxml import OxmlElement

def add_run(paragraph, text, bold=False, size=10.5, font_name='宋体'):
    run = paragraph.add_run(text)
    run.bold = bold
    run.font.size = Pt(size)
    run.font.name = font_name
    rPr = run._element.get_or_add_rPr()
    rFonts = OxmlElement('w:rFonts')
    rFonts.set(qn('w:eastAsia'), font_name)   # CJK font
    rFonts.set(qn('w:ascii'), font_name)       # ASCII font
    rFonts.set(qn('w:hAnsi'), font_name)       # ANSI font
    rPr.insert(0, rFonts)
    return run
```

## Priority of font name vs rFonts

python-docx sets `w:rPr/w:rFonts` based on `run.font.name`, but it only fills `w:ascii` and `w:hAnsi`, NOT `w:eastAsia`. For CJK text, `w:eastAsia` is what Word/WPS uses. Inserting the rFonts element with `w:eastAsia` set ensures CJK text renders correctly.

## Default Font Setup

Set the Normal style's font to 宋体 so all text defaults to it:

```python
style = doc.styles['Normal']
font = style.font
font.name = '宋体'
font.size = Pt(10.5)
rPr = style.element.get_or_add_rPr()
rFonts = OxmlElement('w:rFonts')
rFonts.set(qn('w:eastAsia'), '宋体')
rPr.insert(0, rFonts)
```

## Common CJK Fonts

| Chinese name | Font name for python-docx |
|---|---|
| 宋体 | SimSun |
| 楷体 | KaiTi |
| 黑体 | SimHei |
| 仿宋 | FangSong |
| 微软雅黑 | Microsoft YaHei |

## Table Cell Width Setting

When creating table cells, widths must be set via OxmlElement too:

```python
def set_cell_width(cell, width):
    tc = cell._tc
    tcPr = tc.get_or_add_tcPr()
    tcW = tcPr.find(qn('w:tcW'))
    if tcW is None:
        tcW = OxmlElement('w:tcW')
        tcPr.append(tcW)
    tcW.set(qn('w:w'), str(width))
    tcW.set(qn('w:type'), 'dxa')
```

## Table Border Setup

```python
def add_table_borders(table):
    tbl = table._tbl
    tblPr = tbl.tblPr if tbl.tblPr is not None else OxmlElement('w:tblPr')
    borders = OxmlElement('w:tblBorders')
    for border_name in ['top', 'left', 'bottom', 'right', 'insideH', 'insideV']:
        border = OxmlElement(f'w:{border_name}')
        border.set(qn('w:val'), 'single')
        border.set(qn('w:sz'), '4')
        border.set(qn('w:space'), '0')
        border.set(qn('w:color'), '000000')
        borders.append(border)
    tblPr.append(borders)
```
