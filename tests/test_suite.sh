#!/bin/sh

echo "Running complete 42sh test suite..."
echo "====================================="
STATUS=0
TOTAL_TESTS=0
PASSED_TESTS=0
BIN_PATH="./src/42sh"

if [ ! -f "$BIN_PATH" ]; then
    echo "Error: 42sh binary not found at $BIN_PATH"
    exit 1
fi

run_test() {
    TEST_NAME=$1
    COMMAND=$2
    EXPECTED_EXIT_CODE=$3
    CHECK_CMD=$4

    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    echo -n "Running $TEST_NAME... "

    OUTPUT=$(eval "$COMMAND" 2>&1)
    ACTUAL_EXIT_CODE=$?

    if [ $ACTUAL_EXIT_CODE -eq $EXPECTED_EXIT_CODE ] && eval "$CHECK_CMD"; then
        echo "PASSED"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo "FAILED (expected $EXPECTED_EXIT_CODE, got $ACTUAL_EXIT_CODE)"
        echo "Command output:"
        echo "$OUTPUT"
        STATUS=1
    fi
}

# Tests pour echo
run_test "Test Echo 1: Simple echo" \
    "$BIN_PATH -c 'echo Hello World'" 0 \
    'echo "$OUTPUT" | grep -q "Hello World"'

run_test "Test Echo 2: Echo with -n flag" \
    "$BIN_PATH -c 'echo -n Hello World'" 0 \
    '[ "$OUTPUT" = "Hello World" ]'

run_test "Test Echo 3: Echo with -e flag (newlines)" \
    "$BIN_PATH -c 'echo -e Hello\\nWorld'" 0 \
    'echo "$OUTPUT" | grep -q "Hello" && echo "$OUTPUT" | grep -q "World"'

run_test "Test Echo 4: Echo with -e flag (tabs)" \
    "$BIN_PATH -c 'echo -e Hello\\tWorld'" 0 \
    'echo "$OUTPUT" | grep -q "Hello" && echo "$OUTPUT" | grep -q "World"'

run_test "Test Echo 5: Echo with -E flag (escapes ignored)" \
    "$BIN_PATH -c 'echo -E Hello\\nWorld'" 0 \
    'echo "$OUTPUT" | grep -q "Hello\\nWorld"'

run_test "Test Echo 6: Echo combination of -n and -e" \
    "$BIN_PATH -c 'echo -n -e Hello\\nWorld'" 0 \
    '[ "$OUTPUT" = "Hello\nWorld" ]'

run_test "Test Echo 7: Echo without flags (default behavior)" \
    "$BIN_PATH -c 'echo Text with no flags'" 0 \
    'echo "$OUTPUT" | grep -q "Text with no flags"'

run_test "Test Echo 8: Echo with empty input" \
    "$BIN_PATH -c 'echo '" 0 \
    '[ "$OUTPUT" = "" ]'

run_test "Test Echo 9: Echo with only flags" \
    "$BIN_PATH -c 'echo -n -e -E'" 0 \
    '[ "$OUTPUT" = "" ]'

run_test "Test Echo 10: Echo with invalid flags" \
    "$BIN_PATH -c 'echo -z Hello World'" 0 \
    '[ "$OUTPUT" = "-z Hello World" ]'

run_test "Test Echo 11: Echo with mixed valid and invalid flags" \
    "$BIN_PATH -c 'echo -n -x Hello'" 0 \
    '[ "$OUTPUT" = "Hello" ]'

run_test "Test Echo 12: Echo with special characters" \
    "$BIN_PATH -c 'echo \"Hello $USER!\"'" 0 \
    'echo "$OUTPUT" | grep -q "Hello"'

run_test "Test Echo 13: Echo with unicode characters" \
    "$BIN_PATH -c 'echo 🚀 🌟'" 0 \
    'echo "$OUTPUT" | grep -q "🚀 🌟"'

run_test "Test Echo 14: Echo with multi-line input" \
    "$BIN_PATH -c 'echo -e \"Line1\\nLine2\\nLine3\"'" 0 \
    'echo "$OUTPUT" | grep -q "Line1" && echo "$OUTPUT" | grep -q "Line2" && echo "$OUTPUT" | grep -q "Line3"'

run_test "Test Echo 15: Echo with edge case escapes" \
    "$BIN_PATH -c 'echo -e \"\\n\\t\\\"\"'" 0 \
    'echo "$OUTPUT" | grep -q "\n\t\""'

# Tests supplémentaires pour d'autres fonctionnalités
run_test "Test 16: Builtin true" \
    "$BIN_PATH -c 'true'" 0 'true'

run_test "Test 17: Builtin false" \
    "$BIN_PATH -c 'false'" 1 'true'

run_test "Test 18: If-then-else syntax" \
    "$BIN_PATH -c 'if true; then echo yes; else echo no; fi'" 0 \
    'echo "$OUTPUT" | grep -q "yes"'

run_test "Test 19: Command substitution" \
    "$BIN_PATH -c 'echo \$(echo nested)'" 0 \
    'echo "$OUTPUT" | grep -q "nested"'

run_test "Test 20: Redirection to file" \
    "$BIN_PATH -c 'echo output > test_file'; grep -q "output" test_file; rm test_file" 0 'true'

run_test "Test 21: Nested pipes" \
    "$BIN_PATH -c 'echo chain | tr a b | grep b'" 0 \
    'echo "$OUTPUT" | grep -q "b"'

run_test "Test 22: Subshell variable isolation" \
    "$BIN_PATH -c 'VAR=outer; (VAR=inner; echo \$VAR); echo \$VAR'" 0 \
    'echo "$OUTPUT" | grep -q "inner" && echo "$OUTPUT" | grep -q "outer"'

run_test "Test 23: While loop" \
    "$BIN_PATH -c 'COUNTER=0; while [ \$COUNTER -lt 3 ]; do echo \$COUNTER; COUNTER=\$((COUNTER+1)); done'" 0 \
    'echo "$OUTPUT" | grep -q "0" && echo "$OUTPUT" | grep -q "2"'

run_test "Test 24: Invalid syntax handling" \
    "$BIN_PATH -c 'if true; then; else fi'" 2 \
    'echo "$OUTPUT" | grep -q "Syntax error"'

run_test "Test 25: Exit with specific code" \
    "$BIN_PATH -c 'exit 42'" 42 'true'

echo "====================================="
echo "Complete test suite completed: $PASSED_TESTS/$TOTAL_TESTS tests passed."
exit $STATUS
