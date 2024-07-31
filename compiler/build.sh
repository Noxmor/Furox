#!/bin/bash

DIR="$(dirname "$0")/src"

FILES=$(find "$DIR" -type f -name "*.frx")

rm frx
rm -rf "furox-c"
../bootstrap/bin/Furox-Debug-x86_64-Linux/Furox $FILES
mv a.out frx
