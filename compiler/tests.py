"""
This script is designed to run tests for the Furox compiler.
Every test is part of one of two categories: 'fail' or 'success'.

Category 'fail': A test that is meant to fail during compilation time.
Categiry 'success': A test that is meant to be compiled *and* executed successfully.

Note: A successfull compilation/execution is indicated by an exit status of 0.

HOW TO CREATE A TEST:
Tests are defined in a subdirectory named after their category.
Every test is either directory-based or consists of a single file.
Create either a single file or a directory for the new test in the subdirectory
of the test's category. If the test is directory-based, meaning it consists of
multiple files, the test script will assume that the entry point will be a file
'main' with the appropiate extension inside the newly created directory.
Additional files regarding this test should go in this directory as well.
"""

import os
import subprocess
import tempfile
from colorama import init, Fore, Style

# Initializes colorama for colored output
init()

COMPILER = "./frx" # Path to the compiler this script uses
TEST_DIR = "./tests" # Path to the tests directory
EXTENSION = ".frx" # File extension of the test files

PASS_COUNT = 0 # Number of passed tests
FAIL_COUNT = 0 # Number of failed tests

def run_test(filepath, expected_result):
    global PASS_COUNT, FAIL_COUNT

    print(f"{filepath}: ", end=" ")

    if os.path.isfile(filepath):
        source_file = filepath
    elif os.path.isdir(filepath):
        source_file = os.path.join(filepath, "main" + EXTENSION)
    else:
        print(Fore.YELLOW + "INVALID" + Style.RESET_ALL)
        FAIL_COUNT += 1
        return

    with tempfile.NamedTemporaryFile(delete=False) as temp_executable:
        temp_executable_name = temp_executable.name

    compile_result = subprocess.run([COMPILER, "-o", temp_executable.name, source_file], capture_output=True, text=True)
    actual_result = compile_result.returncode
    compile_output = compile_result.stdout + compile_result.stderr

    if actual_result == expected_result:
        if expected_result == 0:
            exec_result = subprocess.run([temp_executable.name], capture_output=True, text=True)
            exec_returncode = exec_result.returncode
            exec_output = exec_result.stdout + exec_result.stderr

            if exec_returncode == 0:
                print(Fore.GREEN + "PASS" + Style.RESET_ALL)
                PASS_COUNT += 1
            else:
                print(Fore.RED + "FAIL (execution)" + Style.RESET_ALL)
                print(exec_output)
                FAIL_COUNT += 1
        else:
            print(Fore.GREEN + "PASS" + Style.RESET_ALL)
            PASS_COUNT += 1
    else:
        print(Fore.RED + "FAIL (compilation)" + Style.RESET_ALL)
        print(compile_output)
        FAIL_COUNT += 1

def main():
    global PASS_COUNT, FAIL_COUNT
    valid_tests = [os.path.join(TEST_DIR, "success", f) for f in os.listdir(os.path.join(TEST_DIR, "success"))]
    invalid_tests = [os.path.join(TEST_DIR, "fail", f) for f in os.listdir(os.path.join(TEST_DIR, "fail"))]

    total_tests = len(valid_tests) + len(invalid_tests)

    for test in valid_tests:
        run_test(test, 0)

    for test in invalid_tests:
        run_test(test, 1)

    if PASS_COUNT == total_tests:
        print(Fore.GREEN + f"All ({total_tests}) tests passed" + Style.RESET_ALL)
    else:
        print(Fore.RED + f"{PASS_COUNT}/{total_tests} tests passed" + Style.RESET_ALL)

    exit(FAIL_COUNT)

if __name__ == "__main__":
    main()
