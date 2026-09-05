#!/usr/bin/env python3
"""
verify_catalog.py — Consistency check for the NeonArena test suite.

Verifies that every test number listed in ALL_TESTS and QUICK_TESTS has a
corresponding case in dispatch_test() in tests/run_suite.sh, and that there
are no dispatch cases pointing to numbers not listed in either list.

Also checks that TESTS.md documents every number from ALL_TESTS (by scanning the
markdown table). Missing documentation is reported but not fatal — the dispatch
check is the hard gate.

Usage:
    python3 tests/verify_catalog.py [--strict]

--strict: also treat undocumented tests in TESTS.md as failures.
"""

import re
import sys
from pathlib import Path

REPO_DIR = Path(__file__).resolve().parent.parent
SUITE_PATH = REPO_DIR / "tests" / "run_suite.sh"
TESTS_MD_PATH = REPO_DIR / "tests" / "TESTS.md"


def extract_dispatch_cases(suite_text: str) -> set[str]:
    """Return set of test identifiers (e.g. '1', '9b', '15') defined in dispatch_test()."""
    # Find the dispatch_test function body
    m = re.search(r"(?s)dispatch_test\(\)\s*\{[^}]*case\s*\"\$1\"\s*in(.*?)^\}\s*;;\s*\n", suite_text, re.M)
    if not m:
        # Fallback: scan manually for the case block
        idx = suite_text.find("dispatch_test()")
        if idx == -1:
            print("ERROR: dispatch_test() not found in run_suite.sh")
            sys.exit(2)
        slice_ = suite_text[idx:]
        brace_start = slice_.find("{")
        if brace_start == -1:
            print("ERROR: dispatch_test() body not found")
            sys.exit(2)
        depth = 0
        end = None
        for i in range(brace_start, len(slice_)):
            c = slice_[i]
            if c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    end = i
                    break
        if end is None:
            print("ERROR: dispatch_test() body not found (unbalanced braces)")
            sys.exit(2)
        body = slice_[brace_start + 1 : end]
    else:
        body = m.group(1)

    # Match lines like:     N)  run_test ...
    # or:               9b) run_test ...
    # Some cases have pre-commands before run_test (e.g. rm -f ... ;), so match
    # the case label "N)" / "9b)" at line start, not the run_test call.
    pattern = re.compile(r"^\s*([\w]+)\)\s", re.M)
    cases = set(pattern.findall(body))
    return cases


def extract_test_lists(suite_text: str) -> dict[str, set[str]]:
    """Extract ALL_TESTS and QUICK_TESTS as sets of test identifiers."""
    result: dict[str, set[str]] = {}
    for var in ("ALL_TESTS", "QUICK_TESTS"):
        m = re.search(rf"{var}=\"([^\"]+)\"", suite_text)
        if not m:
            print(f"ERROR: {var} not found in run_suite.sh")
            sys.exit(2)
        result[var] = set(m.group(1).split())
    return result


def extract_documented_tests(md_text: str) -> set[str]:
    """Extract test identifiers from the TESTS.md markdown table (first column)."""
    documented: set[str] = set()
    # Match table rows: | N | ... or | N-b | ...  at start of line
    pattern = re.compile(r"^\|\s*([\w]+)\s*\|", re.M)
    for match in pattern.finditer(md_text):
        ident = match.group(1)
        # Skip the header separator line
        if ident in ("#",):
            continue
        documented.add(ident)
    return documented


def main() -> None:
    strict = "--strict" in sys.argv

    if not SUITE_PATH.is_file():
        print(f"ERROR: {SUITE_PATH} not found")
        sys.exit(2)
    suite_text = SUITE_PATH.read_text(encoding="utf-8")

    if not TESTS_MD_PATH.is_file():
        print(f"ERROR: {TESTS_MD_PATH} not found")
        sys.exit(2)
    md_text = TESTS_MD_PATH.read_text(encoding="utf-8")

    dispatch_cases = extract_dispatch_cases(suite_text)
    test_lists = extract_test_lists(suite_text)
    documented = extract_documented_tests(md_text)

    all_tests = test_lists["ALL_TESTS"]
    quick_tests = test_lists["QUICK_TESTS"]

    errors = []

    # Direction 1: every test in ALL_TESTS and QUICK_TESTS must have a dispatch case
    for var_name, test_set in (("ALL_TESTS", all_tests), ("QUICK_TESTS", quick_tests)):
        missing = test_set - dispatch_cases
        if missing:
            errors.append(
                f"FAIL: {var_name} lists {len(missing)} test(s) without dispatch_test() case: "
                f"{', '.join(sorted(missing, key=lambda x: (x.isdigit(), x)))}"
            )

    # Direction 2: every dispatch case must be in ALL_TESTS (no orphan cases)
    documented_or_quick = all_tests | quick_tests
    orphan_cases = dispatch_cases - all_tests
    if orphan_cases:
        errors.append(
            f"WARN: dispatch_test() has {len(orphan_cases)} case(s) not in ALL_TESTS: "
            f"{', '.join(sorted(orphan_cases, key=lambda x: (x.isdigit(), x)))}"
        )

    # Direction 3: every test in ALL_TESTS should be documented in TESTS.md
    undocumented = all_tests - documented
    if undocumented:
        if strict:
            errors.append(
                f"FAIL: TESTS.md does not document {len(undocumented)} test(s) from ALL_TESTS: "
                f"{', '.join(sorted(undocumented, key=lambda x: (x.isdigit(), x)))}"
            )
        else:
            print(
                f"INFO: TESTS.md does not document {len(undocumented)} test(s) from ALL_TESTS: "
                f"{', '.join(sorted(undocumented, key=lambda x: (x.isdigit(), x)))}"
            )

    # Direction 4: every ALL_TESTS id must have assert_<id>()
    assert_fns = set(re.findall(r"^assert_([\w]+)\s*\(", suite_text, re.M))
    missing_assert = all_tests - assert_fns
    if missing_assert:
        errors.append(
            f"FAIL: ALL_TESTS has {len(missing_assert)} test(s) without assert_N(): "
            f"{', '.join(sorted(missing_assert, key=lambda x: (x.isdigit(), x)))}"
        )

    # Direction 5 (soft): every documented entry should be in ALL_TESTS
    undocumented_in_md = documented - all_tests
    if undocumented_in_md:
        print(
            f"INFO: TESTS.md documents {len(undocumented_in_md)} test(s) not in ALL_TESTS: "
            f"{', '.join(sorted(undocumented_in_md, key=lambda x: (x.isdigit(), x)))}"
        )

    if errors:
        print()
        for e in errors:
            print(e)
        print()
        print(f"FAILED: {len(errors)} error(s) found")
        sys.exit(1)

    print("PASS: catalog consistency check passed")
    print(f"  dispatch_test() cases: {len(dispatch_cases)} ( {', '.join(sorted(dispatch_cases, key=lambda x: (x.isdigit(), x)))} )")
    print(f"  ALL_TESTS: {len(all_tests)} ( {', '.join(sorted(all_tests, key=lambda x: (x.isdigit(), x)))} )")
    print(f"  QUICK_TESTS: {len(quick_tests)} ( {', '.join(sorted(quick_tests, key=lambda x: (x.isdigit(), x)))} )")
    print(f"  TESTS.md documented: {len(documented)} ( {', '.join(sorted(documented, key=lambda x: (x.isdigit(), x)))} )")
    sys.exit(0)


if __name__ == "__main__":
    main()
