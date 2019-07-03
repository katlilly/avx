#pragma once
#include <cstdint>

class Simple10avx
{
 public:
  struct row {
    int intsper32;
    int bitwidth;
  };
  
  int num_selectors;
  row *table;
  int registerbits = 512;

 private:
  int current_list_length;
  int remaining_length;
  int current_selector;
  
  
 public:
  Simple10avx()
    {
      num_selectors = 10;
      table = new row[num_selectors];
      
      table[0].intsper32 = 1;
      table[0].bitwidth = 32;
      
      table[1].intsper32 = 2;
      table[1].bitwidth = 16;

      table[2].intsper32 = 3;
      table[2].bitwidth = 10;

      table[3].intsper32 = 4;
      table[3].bitwidth = 8;

      table[4].intsper32 = 5;
      table[4].bitwidth = 6;

      table[5].intsper32 = 6;
      table[5].bitwidth = 5;

      table[6].intsper32 = 8;
      table[6].bitwidth = 4;

      table[7].intsper32 = 10;
      table[7].bitwidth = 3;

      table[8].intsper32 = 16;
      table[8].bitwidth = 2;

      table[9].intsper32 = 32;
      table[9].bitwidth = 1;
    }

  ~Simple10avx()
    {
      delete table;
    }

 public:
  void print_table();
  void dgaps_to_bitwidths(int *dest, int *source, int length);
  int encode(uint32_t *dest, uint8_t *selector, int *source,
	     int length);

 private:
  int chose_selector(int largest_column_width);
  int encode_one_word(uint32_t *dest, uint8_t *dest_selector,
		      int selector, int *source, int length);
  
};
