# Grading script for H2SO4 semaphore project
# @Sherri Goings, last modified 12/17/2018
#
# Run on the command line as follows:
#         python3 TestMany.py ./student_executable args
# e.g.    python3 TestMany.py ./a.out 4 2 8 
#
# Runs H2SO4 program N times, reports the proportion of runs that pass all test cases,
# along with an approximated 95% confidence interval for the proportion.
# Note: no output from TestOutput.py means that no test cases failed.
#
# If at least one test fails it will print one of the failures that occurred.

import math
import subprocess
import sys

def main():
    """
    runs executable with arguments given on command line N times (default 200), and runs
    TestOutput.py on the results of each of those N trials. Aggregates the results of the
    N TestOutput.py runs into % passed all tests vs failed at least one. If any tests failed,
    also prints example of 1 failed test.
    """
    exeName = sys.argv[1]
    atomArgs = " " + sys.argv[2] + " " + sys.argv[3] + " " + sys.argv[4] 

    N = 200
    fail = 0
    fail_output = None
    print("Running tests", end="")

    for i in range(N):
        # runs command e.g. "./a.out 4 2 8 | python3 TestOutput.py" and saves stdout to output
        output = subprocess.check_output(exeName + atomArgs + ' | python3 TestOutput.py', shell=True)

        # empty output means passed all tests, otherwise output will be 1 failed test message
        if len(output) > 0:
            fail += 1
            fail_output = output

        # just so user knows how quickly testing is progressing
        print(".",end=" ")
        sys.stdout.flush()

    # basic confidence interval
    p = float(N - fail) / N
    margin = 1.96 * math.sqrt(p * (1-p) / N)

    print()
    print("Success rate: %.1f +/- %.1f" % ((100.0 * p), (100 * margin)))
    if fail_output:
        print("\nFailure example:\n", fail_output)
        
if __name__=="__main__":
    main()
