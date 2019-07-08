#include <stdio.h>
#include <immintrin.h>
#include "simple10avx.h"
#include "fls.h"

void Simple10avx::print_512word_as_32ints(__m512i word)
{
  int *number = (int *) &word;
  for (int i = 0; i < 16; i++)
    printf("%d ", number[i]);
  printf("\n");
  
}

int Simple10avx::min(int a, int b)
{
  return a < b ? a : b;
}


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



/* 
   Return the row number to use in the selector table
*/
int Simple10avx::chose_selector(int *raw, int* end)
{
  int length = end - raw;
  int bitsused = 0;
  int column_bitwidth;
  int column;
  int largest_column_bw = 0;

  for (int i = 0; i < length; i += 16)
  {
    column = 0;
    for (int j = 0; j < 16; j++)
    {
      if ((raw + i + j) < end)
	column |= raw[i+j];
      column_bitwidth = fls(column);
    }
    if (column_bitwidth > largest_column_bw)
      largest_column_bw = column_bitwidth;
    bitsused += column_bitwidth; 
    if (bitsused > 32)
      break;
  }

  uint8_t row;
  for (int i = 0; i < num_selectors; i++)
  {
    if (largest_column_bw <= table[i].bitwidth)
      row = i;
    else
      break;
  }
  return row;
}


// return value will be number of ints compressed, so we know where to
// move raw to.  each time we return from this function we need to
// increment selector by 1 byte and dest by 64 bytes (16 ints / 512 bits)
int Simple10avx::encode_one_word(uint32_t *dest, int *raw, int* end, uint8_t *selector)
{
  // will need to pack with zeros when we get to the end of a list to fill the register
  // blocks of compressed data will be preceded by their compressed length
  
  int length = end - raw;
  printf("\nlength = %d\n", length);
  printf("next value: %d\n", *raw);
  printf("last value: %d\n", *(end - 1));

  // chose and write out selector
  uint8_t selector_row = chose_selector(raw, end);
  selector[0] = selector_row;
  printf("chose selector %d, %d bits per int\n", selector_row, table[selector_row].bitwidth);

  // do the compression in a 512 bit register
  __m512i compressedword = _mm512_setzero_epi32();
  __m512i indexvector = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
  //__m512i indexvector = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  __m512i columnvector;

  // will need to check for overrun here... and add zeros instead of
  // whatever rubbish is after the end of my raw data
  for (int i = 0; i < table[selector_row].intsper32; i++)
  {
    // gather next 16 ints into a 512 bit register
    columnvector = _mm512_i32gather_epi32(indexvector, raw, 4);

    printf("column vector prior to shift:\n");
    print_512word_as_32ints(columnvector);
    
    // left shift input to correct "column"
    printf("shift amount: %d\n", table[selector_row].bitwidth * i);
    columnvector = _mm512_slli_epi32(columnvector, table[selector_row].bitwidth * i);

    printf("column vector after shift:\n");
    print_512word_as_32ints(columnvector);
    
    // pack this column of 16 dgaps into compressed 512 bit word
    compressedword = _mm512_or_epi32(compressedword, columnvector);
    raw += 16;
  }

  // write out compressed data to destination with scatter instruction
  _mm512_i32scatter_epi32(dest, indexvector, compressedword, 4); // scale = 1 or 4 ??

  // record size of compressed data and return number of dgaps compressed
  num_compressed_512bit_words++;
  num_compressed_32bit_words += 16; 
  return min(length, 16 * table[selector_row].intsper32);
}


int Simple10avx::encode(uint32_t *dest, int *raw, int* end, uint8_t *selector)
{
  int dgaps_compressed = 0;

  while (raw + dgaps_compressed < end)
  {
    dgaps_compressed += encode_one_word(dest, raw + dgaps_compressed, end, selector++);
    dest += 16;
    printf("dgaps compressed: %d\n", dgaps_compressed);
  }

  return dgaps_compressed;
}

