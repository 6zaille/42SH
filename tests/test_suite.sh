#!/bin/sh

printf "Running complete 42sh test suite...\n"
printf "=====================================\n"
STATUS=0
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
BIN_PATH="./src/42sh"

if [ ! -f "$BIN_PATH" ]; then
    printf "Error: 42sh binary not found at %s\n" "$BIN_PATH"
    exit 1
fi

FAILED_TEST_NAMES=""

run_test() {
    TEST_NAME=$1
    COMMAND=$2
    EXPECTED_EXIT_CODE=$3
    CHECK_CMD=$4

    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    OUTPUT=$(eval "$COMMAND" 2>&1)
    ACTUAL_EXIT_CODE=$?

    if [ $ACTUAL_EXIT_CODE -eq $EXPECTED_EXIT_CODE ] && eval "$CHECK_CMD"; then
        COLOR="\033[32m" # Vert
        STATUS_MSG="CHECK"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        COLOR="\033[31m" # Rouge
        STATUS_MSG="ERROR"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        FAILED_TEST_NAMES="$FAILED_TEST_NAMES\n$TEST_NAME"
        STATUS=1
    fi

    RESET="\033[0m"
    if [ $TOTAL_TESTS -gt 9 ]; then
        printf "Test %d: %b%s%b\n" "$TOTAL_TESTS" "$COLOR" "$STATUS_MSG" "$RESET"
    else
        printf "Test %d:  %b%s%b\n" "$TOTAL_TESTS" "$COLOR" "$STATUS_MSG" "$RESET"
    fi
}

# Tests Echo Commands
run_test "Echo: Simple echo" \
    "$BIN_PATH -c 'echo Hello World'" 0 \
    'echo "$OUTPUT" | grep -q "Hello World"'

run_test "Echo: Echo with -n flag" \
    "$BIN_PATH -c 'echo -n Hello World'" 0 \
    '[ "$OUTPUT" = "Hello World" ]'

run_test "Echo: Echo with -e flag (newlines)" \
    "$BIN_PATH -c 'echo -e Hello\nWorld'" 0 \
    'echo "$OUTPUT" | grep -q "Hello" && echo "$OUTPUT" | grep -q "World"'

run_test "Echo: Echo with -e flag (tabs)" \
    "$BIN_PATH -c 'echo -e Hello\tWorld'" 0 \
    'echo "$OUTPUT" | grep -q "Hello" && echo "$OUTPUT" | grep -q "World"'

run_test "Echo: Echo with -E flag (escapes ignored)" \
    "$BIN_PATH -c 'echo -E Hello\nWorld'" 0 \
    'echo "$OUTPUT" | grep -q "HellonWorld"'

run_test "Echo: Combination of -n and -e" \
    "$BIN_PATH -c 'echo -n -e Hello\nWorld'" 0 \
    '[ "$OUTPUT" = "Hello\nWorld" ]'

run_test "Echo: Default behavior" \
    "$BIN_PATH -c 'echo Text with no flags'" 0 \
    'echo "$OUTPUT" | grep -q "Text with no flags"'

run_test "Echo: Empty input" \
    "$BIN_PATH -c 'echo '" 0 \
    '[ "$OUTPUT" = "" ]'

run_test "Echo: Only flags" \
    "$BIN_PATH -c 'echo -n -e -E'" 0 \
    '[ "$OUTPUT" = "" ]'

run_test "Echo: Invalid flags" \
    "$BIN_PATH -c 'echo -z Hello World'" 0 \
    '[ "$OUTPUT" = "-z Hello World" ]'

run_test "Echo: Mixed valid and invalid flags" \
    "$BIN_PATH -c 'echo -n -x Hello'" 0 \
    '[ "$OUTPUT" = "-x Hello" ]'

run_test "Echo: Special characters" \
    "$BIN_PATH -c 'echo \"Hello $USER!\"'" 0 \
    'echo "$OUTPUT" | grep -q "Hello"'

run_test "Echo: Unicode characters" \
    "$BIN_PATH -c 'echo 🚀 🌟'" 0 \
    'echo "$OUTPUT" | grep -q "🚀 🌟"'

run_test "Echo: Multi-line input" \
    "$BIN_PATH -c 'echo -e \"Line1\nLine2\nLine3\"'" 0 \
    'echo "$OUTPUT" | grep -q "Line1" && echo "$OUTPUT" | grep -q "Line3"'

# Tests Builtins
run_test "Builtin: true command" \
    "$BIN_PATH -c 'true'" 0 \
    'true'

run_test "Builtin: false command" \
    "$BIN_PATH -c 'false'" 1 \
    'true'

# Tests Semicolon Commands
run_test "Semicolon: Simple commands with semicolon" \
    "$BIN_PATH -c 'echo toto ; ls tests'" 0 \
    'echo "$OUTPUT" | grep -q "toto" && echo "$OUTPUT" | grep -q "test_suite.sh"'

run_test "Semicolon: Semicolon at the end of command" \
    "$BIN_PATH -c 'echo toto ; ls tests;'" 0 \
    'echo "$OUTPUT" | grep -q "toto" && echo "$OUTPUT" | grep -q "test_suite.sh"'

run_test "Semicolon: Multiple semicolons" \
    "$BIN_PATH -c 'echo first ; ; echo second ; ls tests'" 2 \
    'echo "$OUTPUT" | grep -q "Syntax error"'

run_test "Semicolon: Empty command between semicolons" \
    "$BIN_PATH -c 'echo hello ; ; echo world'" 2 \
    'echo "$OUTPUT" | grep -q "Syntax error"'

run_test "Semicolon: Commands with semicolon and comments" \
    "$BIN_PATH -c 'echo hello ; # comment\necho world'" 0 \
    'echo "$OUTPUT" | grep -q "hello"'

run_test "Semicolon: Semicolon with compound commands" \
    "$BIN_PATH -c 'if true; then echo success; fi ; echo done'" 0 \
    'echo "$OUTPUT" | grep -q "success" && echo "$OUTPUT" | grep -q "done"'

run_test "Semicolon: Escaped semicolon in a string" \
    "$BIN_PATH -c 'echo \"escaped ; semicolon\"'" 0 \
    'echo "$OUTPUT" | grep -q "escaped ; semicolon"'

run_test "Semicolon: Commands separated by newlines and semicolons" \
    "$BIN_PATH -c 'echo line1 ;\necho line2'" 0 \
    'echo "$OUTPUT" | grep -q "line1" && echo "$OUTPUT" | grep -q "line2"'

# Tests Command Substitution
run_test "Command Substitution: Nested substitution" \
    "$BIN_PATH -c 'echo \$(echo nested)'" 0 \
    'echo "$OUTPUT" | grep -q "nested"'

run_test "Command Substitution: Subshell variable isolation" \
    "$BIN_PATH -c 'VAR=outer; (VAR=inner; echo \$VAR); echo \$VAR'" 0 \
    'echo "$OUTPUT" | grep -q "inner" && echo "$OUTPUT" | grep -q "outer"'

run_test "Command Substitution: Redirection to file" \
    "$BIN_PATH -c 'echo output > test_file'; grep -q "output" test_file; rm test_file" 0 \
    'true'

# Tests Redirections
run_test "Redirection: Output redirection" \
    "$BIN_PATH -c 'echo Hello > test_redirection_1.txt'; grep -q 'Hello' test_redirection_1.txt && rm test_redirection_1.txt" 0 \
    'true'

run_test "Redirection: Input redirection" \
    "echo 'Input Test' > input.txt; $BIN_PATH -c 'cat < input.txt' > output.txt && rm input.txt" 0 \
    'grep -q "^Input Test$" output.txt && rm output.txt'

run_test "Redirection: Append redirection" \
    "echo 'First Line' > file.txt; $BIN_PATH -c 'echo Second Line >> file.txt'; grep -q '^Second Line$' file.txt && rm file.txt" 0 \
    'true'

# Tests Conditional Statements
run_test "Conditional: If-then-else syntax" \
    "$BIN_PATH -c 'if true; then echo yes; else echo no; fi'" 0 \
    'echo "$OUTPUT" | grep -q "yes"'

run_test "Conditional: Invalid syntax handling" \
    "$BIN_PATH -c 'if true; then; else fi'" 2 \
    'echo "$OUTPUT" | grep -q "Syntax error"'

run_test "Conditional: Exit with specific code" \
    "$BIN_PATH -c 'exit 42'" 42 'true'

# Tests Comments
run_test "Comment: Single line comment only" \
    "$BIN_PATH -c '# This is a comment'" 0 \
    '[ "$OUTPUT" = "" ]'

run_test "Comment: Comment after a command" \
    "$BIN_PATH -c 'echo Hello # This is a comment'" 0 \
    'echo "$OUTPUT" | grep -q "Hello"'

run_test "Comment: Comment with no space after #" \
    "$BIN_PATH -c 'echo Hello#ThisIsComment'" 0 \
    '[ "$OUTPUT" = "Hello#ThisIsComment" ]'

run_test "Comment: Comment on a separate line" \
    "$BIN_PATH -c '# First comment\necho Hello'" 0 \
    'echo "$OUTPUT" | grep -q ""'

run_test "Comment: Mixed commands and comments" \
    "$BIN_PATH -c 'echo Line1; # Comment in between\necho Line2'" 0 \
    'echo "$OUTPUT" | grep -q "Line1"'

run_test "Comment: Comment with special characters" \
    "$BIN_PATH -c '# $USER and #123 are ignored\necho Hello'" 0 \
    'echo "$OUTPUT" | grep -q ""'

run_test "Comment: Comment in if-else block" \
    "$BIN_PATH -c 'if true; then # Comment in then\n echo yes; else echo no; fi'" 0 \
    'echo "$OUTPUT" | grep -q "yes"'

run_test "Comment: Comment at the end of the file" \
    "$BIN_PATH -c 'echo Last line # End of script'" 0 \
    'echo "$OUTPUT" | grep -q "Last line"'

run_test "Comment: Empty line followed by comment" \
    "$BIN_PATH -c '\n# This is a standalone comment'" 0 \
    '[ "$OUTPUT" = "" ]'

run_test "Comment: Multiple comments in script" \
    "$BIN_PATH -c '# First comment\necho Line1\n# Second comment\necho Line2'" 0 \
    'echo "$OUTPUT" | grep -q ""'

# Tests Pipelines
run_test "Pipeline: Simple pipeline" \
    "$BIN_PATH -c 'echo Hello | grep Hello'" 0 \
    'echo "$OUTPUT" | grep -qx "Hello"'

run_test "Pipeline: Three commands" \
    "$BIN_PATH -c 'echo Hello | tr H J | grep Jello'" 0 \
    'echo "$OUTPUT" | grep -qx "Jello"'

# Tests Negation
run_test "Negation: Simple negation" \
    "$BIN_PATH -c '! false'" 0 \
    '[ "$ACTUAL_EXIT_CODE" -eq 0 ]'

run_test "Negation: Negated pipeline" \
    "$BIN_PATH -c '! echo Hello | false'" 1 \
    '[ "$ACTUAL_EXIT_CODE" -eq 1 ]'

# Tests Quotes
run_test "Quotes: Double quotes with variables" \
    "$BIN_PATH -c 'VAR=42; echo \"Value: \$VAR\"'" 0 \
    'echo "$OUTPUT" | grep -qx "Value: 42"'

run_test "Quotes: Escape characters" \
    "$BIN_PATH -c 'echo \"Hello \\\"World\\\"\"'" 0 \
    'echo "$OUTPUT" | grep -qx "Hello \"World\""'

# Tests Loops
run_test "Loop: While loop" \
    "$BIN_PATH -c 'COUNTER=0; while [ \$COUNTER -lt 3 ]; do echo \$COUNTER; COUNTER=\$((COUNTER + 1)); done'" 0 \
    'echo "$OUTPUT" | grep -qx "0" && echo "$OUTPUT" | grep -qx "2"'

run_test "Loop: Until loop" \
    "$BIN_PATH -c 'COUNTER=5; until [ \$COUNTER -eq 0 ]; do echo \$COUNTER; COUNTER=\$((COUNTER - 1)); done'" 0 \
    'echo "$OUTPUT" | grep -qx "5" && echo "$OUTPUT" | grep -qx "1"'

run_test "Loop: For loop" \
    "$BIN_PATH -c 'for i in 1 2 3; do echo \$i; done'" 0 \
    'echo "$OUTPUT" | grep -qx "1" && echo "$OUTPUT" | grep -qx "3"'

run_test "Loop: Nested loops" \
    "$BIN_PATH -c 'for i in 1 2; do for j in A B; do echo \$i\$j; done; done'" 0 \
    'echo "$OUTPUT" | grep -qx "1A" && echo "$OUTPUT" | grep -qx "2B"'

# Tests Operators
run_test "Operator: Logical AND" \
    "$BIN_PATH -c 'true && echo Success'" 0 \
    'echo "$OUTPUT" | grep -qx "Success"'

run_test "Operator: Logical OR" \
    "$BIN_PATH -c 'false || echo Failure'" 0 \
    'echo "$OUTPUT" | grep -qx "Failure"'

# Tests Variables
run_test "Variable: Variable assignment" \
    "$BIN_PATH -c 'VAR=test; echo \$VAR'" 0 \
    'echo "$OUTPUT" | grep -qx "test"'

run_test "Variable: Special variable \$?" \
    "$BIN_PATH -c 'false; echo \$?'" 0 \
    'echo "$OUTPUT" | grep -qx "1"'

run_test "Variable: Special variable \$\$" \
    "$BIN_PATH -c 'echo $$'" 0 \
    '[ "$OUTPUT" -eq "$$" ]'

# Tests Combined
run_test "Combined: Pipeline and redirection" \
    "$BIN_PATH -c 'echo Hello | tr H J > output.txt'; grep -qx \"Jello\" output.txt && rm output.txt" 0 \
    'true'

# Final Report
echo "====================================="
PERCENT_PASSED=$((PASSED_TESTS * 100 / TOTAL_TESTS))

if [ $PERCENT_PASSED -ge 90 ]; then
    COLOR="\033[32m" # Vert
elif [ $PERCENT_PASSED -ge 50 ]; then
    COLOR="\033[33m" # Jaune
else
    COLOR="\033[31m" # Rouge
fi

RESET="\033[0m"
echo -e "${COLOR}Complete test suite completed: $PASSED_TESTS/$TOTAL_TESTS tests passed ($PERCENT_PASSED%).${RESET}"

if [ $FAILED_TESTS -gt 0 ]; then
    echo -e "\033[31mFailed tests:${RESET}$FAILED_TEST_NAMES"
fi

exit 0
