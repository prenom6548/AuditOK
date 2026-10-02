#!/usr/bin/env python3
"""Build skill embeddings index for semantic search. todo:ljl may be used for optimization of embedding saving.

Scans all SKILL.md files (local + external dirs), encodes name+description
via local qwen3-embedding-0.6b model, and writes to ~/.kylin-agent-runtime/.skill_embeddings.json.

Usage:
    python scripts/build_skills_embeddings.py [--force]

Options:
    --force    Rebuild even if manifest unchanged.

Requirements:
    - sentence-transformers installed: pip install 'kylin-agent-runtime[embeddings]'
    - config.yaml skills.embedding_search.model_path configured
    - Model downloaded at specified path (e.g., /path/to/qwen3-embedding-0.6b)

Output: ~/.kylin-agent-runtime/.skill_embeddings.json

Performance: ~9 seconds for 79 skills on CPU
"""

import argparse
import os
import sys
from pathlib import Path

# Allow importing from repo root
REPO_ROOT = Path(__file__).parent.parent.resolve()
sys.path.insert(0, str(REPO_ROOT))

# Ensure HERMES_HOME is set
os.environ.setdefault("HERMES_HOME", str(Path.home() / ".kylin-agent-runtime"))


def main():
    parser = argparse.ArgumentParser(
        description="Build skill embeddings index for semantic search"
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Force rebuild even if manifest unchanged",
    )
    args = parser.parse_args()

    # Import after path setup
    from agent.skill_embeddings import build_skill_embeddings, _embeddings_path

    print("Building skill embeddings index...")
    print(f"Output: {_embeddings_path()}")
    print()

    try:
        index = build_skill_embeddings(force=args.force)
        if not index:
            print("❌ Failed to build embeddings index.")
            print("   Check config.yaml skills.embedding_search.enabled and model_path.")
            print("   Ensure sentence-transformers is installed: pip install 'kylin-agent-runtime[embeddings]'")
            sys.exit(1)

        skills_count = len(index.get("skills", []))
        dim = index.get("dim", 0)
        model_path = index.get("model_path", "")

        print(f"✅ Embeddings index built successfully!")
        print(f"   Skills indexed: {skills_count}")
        print(f"   Embedding dimension: {dim}")
        print(f"   Model: {model_path}")
        print(f"   Written to: {_embeddings_path()}")
        print()
        print("Enable semantic search in config.yaml:")
        print("  skills:")
        print("    embedding_search:")
        print("      enabled: true")
        print(f"      model_path: \"{model_path}\"")
        print("      top_k: 8")
        print("      min_score: 0.3")

    except KeyboardInterrupt:
        print("\n⚠️  Build interrupted by user.")
        sys.exit(130)
    except Exception as e:
        print(f"❌ Unexpected error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
