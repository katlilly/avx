#pragma once

class Simple10avx
{
 public:
  int num_selectors;
  
 public:
  Simple10avx()
    {
      num_selectors = 10;
    }

  ~Simple10avx()
    {
      
    }

 public:
  int make_table(int **table);
  void print_table(int **table);
  void dgaps_to_bitwidths(int *dest, int *source, int length);
  int encode(int *dest, int *source, int length);
  int chose_selector(int **table, int largest_column_width);

};
