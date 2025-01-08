# Init the 41sh project
AC_INIT([autotools], [0.0], [jules.lange@epita.fr])

# Setup Automake
AM_INIT_AUTOMAKE([subdir-objects] [foreign])

# Pretty display of Makefile rules
AM_SILENT_RULES([yes])

# Enable ar for Makefile
AM_PROG_AR

# Check if ranlib is available
AC_PROG_RANLIB

# Check if a C compiler is available
AC_PROG_CC

# Check if a compiler has this list of flags
AX_COMPILER_FLAGS([], [], [], [-Wall -Wextra -Werror -Wvla -pedantic -std=c98])

# List Makefiles in subdirectories
AC_CONFIG_FILES([
                    Makefile
                    src/Makefile
                    src/execution/Makefile
                    src/lexer/Makefile
                    src/parser/Makefile
                    src/utils/Makefile
                ])
AC_OUTPUT
