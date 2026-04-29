#!/bin/bash

# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

Tokenize() {
  echo "Tokenize start"
  mkdir ./tokenize/
  python ./tool/CreateLogIndexFile.py
  python ./tool/TokenizeAllFile.py
  py_ret=$?
  if [ $py_ret -ne 0 ]; then
    echo "[Error] Tokenization fail. Build process terminated."
    exit 1
  fi
}

Tokenize

mkdir build/
make clean
make # $@

if [ 0 -eq $? ]; then
  echo "Build Finished"
  cp -r build/*_compile .
  rm -rf build
  exit 0
else
  echo "Build Failed"
  exit 1
fi
