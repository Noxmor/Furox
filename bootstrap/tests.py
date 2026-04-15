#!/usr/bin/env python3

"""
This script is designed to run tests for the Furox compiler.

HOW TO CREATE A NEW TEST:
If your test consist only of one source file:
Create a new source file in the {TESTS_DIR} directory. This will be the entry
point for the test runner.

If your test consists of multiple source files:
Create a new directory in the {TESTS_DIR} directory. Inside that newly created
directory create a file named 'main.frx'. This will be the entry point for the
test runner script. You may freely add additional source files in this directory
or any subdirectory. The test runner will automatically discover those
source files.

All tests are expected to compile successfully and when executed return an exit
code of 0.
"""

import argparse
from colorama import init, Fore, Style
from dataclasses import dataclass
from datetime import timedelta
from enum import Enum, auto
from pathlib import Path
import subprocess
import tempfile
from typing import Optional

COMPILER = "./bin/debug/furoxc"
TESTS_DIR = "tests"

MAX_TEST_NAME_LEN = 0

class Status(Enum):
    READY = auto()
    COMP_FAIL = auto()
    COMP_TIMEOUT = auto()
    EXEC_FAIL = auto()
    EXEC_TIMEOUT = auto()
    SUCCESS = auto()

@dataclass
class Test:
    status: Status
    name: str
    files: list[Path]
    comp_timeout: timedelta = timedelta(seconds=2)
    exec_timeout: timedelta = timedelta(seconds=2)
    comp_stdout: str = ""
    comp_stderr: str = ""
    exec_stdout: str = ""
    exec_stderr: str = ""

def discover_tests(root: Path) -> list[Test]:
    tests = []

    for file in root.glob("*.frx"):
        tests.append(
            Test(
                status = Status.READY,
                name = file.stem,
                files = [file]
            )
        )

    for dirpath in root.iterdir():
        if not dirpath.is_dir():
            continue

        entry = dirpath / "main.frx"
        if not entry.exists():
            continue

        files = sorted(
            f for f in dirpath.rglob("*.frx")
            if f != entry
        )

        tests.append(
            Test(
                status = Status.READY,
                name = dirpath.name,
                files = [entry, *files]
            )
        )

        print([entry, files])

    tests = sorted(tests, key = lambda t: t.name)

    global MAX_TEST_NAME_LEN
    MAX_TEST_NAME_LEN = max((len(t.name) for t in tests), default=0) + 2

    return tests

def run_tests(tests: list[Test]) -> bool:
    every_test_passed = True

    for test in tests:
        output_file = tempfile.NamedTemporaryFile(delete = False)
        output_file.close()
        output_path = Path(output_file.name)

        compile_cmd = [COMPILER] + [str(f) for f in test.files] + ["-o", str(output_path)]

        try:
            compile_proc = subprocess.run(
                compile_cmd,
                capture_output = True,
                text = True,
                timeout = test.comp_timeout.total_seconds()
            )
        except subprocess.TimeoutExpired:
            test.status = Status.COMP_TIMEOUT
            test.comp_stdout = compile_proc.stdout
            test.comp_stderr = compile_proc.stderr
            output_path.unlink(missing_ok=True)
            every_test_passed = False
            print_test(test)
            continue

        test.comp_stdout = compile_proc.stdout
        test.comp_stderr = compile_proc.stderr

        if compile_proc.returncode != 0:
            test.status = Status.COMP_FAIL
            output_path.unlink(missing_ok=True)
            every_test_passed = False
            print_test(test)
            continue

        try:
            exec_proc = subprocess.run(
                [str(output_path)],
                capture_output = True,
                text = True,
                timeout=test.exec_timeout.total_seconds()
            )
        except subprocess.TimeoutExpired:
            test.status = Status.EXEC_TIMEOUT
            test.exec_stdout = exec_proc.stdout
            test.exec_stderr = exec_proc.stderr
            output_path.unlink(missing_ok=True)
            every_test_passed = False
            print_test(test)
            continue

        test.exec_stdout = exec_proc.stdout
        test.exec_stderr = exec_proc.stderr

        if exec_proc.returncode == 0:
            test.status = Status.SUCCESS
        else:
            test.status = Status.EXEC_FAIL
            every_test_passed = False

        print_test(test)
        output_path.unlink(missing_ok=True)

    return every_test_passed

def print_test(test: Test):
        match test.status:
            case Status.COMP_FAIL: status_str = f"{Fore.RED + Style.BRIGHT}FAIL (compilation){Style.RESET_ALL}"
            case Status.COMP_TIMEOUT: status_str = f"{Fore.RED + Style.BRIGHT}FAIL (compilation timeout){Style.RESET_ALL}"
            case Status.EXEC_FAIL: status_str = f"{Fore.RED + Style.BRIGHT}FAIL (execution){Style.RESET_ALL}"
            case Status.EXEC_TIMEOUT: status_str = f"{Fore.RED + Style.BRIGHT}FAIL (execution timeout){Style.RESET_ALL}"
            case Status.SUCCESS: status_str = f"{Fore.GREEN}PASS{Style.RESET_ALL}"

        spaces = " " * (MAX_TEST_NAME_LEN - (len(test.name) + 1))
        print(f"{test.name}:{spaces}{status_str}")

def print_tests_summary(tests: list[Test], verbose = False):
    tests_passed = 0
    tests_failed = 0

    summary_len = len(f"{len(tests)} total | {tests_passed} passed | {tests_failed} failed")

    for test in tests:
        if test.status == Status.SUCCESS:
            tests_passed += 1
        else:
            tests_failed += 1

    if verbose:
        line = "Failed tests output:"
        print("=" * summary_len)
        print(line + '\n')
        for test in tests:
            if test.status == Status.SUCCESS:
                continue

            print(f"{test.name}:")

            if test.comp_stdout:
                print(f"compilation (stdout):\n{test.comp_stdout}")

            if test.comp_stderr:
                print(f"compilation (stderr):\n{test.comp_stderr}")

            if test.exec_stdout:
                print(f"execution (stdout):\n{test.exec_stdout}")

            if test.exec_stderr:
                print(f"execution (stderr):\n{test.exec_stderr}")

            print("=" * summary_len)

    if not verbose:
        print("=" * summary_len)

    print(f"{len(tests)} total | {Fore.GREEN}{tests_passed} passed{Style.RESET_ALL} | {Fore.RED + Style.BRIGHT}{tests_failed} failed{Style.RESET_ALL}")
    print("=" * summary_len)

def parse_args():
    parser = argparse.ArgumentParser(
        prog = "./tests.py",
        description = "Test runner for the Furox compiler"
    )

    parser.add_argument(
        "-v",
        "--verbose",
        action = "store_true",
        help = "enable showing compiler and execution output for failed tests"
    )

    return parser.parse_args()

def main():
    init() # colorama initialization

    args = parse_args()

    status = 0
    tests = discover_tests(Path(TESTS_DIR))
    if not run_tests(tests):
        status = 1
    print_tests_summary(tests, verbose = args.verbose)

    return status

if __name__ == "__main__":
    raise SystemExit(main())
