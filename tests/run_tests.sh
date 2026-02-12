#!/bin/bash
set -e
gcc -I./tests/include -I./include -o tests/test_favicon tests/test_favicon.c
./tests/test_favicon
rm tests/test_favicon
