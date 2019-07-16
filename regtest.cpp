#include <stdio.h>
#include <random>
#include "simple10avx.h"

/*
  Print integers in rows of 16 to ease testing
*/
void print_postings_list(int *list, int length)
{
  for (int i = 0; i < length; i++)
  {
    if (i % 16 == 0)
      printf("\n");
    printf("%2d ", list[i]);
  }
  printf("\n");
}

/* 
   Remove zeros and negative numbers from my sample distribution
*/
int abs(int input)
{
  if (input == 0)
    return 1;
  if (input < 0)
    return input * -1;
  else
    return input;
}

/*
  Return largest value of two integers
*/
int max(int a, int b)
{
  return a > b ? a : b;
}



int main(void)
{
  /*
    Create some test data and print it to screen
   */
  uint length = 200;
  int *postingslist = new int[length + 16];
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(32, 20);
  for (uint i = 0; i < length; i++)
    postingslist[i] = abs((int) distribution(generator));

  /* 
     Add 16 zeros to the end, makes it easier for now, but this is not
     the way to do it, too slow and not smart
  */
  for (uint i = length; i < length+16; i++)
    postingslist[i] = 0;
  //print_postings_list(postingslist, length+16);
  

  Simple10avx *compressor = new Simple10avx();
    
  /*
    Encode one postings list
  */
  uint32_t *encoded = new uint32_t[max(16, length)]; 
  uint8_t *selectors = new uint8_t[length];          
  uint32_t num_dgaps_compressed;
  num_dgaps_compressed = compressor->encode(encoded, postingslist, postingslist+length,
					    selectors);
  /* 
    Check for correct number of dgaps encoded 
   */
  if (length != num_dgaps_compressed)
  {
    printf("failed to compress correct number of dgaps\n");
    return 1;
  }

  /*
    Decompress one postings list
   */
  uint32_t *decoded = new uint32_t[length * 2];
  uint dgaps_decompressed = compressor->decode(decoded, encoded,
					      encoded + compressor->num_compressed_32bit_words,
					      selectors, 4);

  if (num_dgaps_compressed != dgaps_decompressed)
  {
    printf("failed to decompress correct number of dgaps\n");
    return 2;
  }
  
  for (uint i = 0; i < dgaps_decompressed; i++)
    if (decoded[i] != (uint) postingslist[i])
    {  
      printf("failed to decode correctly\n");
      return 3;
    }

  printf("All tests pass\n");
  return 0;
}
