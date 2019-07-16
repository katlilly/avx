#!/bin/sh

#export CODECOV_TOKEN="d330997c-36e9-4a4a-bf7f-0a5fb83fd603"

g++ -march=native -std=c++11 -fprofile-arcs -ftest-coverage -o regtest regtest.cpp simple10avx.cpp

./regtest
gcov regtest.cpp simple10avx.cpp

#bash <(curl -s https://codecov.io/bash)

bash <(curl -s https://codecov.io/bash) -t d330997c-36e9-4a4a-bf7f-0a5fb83fd603
