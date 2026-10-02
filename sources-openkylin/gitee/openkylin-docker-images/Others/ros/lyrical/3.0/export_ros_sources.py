#!/usr/bin/env python3
"""Export Git source revisions, including detached shallow imports, as .repos."""
import json
import os
from pathlib import Path
import subprocess
import sys


def git(path, *args):
    return subprocess.check_output(
        ["git", "-C", str(path), *args], text=True
    ).strip()


def main():
    source = Path(sys.argv[1]).resolve(strict=True)
    repositories = {}
    for directory, children, files in os.walk(source):
        children.sort()
        if ".git" not in children and ".git" not in files:
            continue
        path = Path(directory)
        # Submodule revisions are already recorded by their parent Git commit.
        children[:] = []
        repositories[path.relative_to(source).as_posix()] = {
            "type": "git",
            "url": git(path, "remote", "get-url", "origin"),
            "version": git(path, "rev-parse", "--verify", "HEAD"),
        }
    if not repositories:
        raise RuntimeError(f"No Git source repositories found in {source}")
    # JSON is valid YAML and is accepted by vcstool's .repos importer.
    destination = Path(sys.argv[2])
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(
        json.dumps({"repositories": repositories}, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    print(f"Recorded {len(repositories)} source revisions in {destination}")


if __name__ == "__main__":
    main()
