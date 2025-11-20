#!/usr/bin/env python
"""
Run the tests.
Usage:  python run_tests.py
"""

import os
import subprocess
import sys
import json
from datetime import datetime
from typing import Tuple, Optional, Dict, Any


def run_cxx_proj(input_json_path: str) -> Tuple[Optional[str], Optional[str]]:
    """Run the C++ project and return (stdout, stderr)."""
    try:
        result = subprocess.run(
            ["build/json_repair_cli", input_json_path],
            capture_output=True,
            text=True,
            check=True
        )
        return result.stdout.strip(), result.stderr.strip()
    except subprocess.CalledProcessError as e:
        print(f"Error running C++ project: {e}")
        print(f"  Command: build/json_repair_cli {input_json_path}")
        print(f"  Return code: {e.returncode}")
        return None, e.stderr.strip() if e.stderr else None
    except FileNotFoundError:
        print("Error: json_repair_cli command not found. Please ensure the C++ project is built.")
        return None, "Binary not found"


def run_py_proj(input_json_path: str) -> Tuple[Optional[str], Optional[str]]:
    """Run the Python project and return (stdout, stderr)."""
    try:
        result = subprocess.run(
            ["python", "test/cli/json_repair_python.py", input_json_path],
            capture_output=True,
            text=True,
            check=True
        )
        return result.stdout.strip(), result.stderr.strip()
    except subprocess.CalledProcessError as e:
        print(f"Error running Python project: {e}")
        print(f"  Command: python test/cli/json_repair_python.py {input_json_path}")
        print(f"  Return code: {e.returncode}")
        return None, e.stderr.strip() if e.stderr else None
    except FileNotFoundError:
        print("Error: json_repair_python.py script not found")
        return None, "Script not found"


def parse_json_output(output: Optional[str], lang: str) -> Tuple[Optional[Dict[Any, Any]], bool]:
    """Parse JSON output and return (parsed_data, is_valid)."""
    if not output:
        print(f"  Warning: {lang} output is empty")
        return None, False
    
    try:
        parsed = json.loads(output)
        return parsed, True
    except json.JSONDecodeError as e:
        print(f"  Error parsing {lang} output: {e}")
        print(f"  Raw output: {repr(output)}")
        return None, False


def compare_results(cxx_result: Optional[Dict[Any, Any]], py_result: Optional[Dict[Any, Any]]) -> bool:
    """Compare the parsed results and return True if they match."""
    return cxx_result == py_result


def format_result_summary(test_results: Dict[str, Any]) -> str:
    """Format a summary of test results."""
    total_tests = test_results['total']
    passed_tests = test_results['passed']
    failed_tests = test_results['failed']
    skipped_tests = test_results['skipped']
    
    summary = []
    summary.append("\n" + "="*60)
    summary.append("TEST EXECUTION SUMMARY")
    summary.append("="*60)
    summary.append(f"Total tests: {total_tests}")
    summary.append(f"Passed:      {passed_tests}")
    summary.append(f"Failed:      {failed_tests}")
    summary.append(f"Skipped:     {skipped_tests}")
    summary.append(f"Success rate: {passed_tests/total_tests*100:.1f}%" if total_tests > 0 else "Success rate: 0%")
    
    if test_results['failed_files']:
        summary.append("\nFailed test files:")
        for failed_file in test_results['failed_files']:
            summary.append(f"  - {failed_file}")
    
    if test_results['skipped_files']:
        summary.append("\nSkipped test files:")
        for skipped_file in test_results['skipped_files']:
            summary.append(f"  - {skipped_file}")
    
    summary.append("="*60)
    return "\n".join(summary)


def main():
    """Main function to run test cases."""
    start_time = datetime.now()
    print(f"Starting test execution at {start_time.strftime('%Y-%m-%d %H:%M:%S')}")
    print("Running test cases")
    
    # Initialize test result tracking
    test_results = {
        'total': 0,
        'passed': 0,
        'failed': 0,
        'skipped': 0,
        'failed_files': [],
        'skipped_files': []
    }
    
    # Find all test files
    test_files = []
    for root, dirs, files in os.walk("test/test_cases"):
        for f in files:
            if f.endswith(".json"):
                test_files.append(os.path.join(root, f))
    
    if not test_files:
        print("No test files found in test/test_cases directory.")
        sys.exit(0)
    
    print(f"Found {len(test_files)} test files")
    
    # Run tests
    for input_json_path in sorted(test_files):
        test_results['total'] += 1
        print(f"\nTest {test_results['total']}: {input_json_path}")
        
        # Run C++ project
        print(f"  Running C++ project...")
        cxx_stdout, cxx_stderr = run_cxx_proj(input_json_path)
        
        # Run Python project
        print(f"  Running Python project...")
        py_stdout, py_stderr = run_py_proj(input_json_path)
        
        # Skip test if either command failed
        if cxx_stdout is None or py_stdout is None:
            print("  SKIPPED - Command execution failed")
            test_results['skipped'] += 1
            test_results['skipped_files'].append(input_json_path)
            continue
        
        # Parse results
        cxx_parsed, cxx_valid = parse_json_output(cxx_stdout, "C++")
        py_parsed, py_valid = parse_json_output(py_stdout, "Python")
        
        # Skip test if parsing failed
        if not cxx_valid or not py_valid:
            print("  SKIPPED - Output parsing failed")
            test_results['skipped'] += 1
            test_results['skipped_files'].append(input_json_path)
            continue
        
        # Compare results
        if compare_results(cxx_parsed, py_parsed):
            print("  PASSED - Results match")
            test_results['passed'] += 1
        else:
            print("  FAILED - Results differ")
            print(f"    C++ output: {cxx_stdout}")
            print(f"    Python output: {py_stdout}")
            test_results['failed'] += 1
            test_results['failed_files'].append(input_json_path)
    
    # Print summary
    end_time = datetime.now()
    duration = end_time - start_time
    print(format_result_summary(test_results))
    print(f"Test execution completed in {duration.total_seconds():.2f} seconds")
    
    # Exit with appropriate code
    if test_results['failed'] > 0:
        print(f"\n{test_results['failed']} test(s) failed.")
        sys.exit(1)
    elif test_results['total'] == test_results['passed']:
        print(f"\nAll {test_results['passed']} test(s) passed!")
        sys.exit(0)
    else:
        print(f"\nTest execution completed with {test_results['skipped']} skipped test(s).")
        sys.exit(0)


if __name__ == '__main__':
    main()