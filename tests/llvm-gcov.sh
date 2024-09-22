#!/bin/bash

if /usr/bin/which -s llvm-cov; then
  exec llvm-cov gcov "$@"
elif [ -f /usr/lib/llvm-13/bin/llvm-cov ]; then
  exec /usr/lib/llvm-13/bin/llvm-cov gcov "$@"
elif [ -f /usr/lib/llvm-14/bin/llvm-cov ]; then
  exec /usr/lib/llvm-14/bin/llvm-cov gcov "$@"
elif [ -f /usr/lib/llvm-15/bin/llvm-cov ]; then
  exec /usr/lib/llvm-15/bin/llvm-cov gcov "$@"
elif [ -f /usr/lib/llvm-16/bin/llvm-cov ]; then
  exec /usr/lib/llvm-16/bin/llvm-cov gcov "$@"
elif [ -f /usr/lib/llvm-17/bin/llvm-cov ]; then
  exec /usr/lib/llvm-17/bin/llvm-cov gcov "$@"
elif [ -f /usr/lib/llvm-18/bin/llvm-cov ]; then
  exec /usr/lib/llvm-18/bin/llvm-cov gcov "$@"
else
  exec gcov "$@"
fi
