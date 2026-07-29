#!/usr/bin/env python3
"""
Audit and optionally normalize tab whitespace while preserving visual columns.

This tool is intentionally not a formatter. It does not reindent, reflow, trim,
sort includes, or apply a style guide. Its normalization operation is visual tab
expansion: each tab is replaced with exactly enough spaces to reach the next tab
stop for the selected tab width.

Default posture is read-only audit. Writes require --apply. Whole-project writes
also require --all-project-owned so a broad formatting pass cannot happen by
accident.
"""

from __future__ import annotations

import argparse
import difflib
import os
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence

SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent

DEFAULT_EXTENSIONS = {
    ".cpp",
    ".h",
    ".frag",
    ".vert",
    ".sh",
    ".md",
}

DEFAULT_EXCLUDED_DIRS = {
    ".git",
    ".grit",
    ".pathwarden",
    "app",
    "img",
    "lib",
    "log",
    "out",
    "sav",
    "vid",
}

SAMPLE_LIMIT = 4


@dataclass
class LineAudit:
    number: int
    text: str
    tab_count: int
    leading_tab_count: int
    internal_tab_count: int
    mixed_leading: bool
    trailing_whitespace: bool


@dataclass
class FileAudit:
    path: Path
    lines: list[LineAudit]
    undecodable: bool = False

    @property
    def tab_count(self) -> int:
        return sum(line.tab_count for line in self.lines)

    @property
    def leading_tab_lines(self) -> int:
        return sum(1 for line in self.lines if line.leading_tab_count > 0)

    @property
    def internal_tab_lines(self) -> int:
        return sum(1 for line in self.lines if line.internal_tab_count > 0)

    @property
    def mixed_leading_lines(self) -> int:
        return sum(1 for line in self.lines if line.mixed_leading)

    @property
    def trailing_whitespace_lines(self) -> int:
        return sum(1 for line in self.lines if line.trailing_whitespace)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Audit tabs and optionally expand them to spaces while preserving visual columns.",
    )
    parser.add_argument(
        "paths",
        nargs="*",
        default=[str(REPO_ROOT)],
        help="Files or directories to inspect. Defaults to the repository root.",
    )
    parser.add_argument(
        "--tab-width",
        type=int,
        default=4,
        help="Visual tab width used for expansion. Default: 4.",
    )
    parser.add_argument(
        "--ext",
        default=",".join(sorted(DEFAULT_EXTENSIONS)),
        help="Comma-separated extensions to include when walking directories. Use '*' for all files.",
    )
    parser.add_argument(
        "--include-vendor",
        action="store_true",
        help="Include normally excluded vendored/generated/runtime directories.",
    )
    parser.add_argument(
        "--preview",
        action="store_true",
        help="Print unified diffs for files that would change.",
    )
    parser.add_argument(
        "--apply",
        action="store_true",
        help="Write expanded files. Requires explicit file paths unless --all-project-owned is also set.",
    )
    parser.add_argument(
        "--all-project-owned",
        action="store_true",
        help="Allow --apply to write files found by directory traversal.",
    )
    parser.add_argument(
        "--show-clean",
        action="store_true",
        help="Show files with no tabs in audit output.",
    )
    return parser.parse_args()


def extension_filter(ext_arg: str) -> set[str] | None:
    if ext_arg.strip() == "*":
        return None
    exts = set()
    for item in ext_arg.split(","):
        value = item.strip()
        if not value:
            continue
        exts.add(value if value.startswith(".") else f".{value}")
    return exts


def is_excluded(path: Path, include_vendor: bool) -> bool:
    if include_vendor:
        return False
    try:
        rel = path.relative_to(REPO_ROOT)
    except ValueError:
        return False
    return any(part in DEFAULT_EXCLUDED_DIRS for part in rel.parts)


def iter_files(paths: Sequence[str], exts: set[str] | None, include_vendor: bool) -> list[Path]:
    found: list[Path] = []
    for raw in paths:
        path = Path(raw).expanduser()
        if not path.is_absolute():
            path = (Path.cwd() / path).resolve()
        else:
            path = path.resolve()

        if path.is_file():
            found.append(path)
            continue

        if not path.is_dir():
            raise SystemExit(f"ERROR: path not found: {path}")

        for root, dirs, files in os.walk(path):
            root_path = Path(root)
            if is_excluded(root_path, include_vendor):
                dirs[:] = []
                continue
            dirs[:] = [
                directory
                for directory in dirs
                if not is_excluded(root_path / directory, include_vendor)
            ]
            for filename in files:
                candidate = root_path / filename
                if exts is not None and candidate.suffix not in exts:
                    continue
                found.append(candidate.resolve())

    return sorted(set(found))


def split_line_ending(line: str) -> tuple[str, str]:
    if line.endswith("\r\n"):
        return line[:-2], "\r\n"
    if line.endswith("\n"):
        return line[:-1], "\n"
    if line.endswith("\r"):
        return line[:-1], "\r"
    return line, ""


def leading_prefix(body: str) -> str:
    idx = 0
    while idx < len(body) and body[idx] in (" ", "\t"):
        idx += 1
    return body[:idx]


def audit_line(number: int, line: str) -> LineAudit | None:
    body, _ending = split_line_ending(line)
    tab_count = body.count("\t")
    if tab_count == 0:
        trailing = bool(body) and body[-1] in (" ", "\t")
        if not trailing:
            return None
    prefix = leading_prefix(body)
    leading_tabs = prefix.count("\t")
    internal_tabs = tab_count - leading_tabs
    mixed_leading = " " in prefix and "\t" in prefix
    trailing = bool(body) and body[-1] in (" ", "\t")
    return LineAudit(
        number=number,
        text=line,
        tab_count=tab_count,
        leading_tab_count=leading_tabs,
        internal_tab_count=internal_tabs,
        mixed_leading=mixed_leading,
        trailing_whitespace=trailing,
    )


def audit_file(path: Path) -> FileAudit:
    try:
        text = path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return FileAudit(path=path, lines=[], undecodable=True)
    lines = text.splitlines(keepends=True)
    audited = [
        line_audit
        for number, line in enumerate(lines, start=1)
        if (line_audit := audit_line(number, line)) is not None
    ]
    return FileAudit(path=path, lines=audited)


def expand_tabs_line(line: str, tab_width: int) -> str:
    body, ending = split_line_ending(line)
    column = 0
    out: list[str] = []
    for char in body:
        if char == "\t":
            spaces = tab_width - (column % tab_width)
            out.append(" " * spaces)
            column += spaces
        else:
            out.append(char)
            column += 1
    return "".join(out) + ending


def expand_file_text(text: str, tab_width: int) -> str:
    return "".join(expand_tabs_line(line, tab_width) for line in text.splitlines(keepends=True))


def visible_sample(line: str) -> str:
    body, ending = split_line_ending(line)
    rendered = body.replace("\t", "<TAB>")
    if ending:
        rendered += "<EOL>"
    return rendered


def display_path(path: Path) -> str:
    try:
        return str(path.relative_to(REPO_ROOT))
    except ValueError:
        return str(path)


def render_audit(audit: FileAudit, show_clean: bool) -> None:
    if audit.undecodable:
        print(f"{display_path(audit.path)}")
        print("  skipped: not valid UTF-8")
        return

    if not audit.lines:
        if show_clean:
            print(f"{display_path(audit.path)}")
            print("  tabs: 0")
        return

    print(f"{display_path(audit.path)}")
    print(
        "  "
        f"tabs={audit.tab_count}, "
        f"leading-tab-lines={audit.leading_tab_lines}, "
        f"internal-tab-lines={audit.internal_tab_lines}, "
        f"mixed-leading-lines={audit.mixed_leading_lines}, "
        f"trailing-whitespace-lines={audit.trailing_whitespace_lines}"
    )

    tab_samples = [line for line in audit.lines if line.tab_count > 0][:SAMPLE_LIMIT]
    for sample in tab_samples:
        print(f"    L{sample.number}: {visible_sample(sample.text)}")


def render_preview(path: Path, tab_width: int) -> bool:
    try:
        original = path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return False

    expanded = expand_file_text(original, tab_width)
    if expanded == original:
        return False

    rel = display_path(path)
    diff = difflib.unified_diff(
        original.splitlines(keepends=True),
        expanded.splitlines(keepends=True),
        fromfile=f"{rel}\tcurrent",
        tofile=f"{rel}\texpanded-tabs-{tab_width}",
    )
    print("".join(diff), end="")
    return True


def apply_expansion(path: Path, tab_width: int) -> bool:
    try:
        original = path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return False

    expanded = expand_file_text(original, tab_width)
    if expanded == original:
        return False

    path.write_text(expanded, encoding="utf-8")
    return True


def main() -> int:
    args = parse_args()
    if args.tab_width <= 0:
        raise SystemExit("ERROR: --tab-width must be positive")

    exts = extension_filter(args.ext)
    files = iter_files(args.paths, exts, args.include_vendor)

    explicit_files = all(Path(raw).expanduser().is_file() for raw in args.paths)
    if args.apply and not explicit_files and not args.all_project_owned:
        raise SystemExit(
            "ERROR: --apply over directories requires --all-project-owned. "
            "Use --preview first."
        )

    audits = [audit_file(path) for path in files]

    print(f"Root: {REPO_ROOT}")
    print(f"Files scanned: {len(files)}")
    print(f"Tab width: {args.tab_width}")
    if not args.include_vendor:
        print(f"Excluded dirs: {', '.join(sorted(DEFAULT_EXCLUDED_DIRS))}")
    print("")

    for audit in audits:
        render_audit(audit, args.show_clean)

    if args.preview:
        print("")
        print("=== Preview: visual tab expansion ===")
        changed = 0
        for path in files:
            if render_preview(path, args.tab_width):
                changed += 1
        print(f"Preview changed files: {changed}")

    if args.apply:
        print("")
        print("=== Apply: visual tab expansion ===")
        changed = 0
        for path in files:
            if apply_expansion(path, args.tab_width):
                changed += 1
                print(f"expanded: {display_path(path)}")
        print(f"Applied changed files: {changed}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
