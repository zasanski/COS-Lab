#!/bin/bash

SCRIPT_DIR="$(dirname "$0")"
EXCLUDE_FILE="$SCRIPT_DIR/no_format.txt"
EXCLUDE_PATTERNS=()

# Exclude the build directory
EXCLUDE_PATTERNS+=("-not")
EXCLUDE_PATTERNS+=("-path")
EXCLUDE_PATTERNS+=("$SCRIPT_DIR/build/*")

# Read excluded paths from no_format.txt into an array
if [ -f "$EXCLUDE_FILE" ]; then
    echo "The following files from 'no_format.txt' will be excluded:"
    while IFS= read -r line; do
        if [[ -n "$line" ]]; then
            excluded_path="$SCRIPT_DIR/$line"
            echo "  Excluding path: $excluded_path"
            EXCLUDE_PATTERNS+=("-not")
            EXCLUDE_PATTERNS+=("-path")
            EXCLUDE_PATTERNS+=("$excluded_path")
        fi
    done <"$EXCLUDE_FILE"
else
    echo "No 'no_format.txt' file found."
fi

find "$SCRIPT_DIR" -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.cpp" \) "${EXCLUDE_PATTERNS[@]}" -print0 | while IFS= read -r -d $'\0' file; do
    echo "Found file: $file"
    clang-format -i "$file"
done

exit 0
