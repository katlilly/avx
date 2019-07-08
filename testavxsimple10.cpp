#include <stdio.h>
#include <random>
#include "simple10avx.h"

/*
  print integers in groups of 16 to ease testing
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


int main(void)
{
  /*
    Create some test data and print it to screen
   */
  int length = 200;
  int *postingslist = new int[length];
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(32, 20);
  for (int i = 0; i < length; i++)
    postingslist[i] = abs((int) distribution(generator));
  print_postings_list(postingslist, length);
  
  /*
    Initialise a "simple10" avx compressor and show the table
  */
  Simple10avx *compressor = new Simple10avx();
  compressor->print_table();
  
  /*
    Encode one postings list
   */
  // note encoded length could be 16 times longer, but not in current test setup
  uint32_t *encoded = new uint32_t[length];
  uint8_t *selector = new uint8_t[length]; // this is max possible length
  uint32_t num_dgaps_compressed;
  num_dgaps_compressed = compressor->encode(encoded, postingslist, postingslist+length, selector);

  if (length != num_dgaps_compressed)
    exit(printf("%d != %d\n", length, num_dgaps_compressed));
  
  printf("\nselectors used:\n");
  for (int i = 0; i < compressor->num_compressed_512bit_words; i++)
  {
    printf("selector %d: %d bits per int, %d ints per 32\n", selector[i],
  	   compressor->table[selector[i]].bitwidth, compressor->table[selector[i]].intsper32);
  }

  printf("%d\n", compressor->num_compressed_32bit_words);
  for (int i = 0; i < compressor->num_compressed_32bit_words; i++)
  {
    printf("%u\n", encoded[i]);
  }

    
  return 0;
}
