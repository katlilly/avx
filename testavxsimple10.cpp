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
    Create some test data
   */
  int length = 100;
  int *postingslist = new int[length];
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(32, 20);
  for (int i = 0; i < 100; i++)
    postingslist[i] = abs((int) distribution(generator));
  print_postings_list(postingslist, length);
  
  /*
    Initialise a "simple10" avx compressor
   */
  Simple10avx *compressor = new Simple10avx();
  //compressor->print_table();

  /*
    Encode one postings list
   */
  // note encoded length could be 16 times longer, but not in current test setup
  int *encoded = new int[length];
  uint8_t selector;
  uint32_t compressed_length;
  compressed_length = compressor->encode(encoded, &selector, postingslist, length);
  printf("compressed length: %d\n", compressed_length);

  
  
  return 0;
}
