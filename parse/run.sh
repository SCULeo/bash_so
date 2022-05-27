#!/bin/bash
bison --yacc -dv parse.y
cp y.tab.h ../include
cp y.tab.c ../src

