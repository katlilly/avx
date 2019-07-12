#include <stdio.h>
#include <immintrin.h>
#include <math.h>
#include "simple10avx.h"
#include "fls.h"

void Simple10avx::print_512word_as_32ints(__m512i word)
{
  uint32_t *number = (uint32_t *) &word;
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
    printf("row %d: pack %2d dgaps, %d bits per dgap\n", i, table[i].intsper32,
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
   Returns the row number to use in the selector table
   Takes pointers to start and end of a postings list
*/
int Simple10avx::chose_selector(int *raw, int* end)
{
  int length = end - raw;
  int bitsused = 0;
  int column_bitwidth;
  int largest_column_bw = 0;

  for (int i = 0; i < length; i += 16)
  {
    int column = 0;
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
  int length = end - raw;
  printf("\nremaining length = %d\n", length);
  printf("next value: %d\n", *raw);
  printf("last value: %d\n", *(end - 1));

  // chose and write out selector
  uint8_t selector_row = chose_selector(raw, end);
  selector[0] = selector_row;
  printf("chose selector %d, %d bits per int\n", selector_row, table[selector_row].bitwidth);


  /*
    Do the compression in a 512 bit register
  */
  __m512i compressedword = _mm512_setzero_epi32();
  __m512i indexvector = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
  __m512i columnvector;

  
  // check if this is the last 512 bit word for this list
  uint max_dgaps_packable = 16 * table[selector_row].intsper32;
  if (length < max_dgaps_packable)
  {
    printf("near the end of a list, %d dgaps remaining\n", length);
    // do similar to below, but with more checking for end of list use
    // the zeros column to pack zeros. for now i'm just not packing the
    // last word, but should be returning correct "dgaps compressed"
    // value
    int i;
    for (i = 0; i < table[selector_row].intsper32; i++)
    {
      if (raw + 16 > end)
	{
	  printf("raw plus 16 is beyond the end\n");
	  break;
	}
      // gather next 16 ints into a 512 bit register
      columnvector = _mm512_i32gather_epi32(indexvector, raw, 4);
      
      // left shift input to correct "column"
      columnvector = _mm512_slli_epi32(columnvector, table[selector_row].bitwidth * i);
      
      // pack this column of 16 dgaps into compressed 512 bit word
      compressedword = _mm512_or_epi32(compressedword, columnvector);
      raw += 16;
    }
    
  }

  else
  { // in this case we don't need to check for end of list at all
   
    for (int i = 0; i < table[selector_row].intsper32; i++)
    {

      // gather next 16 ints into a 512 bit register
      columnvector = _mm512_i32gather_epi32(indexvector, raw, 4);
      
      // left shift input to correct "column"
      columnvector = _mm512_slli_epi32(columnvector, table[selector_row].bitwidth * i);
      
      // pack this column of 16 dgaps into compressed 512 bit word
      compressedword = _mm512_or_epi32(compressedword, columnvector);
      raw += 16;
    }

    /*
      write compressed data to memory as 32 bit ints
    */
    _mm512_i32scatter_epi32(dest, indexvector, compressedword, 4); 

  }
  
  /*
    record size of compressed data in class variables, and return number of dgaps compressed
  */
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


int Simple10avx::decode(uint32_t *dest, uint32_t *encoded, uint32_t *end, uint8_t *selectors, int num_s)
{
  int dgaps_decompressed = 0;
  int i = 0;
  
  while (i < num_s)
  {
    dgaps_decompressed += decode_one_word(dest + dgaps_decompressed, encoded + i*16, end, selectors++);
    i++;
  }

  return dgaps_decompressed;
}

int Simple10avx::decode_one_word(uint32_t *dest, uint32_t *encoded, uint32_t *end, uint8_t *selectors)
{
  int dgaps_decompressed = 0;
  printf("encoded length remaining: %d\n", end - encoded);
  
  // get information from selectors array
  int bits_per_dgap = table[selectors[0]].bitwidth;
  int dgaps_per_int = table[selectors[0]].intsper32;
  int mask = pow(2, bits_per_dgap) - 1;
  __m512i mask_vect = _mm512_set1_epi32(mask);

  printf("selector %d, %d bits per int, %d dgaps per int\n", *selectors,
	 table[selectors[0]].bitwidth, table[selectors[0]].intsper32);
  
  /* 
     Load 512 bits of compressed data into a register 
  */
  __m512i indexvector = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
  __m512i compressed_word = _mm512_i32gather_epi32(indexvector, encoded, 4);

  /* 
     Declare a 512 bit vector for decoding into 
  */
  __m512i decomp_vect; 
  

  /* 
     Decompress 512 bits of encoded data
   */
  for (int i = 0; i < dgaps_per_int; i++)
  {
    // get 16 dgaps by ANDing mask with compressed word
    decomp_vect = _mm512_and_epi32(compressed_word, mask_vect);

    // write those 16 numbers to uint32_t array "dest"
    _mm512_i32scatter_epi32(dest, indexvector, decomp_vect, 4);
        
    // right shift the remaining data in the compressed word
    compressed_word = _mm512_srli_epi32(compressed_word, bits_per_dgap);
    
    dgaps_decompressed += 16;
    for (int j = 0; j < 16; j++)
      printf("%2d, ", dest[j]);
    printf("\n");
    dest += 16;
  }

  
  /*
    In the last 512 bits of compressed data, it is necessary to find
    (by searching for zeros) where the end of the postings list is,
    because we only know the compressed size - not the list length -
    when beginning decompression
   */
  if (end - encoded == 16) 
  {
    /* 
       there will be between zero and 16 * intersper32 zeros in the
       dest array now. there may be a real zero in the first position
       in very rare cases, but there won't every be an end-marking
       zero in the first position. So start checking at dest+1
    */
    int backtrack = 16 * table[selectors[0]].intsper32;
    //printf("backtrack distance: %d\n", backtrack);
    dest -= backtrack;
    //printf("last element in list: %d\n", *(--dest));
    for (int i = 1; i < table[selectors[0]].intsper32 * 16; i++)
      if (dest[i] == 0)
	{
	  return i;
	  dgaps_decompressed = i;
	  break;
	}
    

            
  }


  // check for any dgap other than the first one being a zero? that won't always work?
  // if remaining length less than 64 check for any zeros?
  // how can i deal with the possibility of a zero at the start of a list?
  // i guess easiest is checking for two zeros in a row?
  
  
  return dgaps_decompressed;
}



