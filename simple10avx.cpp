#include <stdio.h>
#include "simple10avx.h"
#include "fls.h"

void Simple10avx::print_table(int **table)
{
  printf("\nSimple 10 AVX selector table:\n");
  for (int i = 0; i < num_selectors; i++)
  {
    printf("%2d int packing: ", table[i][0]);
    for (int j = 1; j < table[i][0] + 1; j++)
      printf("%d ", table[i][j]);
    printf("\n");
  }
  printf("\n");
}

int Simple10avx::make_table(int **table)
{
  table[0] = new int[2] {1, 32};
  table[1] = new int[3] {2, 16, 16};
  table[2] = new int[4] {3, 10, 10, 10};
  table[3] = new int[5] {4, 8, 8, 8, 8};
  table[4] = new int[6] {5, 6, 6, 6, 6, 6};
  table[5] = new int[7] {6, 5, 5, 5, 5, 5, 5};
  table[6] = new int[9] {8, 4, 4, 4, 4, 4, 4, 4, 4};
  table[7] = new int[11] {10, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
  table[8] = new int[17] {16, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
  table[9] = new int[33] {32, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  return 0;
}

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


int Simple10avx::encode(int *dest, int *source, int length)
{
  int compressed_length = 0;
  


  return compressed_length;
}

/* 
   Return the row number to use in the selector table
*/
int Simple10avx::chose_selector(int **table, int largest_column_width)
{
  int row;
  for (int i = 0; i < num_selectors; i++)
  {
    if (largest_column_width <= table[i][1])
      row = i;
    else
      break;
  }
  return row;
}
