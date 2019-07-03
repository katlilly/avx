#include <stdio.h>
#include "simple10avx.h"
#include "fls.h"


void Simple10avx::print_table()
{
  printf("\nSimple 10 AVX selector table:\n");
  for (int i = 0; i < num_selectors; i++)
    printf("%2d int packing: %d bits per dgap\n", table[i].intsper32,
	   table[i].bitwidth);
}


void Simple10avx::dgaps_to_bitwidths(int *dest, int *source, int length)
{
  // first docnum may be zero, in which case fls won't give correct
  // bitwidth
  if (source[0] == 0)
    dest[0] = 1;
  else
    dest[0] = fls(source[0]);
  
  for (int i = 1; i < length; i++)
    dest[i] = fls(source[i]);
}

int encode_one_word(uint32_t *dest, uint8_t *dest_selector,
		    int selector, int *source, int length)
{

}


int Simple10avx::encode(uint32_t *dest, uint8_t *selector, int *source,
			int length)
{
  // this is counted in 32 bit chunks, so for every "compressed word"
  // (except usualy the last one) this will be incremented by 16
  int compressed_length = 0;
  int column_bitwidth;
  int *where = source; // pointer to next dgap to compress

  if (length > 16)
  {
    printf("starting at %d\n", *source);
    int row = chose_selector(13);
    printf("chose selector row %d, %d ints per 32\n", row, table[row].intsper32);
    
    where += table[row].intsper32;
      
  }



    // deal with ends of lists / short lists later



  
  while (length > 16)
    {
      // chose next selector
      column_bitwidth = 0;
      for (int i = 0; i < 16; i++)
	column_bitwidth != source[i];


      for (int i = 0; i < length; i += 16)
	{
	  column_bitwidth = 0;
	  for (int j = 0; j < 16; j++)
	    column_bitwidth |= source[i+j];
	  //if (column_bitwidth
    
	  // encode one 512bit word

	  // increment compressedlength
	  compressed_length += 16;

	  length--;
	  // update number encoded, where
    
	}
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
