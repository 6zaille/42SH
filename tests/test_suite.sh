#!/bin/sh

# Initialize
echo "Running 42sh tests..."
STATUS=0

# Test 1: Simple echo command
./42sh -c "echo Hello World" | grep -q "Hello World"
if [ $? -eq 0 ]; then
  echo "Test 1 passed."
else
  echo "Test 1 failed."; STATUS=1
fi

# Test 2: Builtin true
./42sh -c "true"
if [ $? -eq 0 ]; then
  echo "Test 2 passed."
else
  echo "Test 2 failed."; STATUS=1
fi

# Test 3: Builtin false
./42sh -c "false"
if [ $? -eq 1 ]; then
  echo "Test 3 passed."
else
  echo "Test 3 failed."; STATUS=1
fi

# Test 4: Command list
./42sh -c "echo foo; echo bar" | grep -q "foo" && ./42sh -c "echo foo; echo bar" | grep -q "bar"
if [ $? -eq 0 ]; then
  echo "Test 4 passed."
else
  echo "Test 4 failed."; STATUS=1
fi

# Test 5: If command
./42sh -c "if true; then echo success; fi" | grep -q "success"
if [ $? -eq 0 ]; then
  echo "Test 5 passed."
else
  echo "Test 5 failed."; STATUS=1
fi

# Test 6: Comments
./42sh -c "echo visible # this is a comment" | grep -q "visible"
if [ $? -eq 0 ]; then
  echo "Test 6 passed."
else
  echo "Test 6 failed."; STATUS=1
fi

# Test 7: Single quotes
./42sh -c "echo 'single quotes test'" | grep -q "single quotes test"
if [ $? -eq 0 ]; then
  echo "Test 7 passed."
else
  echo "Test 7 failed."; STATUS=1
fi

# Test 8: Invalid syntax
./42sh -c "if true then" 2>/dev/null
if [ $? -eq 2 ]; then
  echo "Test 8 passed."
else
  echo "Test 8 failed."; STATUS=1
fi

# Test 9: File input
echo "echo File input test" > test_script.sh
./42sh test_script.sh | grep -q "File input test"
if [ $? -eq 0 ]; then
  echo "Test 9 passed."
else
  echo "Test 9 failed."; STATUS=1
fi
rm test_script.sh


# Exit with final status
exit $STATUS