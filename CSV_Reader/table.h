#ifndef TABLE_H
#define TABLE_H

#include "utils.h"

typedef struct {
    char* name;
} Column;

typedef struct {
    int is_formula;
    char* expr;
    double value;
    int computed;
    int computing;
} Cell;

typedef struct {
    int row_num;
    Cell* cells;
} Row;

extern Column* columns;
extern int col_count;
extern Row* rows;
extern int row_count;
extern int row_capacity;

int read_csv(const char* filename);
void print_csv(void);
void free_all(void);

#endif