#!/bin/bash

# Test #1 - The use of grep sed awk
echo "Test #1 - Does the script use grep, sed, awk? It must use grep and one of either sed/awk (or both sed and awk)"
grep 'grep' covidata.sh
grep 'sed' covidata.sh
grep 'awk' covidata.sh
echo

# Test #2 - Test for the existance of the errorMsg function
echo "Test #2 - Existance of errorMsg function"
grep 'errorMsg' covidata.sh
echo

# Test #3 - Procedure name fail test
echo "Test #3 - Expected error message for wrong procedure name"
./covidata.sh bob 35 testfile.csv outputfile.csv
echo

# Test #4 - Wrong number of parameters fail tests (3 tests)
echo "Three wrong parameter tests:"
echo "Test #4A - Expected error message for the wrong number of parameters for get"
./covidata.sh get 35 testfile.csv
echo "-------"
echo "Test #4B - Expected error message for the wrong number of parameters for compare"
./covidata.sh compare 10 inputdata.csv outputdata.csv
echo "-------"
echo "Test #4C - Expected error message for the wrong number of parameters using -r"
./covidata.sh -r get 35 inputdata.csv outputdata.csv
echo

# Test #5 - Correct use of get procedure
echo "Test #5 - GET procedure test - no errors expected"
./covidata.sh get 59 $1 result.csv
# more result.csv
cat result.csv # you can change this to more, if you like
echo

# Test #6 - Overwrite test with get procedure
echo "Test #6 - GET procedure overwrite output file - no error/warning expected"
./covidata.sh get 59 $1 result.csv
echo

# Test #7 - Error message for get procedure, wrong ID
echo "Test #7 - GET procedure ID test - errorMsg expected"
./covidata.sh get 5321 $1 result.csv
echo

# Test #8 - Correct use of compare procedure
echo "Test #8 - COMPARE procedure test - no errors expected"
./covidata.sh get 59 $1 result.csv
./covidata.sh compare 35 $1 result2.csv result.csv
# more result2.csv
cat result2.csv
echo

# # Test #9 - Correct use of -r
echo "Test #9 - Correct use of -r (2 tests)"
echo "Test #9A - Testing -r with GET procedure - no errors expected"
./covidata.sh -r get 35 $2 $3 $1 resultGET.csv
# more resultGET.csv
cat resultGET.csv
echo "-----"
echo "Test #9B - Testing -r with COMPARE procedure - no errors expected"
./covidata.sh -r compare 35 $2 $3 $1 resultCOMP.csv resultGET.csv
# more resultCOMP.csv
cat resultCOMP.csv # you can change to more
echo
echo "ALL THE TESTS ARE DONE"
