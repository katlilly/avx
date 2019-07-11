main:
	g++ -Wall -march=native -o testavxSimple10 simple10avx.cpp testavxsimple10.cpp

hello:
	gcc -march=native -o hello_avx hello_avx.c




#gcc -mavx -o hello_avx hello_avx.c

# echo | gcc -dM -E - -march=native | grep AVX
