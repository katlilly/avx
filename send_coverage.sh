#!/bin/bash

g++ -march=knl -Wall -std=c++11 -fprofile-arcs -ftest-coverage -o regtest simple10avx.cpp regtest.cpp

./regtest

gcov regtest.cpp simple10avx.cpp

export CODECOV_TOKEN="d330997c-36e9-4a4a-bf7f-0a5fb83fd603"

bash <(curl -s https://codecov.io/bash)
