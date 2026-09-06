#!/usr/bin/env python3
"""
NeonArena Test Runner — parallel, retry, JSON output.

Usage:
    python3 test_runner.py                     # run all tests
    python3 test_runner.py --parallel 4        # parallel execution
    python3 test_runner.py --test 1,2,3        # specific tests
    python3 test_runner.py --list              # list all tests
    python3 test_runner.py --retry 2           # retry failed tests
    python3 test_runner.py --json              # JSON output for CI
    python3 test_runner.py --verbose           # verbose output
    python3 test_runner.py --quick             # smoke tests only
"""

import argparse
import concurrent.futures
import json
import os
import re
import subprocess
import sys
import time
from dataclasses import dataclass, field, asdict
from pathlib import Path
from typing import List, Optional

# --- Configuration ---
TESTS_DIR = Path(__file__).parent / "tests"
HELPERS_DIR = TESTS_DIR / "helpers"
DEFAULT_TIMEOUT = 120  # seconds
MAX_PARALLEL = 8

# --- Data Classes ---
@dataclass
class TestResult:
    num: int
    name: str
    passed: bool
    duration: float
    output: str = ""
    error: str = ""
    retries: int = 0

    def to_dict(self):
        return asdict(self)

@dataclass
class TestSuite:
    results: List[TestResult] = field(default_factory=list)
    
    @property
    def passed(self):
        return sum(1 for r in self.results if r.passed)
    
    @property
    def failed(self):
        return sum(1 for r in self.results if not r.passed)
    
    @property
    def total(self):
        return len(self.results)
    
    @property
    def duration(self):
        return sum(r.duration for r in self.results)
    
    def to_dict(self):
        return {
            "passed": self.passed,
            "failed": self.failed,
            "total": self.total,
            "duration": round(self.duration, 2),
            "results": [r.to_dict() for r in self.results]
        }

# --- Test Discovery ---
def discover_tests() -> List[tuple[int, str, Optional[Path]]]:
    """Discover all test scripts in tests/ directory.
    Returns (num, name, script_path) tuples.
    For tests without a dedicated script, script_path is None (uses bash bridge).
    """
    tests = []
    for script in sorted(TESTS_DIR.glob("test_*.sh")):
        m = re.match(r"test_(\d+)_(.+)\.sh", script.name)
        if m:
            num = int(m.group(1))
            name = m.group(2)
            tests.append((num, name, script))
    
    # Also find tests defined only in run_suite.sh
    suite_file = TESTS_DIR / "run_suite.sh"
    if suite_file.exists():
        suite_content = suite_file.read_text()
        for m in re.finditer(r"assert_(\d+)\(\)", suite_content):
            num = int(m.group(1))
            if not any(t[0] == num for t in tests):
                # Extract name from run_test call: `    81) run_test 81 "ghost-balance-cvars" ...`
                name_m = re.search(rf"\s{num}\)\s+run_test\s+{num}\s+\"([^\"]+)\"", suite_content)
                name = name_m.group(1) if name_m else f"test-{num}"
                tests.append((num, name, None))
    
    return sorted(tests, key=lambda x: x[0])

def get_test_script(num: int) -> Optional[Path]:
    """Get test script by number."""
    m = re.search(r"test_(\d+)_", f"test_{num}_")
    for n, _, script in discover_tests():
        if n == num:
            return script
    return None

# --- Bash Bridge (for tests without dedicated scripts) ---
def run_bash_test(num: int, timeout: int = DEFAULT_TIMEOUT) -> TestResult:
    """Run a test via run_suite.sh --test N (for tests without dedicated scripts)."""
    start = time.time()
    script = TESTS_DIR / "run_suite.sh"
    
    try:
        result = subprocess.run(
            [str(script), "--test", str(num)],
            capture_output=True,
            text=True,
            timeout=timeout,
            cwd=str(TESTS_DIR.parent)
        )
        duration = time.time() - start
        output = result.stdout + result.stderr
        
        # Parse output: "TEST N: name ... PASS/FAIL"
        name = f"test-{num}"
        passed = "PASS" in output and "FAIL" not in output
        
        m = re.search(rf"TEST {num}: ([\w-]+)", output)
        if m:
            name = m.group(1)
        
        return TestResult(
            num=num,
            name=name,
            passed=passed,
            duration=duration,
            output=output[-2000:],
            error="" if passed else output[-1000:]
        )
    except subprocess.TimeoutExpired:
        return TestResult(num=num, name=f"test-{num}", passed=False, duration=timeout,
                         error=f"Test timed out after {timeout}s")
    except Exception as e:
        return TestResult(num=num, name=f"test-{num}", passed=False, duration=time.time()-start,
                         error=str(e))

# --- Test Execution ---
def run_single_test(num: int, name: str, script: Optional[Path], timeout: int = DEFAULT_TIMEOUT) -> TestResult:
    """Run a single test. Uses dedicated script if available, else falls back to run_suite.sh."""
    if script and script.exists():
        start = time.time()
        try:
            result = subprocess.run(
                [str(script)],
                capture_output=True,
                text=True,
                timeout=timeout,
                cwd=str(TESTS_DIR.parent),
                env={**os.environ, "TEST_NUM": str(num)}
            )
            duration = time.time() - start
            output = result.stdout + result.stderr
            passed = "PASS" in output and "FAIL" not in output
            return TestResult(
                num=num, name=name, passed=passed, duration=duration,
                output=output[-2000:] if len(output) > 2000 else output,
                error="" if passed else output[-1000:]
            )
        except subprocess.TimeoutExpired:
            return TestResult(num=num, name=name, passed=False, duration=timeout,
                             error=f"Test timed out after {timeout}s")
        except Exception as e:
            return TestResult(num=num, name=name, passed=False, duration=time.time()-start,
                             error=str(e))
    else:
        # No dedicated script — use bash bridge
        return run_bash_test(num, timeout)

# --- Parallel Runner ---
def run_tests_parallel(tests: List[tuple[int, str, Optional[Path]]], max_workers: int = 4,
                       retry: int = 0, timeout: int = DEFAULT_TIMEOUT) -> TestSuite:
    """Run tests in parallel with retry support."""
    suite = TestSuite()
    
    with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
        future_to_test = {
            executor.submit(run_single_test, num, name, script, timeout): (num, name, script)
            for num, name, script in tests
        }
        
        for future in concurrent.futures.as_completed(future_to_test):
            num, name, script = future_to_test[future]
            result = future.result()
            
            # Retry failed tests
            if not result.passed and retry > 0:
                for attempt in range(retry):
                    retry_result = run_single_test(num, name, script, timeout)
                    retry_result.retries = attempt + 1
                    if retry_result.passed:
                        result = retry_result
                        break
            
            suite.results.append(result)
    
    suite.results.sort(key=lambda r: r.num)
    return suite

# --- Output Formatting ---
class Colors:
    GREEN = "\033[92m"
    RED = "\033[91m"
    YELLOW = "\033[93m"
    BLUE = "\033[94m"
    RESET = "\033[0m"
    BOLD = "\033[1m"

def print_result(result: TestResult, verbose: bool = False):
    """Print a single test result."""
    if result.passed:
        status = f"{Colors.GREEN}PASS{Colors.RESET}"
    else:
        status = f"{Colors.RED}FAIL{Colors.RESET}"
    
    retry_info = f" (retried {result.retries}x)" if result.retries > 0 else ""
    print(f"  {result.num:>3}. [{status}] {result.name:<35} {result.duration:>6.1f}s{retry_info}")
    
    if verbose and not result.passed:
        print(f"       {Colors.RED}{result.error[:200]}{Colors.RESET}")

def print_summary(suite: TestSuite):
    """Print test suite summary."""
    print(f"\n{'='*60}")
    if suite.failed == 0:
        print(f"{Colors.GREEN}All tests passed!{Colors.RESET}")
    else:
        print(f"{Colors.RED}{suite.failed} test(s) failed{Colors.RESET}")
    print(f"  Passed: {suite.passed}/{suite.total}")
    print(f"  Failed: {suite.failed}/{suite.total}")
    print(f"  Duration: {suite.duration:.1f}s")
    print(f"{'='*60}")

# --- Main ---
def main():
    parser = argparse.ArgumentParser(description="NeonArena Test Runner")
    parser.add_argument("--parallel", type=int, default=1, help="Number of parallel workers")
    parser.add_argument("--test", type=str, help="Run specific tests (comma-separated)")
    parser.add_argument("--list", action="store_true", help="List all tests")
    parser.add_argument("--retry", type=int, default=0, help="Retry failed tests N times")
    parser.add_argument("--json", action="store_true", help="Output JSON for CI")
    parser.add_argument("--verbose", "-v", action="store_true", help="Verbose output")
    parser.add_argument("--quick", action="store_true", help="Run smoke tests only")
    parser.add_argument("--timeout", type=int, default=DEFAULT_TIMEOUT, help="Test timeout in seconds")
    
    args = parser.parse_args()
    
    # Discover tests
    all_tests = discover_tests()
    
    if args.list:
        print("Available tests:")
        for num, name, script in all_tests:
            print(f"  {num:>3}. {name}")
        return 0
    
    # Filter tests
    if args.test:
        selected = [int(t) for t in args.test.split(",")]
        tests = [(n, name, s) for n, name, s in all_tests if n in selected]
    elif args.quick:
        # Quick smoke tests: 1, 2, 3, 7, 8
        quick_ids = {1, 2, 3, 7, 8}
        tests = [(n, name, s) for n, name, s in all_tests if n in quick_ids]
    else:
        tests = all_tests
    
    if not tests:
        print("No tests found!")
        return 1
    
    # Run tests
    if not args.json:
        print(f"Running {len(tests)} test(s) with {args.parallel} worker(s)...")
    
    suite = run_tests_parallel(
        tests,
        max_workers=min(args.parallel, MAX_PARALLEL),
        retry=args.retry,
        timeout=args.timeout
    )
    
    # Output
    if args.json:
        print(json.dumps(suite.to_dict(), indent=2))
    else:
        for result in suite.results:
            print_result(result, args.verbose)
        print_summary(suite)
    
    return 0 if suite.failed == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
