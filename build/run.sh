#!/bin/bash
make
cp libBASH_SHARED.so ../include/
gcc ../include/bash_detect_demo.c ../include/libBASH_SHARED.so -g3 -o demo_bash_c -lpthread
