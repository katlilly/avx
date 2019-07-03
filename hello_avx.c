#include <immintrin.h>
#include <stdio.h>

void print_512word_as_32ints(__m512i word)
{
  int *number = (int *) &word;
  for (int i = 0; i < 16; i++)
    printf("%d ", number[i]);
  printf("\n");
}


int main() {

  /* Initialize the two argument vectors */
  __m256 evens = _mm256_set_ps(2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0);
  __m256 odds = _mm256_set_ps(1.0, 3.0, 5.0, 7.0, 9.0, 11.0, 13.0, 15.0);
  //__m256 zeros = _mm256_setzero_ps();
  
  /* Compute the difference between the two vectors */
  __m256 result = _mm256_sub_ps(evens, odds);

  /* Display the elements of the result vector */
  float* f = (float*)&result;
  printf("%f %f %f %f %f %f %f %f\n\n",
    f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7]);


  int *source = malloc(32 * sizeof(*source));
  for (int i = 0; i < 32; i++)
    source[i] = i+1;

  __m512i indexvector = _mm512_setzero_epi32();
  print_512word_as_32ints(indexvector);

  indexvector = _mm512_setr4_epi32(1, 2, 3, 4);
  print_512word_as_32ints(indexvector);

  __m512i gathered = _mm512_i32gather_epi32(indexvector, source, 4);

  print_512word_as_32ints(gathered);

  __m512i manuallyset = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6,
					 5, 4, 3, 2, 1, 0);
  print_512word_as_32ints(manuallyset);
  
  printf("\n");
  
  return 0;
}
