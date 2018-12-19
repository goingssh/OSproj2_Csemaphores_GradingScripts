#!/bin/bash
# @Sherri Goings for H2SO4 project grading
# run all files ending in .test as sample program output through
# python grading tests in TestOutput.py. Each should fail the test
# described in the test file name and output a recognizeable error msg.

for d in *;
do 
    if [[ $d == *.test ]]
    then
	echo running test $d
	cat $d | python3 ../TestOutput.py
	echo end test $d
	echo
    fi;
done
