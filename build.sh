#!/bin/bash

SCRIPT_DIR="$(dirname "$0")"
BUILD_DIR="$SCRIPT_DIR/build"

usage() {
    echo "Usage: $0 <command>"
    echo "Commands:"
    echo "  build       : Build the project."
    echo "  configure   : Run CMake configuration."
    echo "  reconfigure : Force re-run CMake configuration (clears build directory)."
    exit 1
}

create_build_dir() {
    if [ ! -d "$BUILD_DIR" ]; then
        echo "Creating build directory: $BUILD_DIR"
        mkdir -p "$BUILD_DIR"
    fi
}

build_project() {
    create_build_dir
    echo "Building the project in $BUILD_DIR..."
    cd "$BUILD_DIR" || exit 1
    cmake --build .
    cd "$SCRIPT_DIR" || exit 1
}

configure_project() {
    create_build_dir
    echo "Running CMake configuration in $BUILD_DIR..."
    cd "$BUILD_DIR" || exit 1
    cmake .
    cd "$SCRIPT_DIR" || exit 1
}

reconfigure_project() {
    echo "Force re-configuring the project..."
    if [ -d "$BUILD_DIR" ]; then
        echo "Removing existing build directory: $BUILD_DIR"
        rm -rf "$BUILD_DIR"
    fi
    configure_project
}

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
