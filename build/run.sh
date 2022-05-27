#!/bin/bash
make
cp libBASH_SHARED.so ../include/
gcc ../include/testfor_bash.c ../include/libBASH_SHARED.so -g3 -o bash_c
