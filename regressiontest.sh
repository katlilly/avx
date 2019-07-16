#!/bin/sh

g++ -Wall -march=native -o regtest simple10avx.cpp regtest.cpp
./regtest
