#!/usr/bin/env python
# coding=utf-8
import os
for line in open("a.txt", encoding='utf-8'):
    f = open('test.txt', 'w')
    f.write(line)
    f.close()
    os.system("./bash_c test.txt")
