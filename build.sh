#!/bin/bash

# Exit immediately if any command fails.
set -e

# Define project directories.
SCRIPT_DIR="$(dirname "$0")"
BUILD_DIR="$SCRIPT_DIR/build"

# Display usage information.
usage() {
    echo "Usage: $0 <command>"
    echo "Commands:"
    echo "  build       : Build the project."
    echo "  configure   : Run CMake configuration."
    echo "  reconfigure : Force re-run CMake configuration (clears build directory)."
    exit 1
}

# Ensure build directory exists.
create_build_dir() {
    if [ ! -d "$BUILD_DIR" ]; then
        echo "Creating build directory: $BUILD_DIR"
        mkdir -p "$BUILD_DIR"
    fi
}

# Build the project.
build_project() {
    create_build_dir
    echo "Building project in '$BUILD_DIR'..."
    pushd "$BUILD_DIR" >/dev/null
    cmake --build .
    popd >/dev/null
    echo "Build complete."
}

# Configure the project.
configure_project() {
    create_build_dir
    echo "Configuring project in '$BUILD_DIR'..."
    pushd "$BUILD_DIR" >/dev/null
    cmake ..
    popd >/dev/null
    echo "Configuration complete."
}

# Force reconfigure the project.
reconfigure_project() {
    echo "Reconfiguring project (removing '$BUILD_DIR')..."
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
    configure_project
    echo "Reconfiguration complete."
}

# Main script logic.
if [ -z "$1" ]; then
    usage
fi

command="$1"
case "$command" in
"build")
    build_project
    ;;
"configure")
    configure_project
    ;;
"reconfigure")
    reconfigure_project
    ;;
*)
    echo "Error: Unknown command '$command'"
    usage
    ;;
esac

exit 0
