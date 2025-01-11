#!/bin/sh

echo "Running 42sh test suite..."
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

# Tests existants
run_test "Test 1: Simple echo" \
    "$BIN_PATH -c 'echo Hello World'" 0 \
    'echo "$OUTPUT" | grep -q "Hello World"'

run_test "Test 2: Builtin true" \
    "$BIN_PATH -c 'true'" 0 'true'

run_test "Test 3: Builtin false" \
    "$BIN_PATH -c 'false'" 1 'true'

run_test "Test 4: Command list" \
    "$BIN_PATH -c 'echo foo; echo bar'" 0 \
    'echo "$OUTPUT" | grep -q "foo" && echo "$OUTPUT" | grep -q "bar"'

# Nouveaux tests
run_test "Test 5: If-then-else" \
    "$BIN_PATH -c 'if true; then echo success; else echo fail; fi'" 0 \
    'echo "$OUTPUT" | grep -q "success"'

run_test "Test 6: Invalid syntax" \
    "$BIN_PATH -c 'if true then'" 2 \
    'echo "$OUTPUT" | grep -q "Syntax error"'

run_test "Test 7: Builtin exit" \
    "$BIN_PATH -c 'exit 42'" 42 'true'

run_test "Test 8: Redirection to file" \
    "$BIN_PATH -c 'echo Hello > test_file'; grep -q "Hello" test_file; rm test_file" 0 'true'

run_test "Test 9: Command chaining with &&" \
    "$BIN_PATH -c 'true && echo success'" 0 \
    'echo "$OUTPUT" | grep -q "success"'

run_test "Test 10: Command chaining with ||" \
    "$BIN_PATH -c 'false || echo success'" 0 \
    'echo "$OUTPUT" | grep -q "success"'

run_test "Test 11: Parsing error with unexpected token" \
    "$BIN_PATH -c 'echo foo; ; echo bar'" 2 \
    'echo "$OUTPUT" | grep -q "Flux non terminé correctement"'

run_test "Test 12: Echo with escapes (-e)" \
    "$BIN_PATH -c 'echo -e Hello\\nWorld'" 0 \
    'echo "$OUTPUT" | grep -q "Hello" && echo "$OUTPUT" | grep -q "World"'

run_test "Test 13: Single quotes handling" \
    "$BIN_PATH -c 'echo '\''single quotes'\'''" 0 \
    'echo "$OUTPUT" | grep -q "single quotes"'

run_test "Test 14: Verbose mode logging" \
    "VERBOSE=1 $BIN_PATH -c 'true'" 0 \
    'echo "$OUTPUT" | grep -q "\\[VERBOSE\\]"'

run_test "Test 15: Multiple commands with semicolons" \
    "$BIN_PATH -c 'echo cmd1; echo cmd2; echo cmd3'" 0 \
    'echo "$OUTPUT" | grep -q "cmd1" && echo "$OUTPUT" | grep -q "cmd2" && echo "$OUTPUT" | grep -q "cmd3"'

run_test "Test 16: Invalid built-in command" \
    "$BIN_PATH -c 'invalid_cmd'" 127 \
    'echo "$OUTPUT" | grep -q "commande inconnue"'

echo "====================================="
echo "Test suite completed: $PASSED_TESTS/$TOTAL_TESTS tests passed."
exit $STATUS