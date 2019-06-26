#include <stdio.h>
#include "simple10avx.h"
#include "fls.h"

void Simple10avx::print_table()
{
  printf("\nSimple 10 AVX selector table:\n");
  for (int i = 0; i < num_selectors; i++)
    printf("%2d int packing: %d bits per int\n", table[i].intsper32,
	   table[i].bitwidth);
}

// int Simple10avx::make_table()
// {
//   table[0] = new int[2] {1, 32};
//   table[1] = new int[3] {2, 16, 16};
//   table[2] = new int[4] {3, 10, 10, 10};
//   table[3] = new int[5] {4, 8, 8, 8, 8};
//   table[4] = new int[6] {5, 6, 6, 6, 6, 6};
//   table[5] = new int[7] {6, 5, 5, 5, 5, 5, 5};
//   table[6] = new int[9] {8, 4, 4, 4, 4, 4, 4, 4, 4};
//   table[7] = new int[11] {10, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
//   table[8] = new int[17] {16, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
//   table[9] = new int[33] {32, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
// 			  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
//   return 0;
// }

void Simple10avx::dgaps_to_bitwidths(int *dest, int *source, int length)
{
  // first docnum may be zero, in which case fls won't give correct bitwidth
  if (source[0] == 0)
    dest[0] = 1;
  else
    dest[0] = fls(source[0]);
  
  for (int i = 1; i < length; i++)
    dest[i] = fls(source[i]);
}


int Simple10avx::encode(int *dest, uint8_t *selector, int *source, int length)
{
  // this is counted in 32 bit chunks, so for every "compressed word"
  // (except usualy the last one) this will be incremented by 16
  int compressed_length = 0;
  int column_bitwidth;
  int *where = source;
  
  while (length > 16)
  {
    // chose next selector
    column_bitwidth = 0;
    for (int i = 0; i < 16; i++)
      column_bitwidth != source[i];
    
    // encode one 512bit word

    // increment compressedlength
    compressed_length += 16;

    length--;
    // update number encoded, where
    
  }

  return compressed_length;
}

/* 
   Return the row number to use in the selector table
*/
int Simple10avx::chose_selector(int largest_column_width)
{
  int row;
  for (int i = 0; i < num_selectors; i++)
  {
    if (largest_column_width <= table[i].bitwidth)
      row = i;
    else
      break;
  }
  return row;
}
