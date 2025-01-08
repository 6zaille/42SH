#!/bin/sh

echo "Running tests..."
./42sh -c "echo Hello World" | grep -q "Hello World" || exit 1
echo "Test 1 passed."

./42sh -c "true" || exit 1
echo "Test 2 passed."

./42sh -c "false" && exit 1
echo "Test 3 passed."

exit 0