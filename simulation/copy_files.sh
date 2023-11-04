#!/bin/bash

# Check if a name is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <name>"
    exit 1
fi

# Get the name from the command line argument
name=$1

# Create the directory
mkdir -p "$name"

# Copy files with the prefix of the given name into the directory
cp "${name}"* "${name}/"

echo "Files with prefix '${name}' have been copied to directory '${name}'"
