#!/bin/sh

g++ -march=native -std=c++11 -fprofile-arcs -ftest-coverage -o testavxSimple10 simple10avx.cpp testavxsimple10.cpp

./testavxSimple10
gcov simple10avx.cpp testavxsimple10.cpp

#-march=knl ?? or sandybridge 
