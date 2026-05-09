#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char* name;
} Column;

Column* columns = NULL;
int col_count = 0;

char* safe_copy(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* new_s = malloc(len + 1);
    if (new_s) strcpy(new_s, s);
    return new_s;
}

char* trim_copy(const char* s) {
    if (!s) return NULL;
    size_t start = strspn(s, " \t\n\r\f\v");
    if (s[start] == '\0') return safe_copy("");
    size_t end = strlen(s) - 1;
    while (end > start && isspace((unsigned char)s[end])) end--;
    size_t len = end - start + 1;
    char* result = malloc(len + 1);
    if (result) {
        memcpy(result, s + start, len);
        result[len] = '\0';
    }
    return result;
}

char** split_csv(const char* line, int* n_fields) {
    if (!line) return NULL;
    int capacity = 16;
    char** fields = malloc(capacity * sizeof(char*));
    if (!fields) return NULL;
    int count = 0;
    const char* start = line;
    const char* p = line;
    while (1) {
        if (*p == ',' || *p == '\0') {
            size_t len = p - start;
            char* field = malloc(len + 1);
            if (field) {
                memcpy(field, start, len);
                field[len] = '\0';
                if (count >= capacity) {
                    capacity *= 2;
                    char** new_fields = realloc(fields, capacity * sizeof(char*));
                    if (!new_fields) {
                        free(field);
                        for (int i = 0; i < count; i++) free(fields[i]);
                        free(fields);
                        return NULL;
                    }
                    fields = new_fields;
                }
                fields[count++] = field;
            }
            else {
                for (int i = 0; i < count; i++) free(fields[i]);
                free(fields);
                return NULL;
            }
            if (*p == '\0') break;
            start = p + 1;
        }
        p++;
    }
    *n_fields = count;
    return fields;
}

void free_strings(char** arr, int n) {
    if (!arr) return;
    for (int i = 0; i < n; i++) free(arr[i]);
    free(arr);
}

int read_header_only(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Error: cannot open file '%s'.\n", filename);
        return 0;
    }

    char line[8192];
    if (!fgets(line, sizeof(line), f)) {
        printf("Error: file '%s' is empty.\n", filename);
        fclose(f);
        return 0;
    }
    line[strcspn(line, "\r\n")] = '\0';
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
    for (int i = 0; i < col_count; i++) {
        char* trimmed = trim_copy(header_fields[start_idx + i]);
        columns[i].name = trimmed ? trimmed : safe_copy("");
    }
    free_strings(header_fields, n_fields);
    fclose(f);
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <csv_file>\n", argv[0]);
        return 1;
    }
    if (read_header_only(argv[1])) {
        printf("Header read successfully:\n");
        for (int i = 0; i < col_count; i++)
            printf(" %s\n", columns[i].name);
        for (int i = 0; i < col_count; i++) free(columns[i].name);
        free(columns);
    }
    else {
        printf("Failed to read header.\n");
    }
    return 0;
}