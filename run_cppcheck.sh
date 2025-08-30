#!/bin/bash
# Script to run cppcheck with proper configuration for CI

# Run cppcheck with proper include paths and suppressions
cppcheck --enable=all --error-exitcode=1 \
  --suppress=missingIncludeSystem \
  --suppress=unusedFunction \
  -I. -I./common -I./server -I./client -I./tests \
  client/ server/ common/ tests/

exit $?