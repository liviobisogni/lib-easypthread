#!/bin/bash

# If the "build" directory does not exist, the script creates it
if [ ! -d "build" ]; then
    mkdir build
fi

cd build    # changes to the "build" directory
cmake ..    # generate the build system files
make        # compile the project.
