#define _CRT_SECURE_NO_WARNINGS
#include "comput.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

static int cycle_flag = 0;

// Запрос на получения значения в ячейках
double get_cell_value_by_names(const char* col_name, int row_num) {
    int col_idx = -1;
    for (int i = 0; i < col_count; i++)
        if (strcmp(columns[i].name, col_name) == 0) { col_idx = i; break; }
    if (col_idx == -1) return 0.0;
    int row_idx = -1;
    for (int i = 0; i < row_count; i++)
        if (rows[i].row_num == row_num) { row_idx = i; break; }
    if (row_idx == -1) return 0.0;
    return compute_cell_value(col_idx, row_idx);
}

// Выделяем аргументы из уравнений
double arg_to_number(const char* s) {
    if (!s || *s == '\0') return 0.0;
    if (isalpha((unsigned char)s[0])) {
        char col_name[64];
        int row_num = 0;
        int i = 0;
        while (s[i] && !isdigit((unsigned char)s[i])) {
            col_name[i] = s[i];
            i++;
            if (i >= 63) break;
        }
        col_name[i] = '\0';
        row_num = atoi(s + i);
        return get_cell_value_by_names(col_name, row_num);
    }
    else {
        return atof(s);
    }
}

// Парсит выражение
double compute_cell_value(int col_idx, int row_idx) {
    Cell* cell = &rows[row_idx].cells[col_idx];

    if (cell->computed) {
        return cell->value;
    }
    if (cell->computing) {
        cycle_flag = 1;
        return 0.0;
    }
    if (!cell->is_formula) {
        cell->computed = 1;
        return cell->value;
    }
    cell->computing = 1;
    char* expr = cell->expr;
    if (!expr || strlen(expr) == 0) {
        cell->computed = 1;
        cell->value = 0.0;
        cell->computing = 0;
        return 0.0;
    }
    char* buf = safe_copy(expr);
    if (!buf) return 0.0;
    char* trimmed = trim_copy(buf);
    free(buf);
    if (!trimmed) {
        cell->computed = 1;
        cell->value = 0.0;
        cell->computing = 0;
        return 0.0;
    }
    // поиск оператора
    int op_pos = -1;
    char op = 0;
    for (int i = 0; trimmed[i]; i++) {
        if (trimmed[i] == '+' || trimmed[i] == '-' || trimmed[i] == '*' || trimmed[i] == '/') {
            op_pos = i;
            op = trimmed[i];
            break;
        }
    }
    if (op_pos == -1) {
        free(trimmed);
        cell->computed = 1;
        cell->value = 0.0;
        cell->computing = 0;
        return 0.0;
    }
    char* left_str = trimmed;
    char* right_str = trimmed + op_pos + 1;
    trimmed[op_pos] = '\0';
    char* left_trim = trim_copy(left_str);
    char* right_trim = trim_copy(right_str);
    double arg1 = arg_to_number(left_trim);
    double arg2 = arg_to_number(right_trim);
    free(left_trim);
    free(right_trim);
    double result = 0.0;
    if (!cycle_flag) {
        switch (op) {
        case '+': result = arg1 + arg2; break;
        case '-': result = arg1 - arg2; break;
        case '*': result = arg1 * arg2; break;
        case '/': result = (arg2 != 0.0) ? arg1 / arg2 : 0.0; break;
        }
    }
    else {
        result = 0.0;
    }
    free(trimmed);
    cell->value = result;
    cell->computed = 1;
    cell->computing = 0;
    free(cell->expr);
    cell->expr = NULL;
    return result;
}

// Проход по всем значениям ячеек
void compute_all() {
    for (int r = 0; r < row_count; r++)
        for (int c = 0; c < col_count; c++)
            compute_cell_value(c, r);
}