#!/bin/bash

# Loop over all files with .um extension
for file in *.um; do
  # Check if file exists (in case no .um files are found)
  [ -e "$file" ] || continue

  echo "Running ./um on $file"
  ./um "$file"
done