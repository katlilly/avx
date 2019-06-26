#include <immintrin.h>
#include <stdio.h>
#include <random>
#include "simple10avx.h"
#include "fls.h"

void print_512word_as_32ints(__m512i word)
{
  int *number = (int *) &word;
  for (int i = 0; i < 16; i++)
    printf("%d ", number[i]);
  printf("\n");
  
}

int abs(int input)
{
  if (input == 0)
    return 1;
  if (input < 0)
    return input * -1;
  else
    return input;
}

int main(void)
{
  int *source = new int[32];
  for (int i = 0; i < 32; i++)
    source[i] = i+1;

  //__m512i indexvector = _mm512_setzero_epi32();
  //indexvector = _mm512_setr4_epi32(1, 2, 3, 4);
  //__m512i gathered = _mm512_i32gather_epi32(indexvector, source, 4);
  //print_512word_as_32ints(gathered);

  Simple10avx *avxcompressor = new Simple10avx();
  int **table = new int*[10];
  avxcompressor->make_table(table);
  //printf("%d\n", avxcompressor->num_selectors);
  //avxcompressor->print_table(table);

  std::default_random_engine generator;
  std::normal_distribution<double> distribution(32, 20);

  int testlength = 100;
  int *testdata = new int[testlength + 16];
  for (int i = 0; i < 100; i++)
    testdata[i] = abs((int) distribution(generator));
  // pack 16 zeros at end of array (makes life easy for now)
  for (int i = testlength; i < testlength+16; i++)
    testdata[i] = 0;

  int *testbitwidths = new int[testlength + 16];
  avxcompressor->dgaps_to_bitwidths(testbitwidths, testdata, testlength + 16);

  printf("test data:");
  for (int i = 0; i < 116; i++)
  {
    if (i % 16 == 0)
      printf("\n");
    printf("%2d ", testdata[i]);
  }
  printf("\n\n");

  // for (int i = 0; i < 116; i++)
  // {
  //   if (i % 16 == 0)
  //     printf("\n");
  //   printf("%d ", testbitwidths[i]);

  // }
  // printf("\n");

  // compress one 512 bit vector
  __m512i compressedword = _mm512_setzero_epi32();

  int wordbits = 0;
  int column_bitwidth;
  int largest_column_bw = 0;
  
  for (int j = 0; j < testlength; j += 16)
  {
    column_bitwidth = 0;
    for (int i = 0; i < 16; i++)
      column_bitwidth |= testdata[i+j];
    //printf("bits needed for %dth column: %d\n", j/16, fls(column_bitwidth));
    if (column_bitwidth > largest_column_bw)
      largest_column_bw = fls(column_bitwidth);
    wordbits += fls(column_bitwidth);
    if (wordbits > 32)
      break;
  }
  //printf("largest bitwidth (bitwidth needed): %d\n", largest_column_bw);

  int row = avxcompressor->chose_selector(table, largest_column_bw);
  //printf("will use selector %d (%d %d-bit ints per 32bits)\n", row, table[row][0], table[row][1]);


  int intsper32 = 4;
  int bitsperint = 8;
  __m512i indexvector = _mm512_setzero_epi32();
  indexvector = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
  __m512i columnvector;
  printf("start compression\n");
  int *rawdatapointer = testdata;

  // encode one 512 bit compressed word
  for (int i = 0; i < intsper32; i++)
  {
    // gather next 16 ints into a 512 bit register
    columnvector = _mm512_i32gather_epi32(indexvector, rawdatapointer, 4);
    print_512word_as_32ints(columnvector);

    // left shift input to correct "column"
    columnvector = _mm512_slli_epi32(columnvector, bitsperint * i);
    
    // pack this column of 16 dgaps into compressed 512 bit word
    compressedword = _mm512_or_epi32(compressedword, columnvector);
    rawdatapointer = rawdatapointer + 16;

  }

  __m512i temp_decompress;
  int *decompressed = new int[16*intsper32]; // set these to zero
  int *startoutput = decompressed;
  int decompressed_length = 0;
  int mask = 0xff;
  __m512i maskvector = _mm512_set1_epi32(mask);
  printf("\nstart decompression\n");
  // decode ints from one 512 bit word
  for (int i = 0; i < intsper32; i++)
  {
    // get 16 numbers by ANDing mask with compressedword
    temp_decompress = _mm512_and_epi32(compressedword, maskvector);
    print_512word_as_32ints(temp_decompress);
    
    // put those numbers in output array using scatter instruction
    _mm512_i32scatter_epi32(decompressed, indexvector, temp_decompress, 4);
    
    // right shift the compressed word
    compressedword = _mm512_srli_epi32(compressedword, bitsperint);

    
    // update decompressed length - will need to check for up to 15
    // zeros after finish decompressing to find actual length
    decompressed += 16;
  }

  
  printf("\nDecompressed length: %d\nDecompressed test data:", decompressed - startoutput);
  for (int i = 0; i < 16*4; i++)
    {
      if (i % 16 == 0)
	printf("\n");
      printf("%2d ", startoutput[i]);
    }
  printf("\n");
	 

  
  for (int i = 0; i < 10; i++)
    delete [] table[i];
  delete [] table;
      
  delete avxcompressor;
  delete [] source;
  delete [] testdata;
  delete [] testbitwidths;
  
  return 0;
}
