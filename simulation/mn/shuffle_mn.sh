#!/bin/bash

# This script will print the names of all files in the current directory

# Go to the directory (if you want to specify a different directory, change '.' to the path of the directory)
cd ./mn

# List all files
for file in *.csv; do
    if [ -f "$file" ]; then
        echo "$file"
        newfilename="${file%.csv}_shuffled.csv"
        python3 ../scripts/shuffle.py -fi $file -fo $newfilename
    fi
done
