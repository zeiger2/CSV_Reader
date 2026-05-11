#ifndef COMPUTE_H
#define COMPUTE_H

#include "table.h"

double get_cell_value_by_names(const char* col_name, int row_num);
double arg_to_number(const char* s);
double compute_cell_value(int col_idx, int row_idx);
void compute_all(void);

#endif