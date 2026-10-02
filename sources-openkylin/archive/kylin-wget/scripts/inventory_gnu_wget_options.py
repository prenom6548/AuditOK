#!/usr/bin/env python3
"""
从 GNU wget 源码 main.c 的 option_data[] 中提取长选项名，生成可追溯的「全量接口清单」，
用于 wget-rust 与原文档协议对齐（见 docs/功能对等目标.md）。
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


def extract_long_options(main_c: Path) -> list[str]:
    text = main_c.read_text(encoding="utf-8", errors="replace").splitlines()
    in_array = False
    names: list[str] = []
    for line in text:
        if "option_data[]" in line and "=" in line:
            in_array = True
            continue
        if in_array and line.strip() == "};":
            break
        if not in_array:
            continue
        # 标准行: { "accept", 'A', ...
        m = re.search(r"\{\s*\"([^\"]+)\"", line)
        if m:
            names.append(m.group(1))
            continue
        # IF_SSL ( "ca-certificate", ...
        m = re.search(r"IF_SSL\s*\(\s*\"([^\"]+)\"", line)
        if m:
            names.append(m.group(1))
    return names


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--main-c",
        type=Path,
        default=Path("/home/zoey/code/gerrit/wget/src/main.c"),
        help="GNU wget 源码中的 main.c 路径",
    )
    ap.add_argument("-o", "--output", type=Path, default=None, help="写入 Markdown；默认定 stdout")
    args = ap.parse_args()

    if not args.main_c.is_file():
        print(f"ERROR: 找不到 {args.main_c}", file=sys.stderr)
        return 2

    opts = extract_long_options(args.main_c)
    # 稳定排序便于 diff（原文档顺序即定义顺序，保留原序 + 去重仅防万一）
    seen: set[str] = set()
    unique_ordered: list[str] = []
    for o in opts:
        if o not in seen:
            seen.add(o)
            unique_ordered.append(o)

    lines = [
        "<!-- 由 scripts/inventory_gnu_wget_options.py 生成，请勿手改长选项列表 -->",
        "",
        f"# GNU wget `option_data[]` 长选项清单（共 **{len(unique_ordered)}** 项）",
        "",
        "基准文件：`main.c` 中 `option_data[]`。",
        "",
        "说明：",
        "",
        "- 受 **编译宏** 影响的项（如 `IF_SSL` / `HAVE_METALINK`）在未定义宏的构建中可能不存在；",
        "  wget-rust 以 **常见 Linux 发行版默认构建能力** 为验收参考，必要时在兼容矩阵中逐项标注。",
        "- 短选项与 `--no-` 派生名不计入本表行数（与 GNU 行为一致时再单独对齐）。",
        "",
        "## 长选项名（按源码出现顺序）",
        "",
    ]
    for i, name in enumerate(unique_ordered, 1):
        lines.append(f"{i}. `--{name}`")

    out = "\n".join(lines) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(out, encoding="utf-8")
    else:
        sys.stdout.write(out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
