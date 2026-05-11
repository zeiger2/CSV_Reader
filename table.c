#define _CRT_SECURE_NO_WARNINGS
#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

Column* columns = NULL;
int col_count = 0;
Row* rows = NULL;
int row_count = 0;
int row_capacity = 0;

// Чтение файла построчно
int read_csv(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Error: cannot open file '%s'.\n", filename);
        return 0;
    }

    char line[8192];
    // читаем заголовок
    if (!fgets(line, sizeof(line), f)) {
        printf("Error: file '%s' is empty.\n", filename);
        fclose(f);
        return 0;
    }
    // удаляем символы перевода строки
    line[strcspn(line, "\r\n")] = '\0';

    // пропускаем возможные пустые строки
    while (strlen(line) == 0 && fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
    }
    if (strlen(line) == 0) {
        printf("Error: no header found in '%s'.\n", filename);
        fclose(f);
        return 0;
    }

    int n_fields = 0;
    char** header_fields = split_csv(line, &n_fields);

    if (!header_fields || n_fields == 0) {
        printf("Error: cannot parse header.\n");
        if (header_fields) free_strings(header_fields, n_fields);
        fclose(f);
        return 0;
    }
    /*printf("LenHead: %zu\n", strlen(header_fields));
    printf("%d", n_fields);
    for (int i = 0; i < n_fields; i++) {
        printf("%s", header_fields[i]);
        printf("\n");
    }*/

    // первое поле должно быть пустым
    int start_idx = 0;
    if (n_fields > 0 && header_fields[0][0] == '\0') start_idx = 1;
    col_count = n_fields - start_idx;
    if (col_count <= 0) {
        printf("Error: no column names found in header.\n");
        free_strings(header_fields, n_fields);
        fclose(f);
        return 0;
    }
    columns = malloc(col_count * sizeof(Column));
    if (!columns) {
        printf("Memory allocation failed for columns.\n");
        free_strings(header_fields, n_fields);
        fclose(f);
        return 0;
    }
    // проверка на дубликат и изменение имени в случае дубликата
    for (int i = 0; i < col_count; i++) {
        char* original = trim_copy(header_fields[start_idx + i]);
        if (!original) original = safe_copy("");

        char* new_name = safe_copy(original);
        int suffix = 1;
        int conflict = 1;

        while (conflict) {
            conflict = 0;
            for (int j = 0; j < i; j++) {
                if (strcmp(columns[j].name, new_name) == 0) {
                    conflict = 1;
                    break;
                }
            }
            if (conflict) {
                free(new_name);
                new_name = malloc(strlen(original) + 12);
                if (!new_name) {
                    fprintf(stderr, "Memory allocation failed\n");
                    free(original);
                }
                sprintf(new_name, "%s_%d", original, suffix);
                suffix++;
            }
        }
        columns[i].name = new_name;
        free(original);
    }
    free_strings(header_fields, n_fields);
    // закончили читать заголовок

    // читаем строки
    row_capacity = 10;
    rows = malloc(row_capacity * sizeof(Row));
    if (!rows) {
        printf("Memory allocation failed for rows.\n");
        fclose(f);
        return 0;
    }
    row_count = 0;
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;
        char** fields = split_csv(line, &n_fields);
        if (!fields) {
            printf("Warning: failed to parse line, skipping.\n");
            continue;
        }
        if (n_fields == 0) {
            free_strings(fields, n_fields);
            continue;
        }
        int row_num = atoi(fields[0]);
        if (row_num <= 0) {
            free_strings(fields, n_fields);
            continue;
        }
        // проверка на дубликат номера строки
        int duplicate = 0;
        for (int i = 0; i < row_count; i++) {
            if (rows[i].row_num == row_num) {
                //printf("Duplicate row found! Skipping.\n");
                duplicate = 1;
                break;
            }
        }
        if (duplicate) {
            free_strings(fields, n_fields);
            continue;
        }

        // расширяем массив строк если надо
        if (row_count >= row_capacity) {
            row_capacity *= 2;
            Row* new_rows = (Row*)realloc(rows, row_capacity * sizeof(Row));
            if (!new_rows) {
                free_strings(fields, n_fields);
                fclose(f);
                return 0;
            }
            rows = new_rows;
        }
        Row* r = &rows[row_count];
        r->row_num = row_num;
        r->cells = malloc(col_count * sizeof(Cell));
        if (!r->cells) {
            free_strings(fields, n_fields);
            fclose(f);
            return 0;
        }
        // запоняем структуру значениями
        for (int c = 0; c < col_count; c++) {
            int idx = c + 1;
            const char* val = (idx < n_fields) ? fields[idx] : "";
            char* trimmed = trim_copy(val);
            if (!trimmed) trimmed = safe_copy("");
            r->cells[c].expr = NULL;
            r->cells[c].computed = 0;
            r->cells[c].computing = 0;
            if (trimmed[0] == '=') {
                r->cells[c].is_formula = 1;
                r->cells[c].expr = safe_copy(trimmed + 1);
                r->cells[c].value = 0.0;
            }
            else if (trimmed[0] == '\0') {
                r->cells[c].is_formula = 0;
                r->cells[c].value = 0.0;
                r->cells[c].computed = 1;
            }
            else {
                r->cells[c].is_formula = 0;
                r->cells[c].value = atof(trimmed);
                r->cells[c].computed = 1;
            }
            free(trimmed);
        }
        free_strings(fields, n_fields);
        row_count++;
    }
    fclose(f);
    if (row_count == 0) {
        printf("Error: no data rows found in '%s'.\n", filename);
        return 0;
    }
    return 1;
}

// Вывод всех элементов
void print_csv() {
    printf(",");
    for (int i = 0; i < col_count; i++) {
        printf("%s", columns[i].name);
        if (i < col_count - 1) printf(",");
    }
    printf("\n");
    for (int r = 0; r < row_count; r++) {
        printf("%d", rows[r].row_num);
        for (int c = 0; c < col_count; c++) {
            double val = rows[r].cells[c].value;
            if (fabs(val - (int)val) < 1e-9)
                printf(",%d", (int)val);
            else
                printf(",%g", val);
        }
        printf("\n");
    }
}
// освобождение всей памяти
void free_all() {
    if (columns) {
        for (int i = 0; i < col_count; i++) free(columns[i].name);
        free(columns);
        columns = NULL;
    }
    if (rows) {
        for (int i = 0; i < row_count; i++) {
            if (rows[i].cells) {
                for (int c = 0; c < col_count; c++)
                    if (rows[i].cells[c].expr) free(rows[i].cells[c].expr);
                free(rows[i].cells);
            }
        }
        free(rows);
        rows = NULL;
    }
}