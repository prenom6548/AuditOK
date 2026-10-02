# Paper Content Analysis: Extracting Sections & Synthesis

Reference for extracting Related Work, Methodology, and other sections from arxiv papers via their ar5iv HTML conversion, and synthesizing findings across papers.

## Why ar5iv HTML instead of PDF?

The arXiv HTML conversion (LaTeXML) produces clean, parseable HTML with:
- Consistent section numbering (`<span class="ltx_tag ltx_tag_section">2</span>`)
- Paragraph-level `<p>` tags inside `<div class="ltx_para">`
- Linkable citations and references
- No PDF parsing complexity

Availability check: `curl -sL "https://arxiv.org/html/{id}v1"` — returns 404 if unavailable.

## Full Extraction Script (Python, stdlib only)

```python
#!/usr/bin/env python3
"""Extract sections from arxiv papers for cross-paper analysis."""
import urllib.request, re, html, time, sys

def get_sections(arxiv_id):
    """Fetch paper HTML and return list of (heading_text, body_text) pairs."""
    url = f'https://arxiv.org/html/{arxiv_id}v1'
    req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
    data = urllib.request.urlopen(req, timeout=20).read().decode('utf-8')
    
    # Get all section headings
    headings = []
    for m in re.finditer(
        r'<h[234][^>]*>\s*<span[^>]*>(\d+\.?\d*)\s+</span>\s*(.*?)</h[234]>',
        data, re.DOTALL
    ):
        num = m.group(1)
        title = re.sub(r'<[^>]+>', '', m.group(2)).strip()
        headings.append((num, html.unescape(title)))
    
    # Extract Related Work specifically
    rw_matches = list(re.finditer(
        r'(<h[234][^>]*>.*?[Rr]elated [Ww]ork.*?</h[234]>)'
        r'(.*?)(?=<h[234])',
        data, re.DOTALL
    ))
    
    related_work_paras = []
    for m in rw_matches:
        body = m.group(2)
        for p in re.findall(r'<p[^>]*>(.*?)</p>', body, re.DOTALL):
            clean = re.sub(r'<[^>]+>', ' ', p)
            clean = html.unescape(clean)
            clean = re.sub(r'\s+', ' ', clean).strip()
            if clean and len(clean) > 50:
                related_work_paras.append(clean)
    
    return {
        'headings': headings,
        'related_work': '\n\n'.join(related_work_paras),
        'raw_text': re.sub(r'<[^>]+>', '\n', html.unescape(data)),
        'section_count': len(headings)
    }

# Usage
if __name__ == '__main__':
    for pid in sys.argv[1:]:
        result = get_sections(pid)
        print(f'\n=== Paper: {pid} ===')
        print(f'Sections ({result["section_count"]}):')
        for num, title in result['headings']:
            print(f'  {num}: {title}')
        print(f'\nRelated Work ({len(result["related_work"])} chars):')
        print(result['related_work'][:2000])
        time.sleep(2)  # rate limit
```

## Worked Example: Long-Horizon Agent Planning (2026)

Five papers analyzed in the source session:

| # | Paper | ID | Date | Topic |
|---|-------|----|------|-------|
| 1 | DeepPlanning | 2601.18137 | Jan 2026 | Long-horizon agentic planning benchmark |
| 2 | PIVOT | 2605.11225 | May 2026 | Trajectory refinement for plan-execution alignment |
| 3 | Agent Planning Benchmark (APB) | 2606.04874 | Jun 2026 | Diagnostic planning benchmark |
| 4 | PlanBench-XL | 2606.22388 | Jun 2026 | Large-tool-ecosystem planning |
| 5 | Self-GC | 2607.00692 | Jul 2026 | Context management for long-horizon agents |

### Section Headings Extracted

| Paper | Has ar5iv? | Sections |
|-------|-----------|----------|
| DeepPlanning | Yes | 5 sections, sub-sections 2.1 (Agentic Tool Use), 2.2 (LLM Planning) |
| PIVOT | Yes | 5 sections, Section 2 is Related Work with 3 sub-topics |
| APB | Yes | 22 domains, multi-setting diagnostic framework |
| PlanBench-XL | Yes | Multiple sections spanning tool planning and evaluation |
| Self-GC | Yes | 20+ sections including "Agent Memory and Context Management" |
| PolyWorkBench | No (404) | Abstract only — fall back to PDF |

### Related Work Synthesis Strategy

After extracting all Related Work sections:

1. **Identify recurring citations**: Papers that appear in multiple Related Work sections (e.g., ReAct, Reflexion, Self-Refine, WebArena)
2. **Map sub-directions**: Each paper carves a unique niche:
   - DeepPlanning: constrained optimization vs step-level reasoning
   - PIVOT: trajectory-as-optimizable-object vs local refinement
   - APB: diagnostic (find planning failures) vs end-to-end (rely on final success)
   - PlanBench-XL: partial tool visibility vs full access
   - Self-GC: context lifecycle governance vs heuristic pruning

3. **Classify the research space**: Benchmark design (DeepPlanning, APB, PlanBench-XL) vs Method framework (PIVOT, Self-GC) vs Diagnosis vs Mitigation vs Evaluation

4. **Build the report structure**: Background → Landscape (organized by theme) → Method Comparison → Open Challenges

### Pitfalls

- **Rate limiting**: arXiv API is ~1 req/3 sec. Use `time.sleep(3)` between queries. Semantic Scholar is 1 req/sec (100/sec with key).
- **Version suffix**: Always use `{id}v1` for HTML URL. Bare `{id}` sometimes returns abstract page instead of full text for very new papers.
- **PolyWorkBench 404**: Not all papers have ar5iv HTML — newer submissions (days-old) may not be converted yet. Fall back to PDF extraction.
- **Section boundaries**: The regex `(?=<h[234])` as section-end delimiter works for well-structured papers. Some papers use `<section>` tags — match from `</h[234]>` to `</section>` instead.
- **Non-standard section names**: Some papers name it "Background and Related Work" or embed it in the Introduction. Check section headings first.
- **Sub-section granularity**: DeepPlanning uses numbered sub-sections (2.1, 2.2) while Self-GC uses thematic headings. Adapt regex accordingly.
