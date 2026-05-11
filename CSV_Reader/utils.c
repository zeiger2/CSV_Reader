#define _CRT_SECURE_NO_WARNINGS
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//  опирование строки с выделением пам€ти
char* safe_copy(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* new_s = malloc(len + 1);
    if (new_s) strcpy(new_s, s);
    return new_s;
}

// ”даление пробелов в начале и конце строки
char* trim_copy(const char* s) {
    if (!s) return NULL;
    // ѕропускаем начальные пробелы
    size_t start = strspn(s, " \t\n\r\f\v");
    if (s[start] == '\0') return safe_copy(""); // строка из одних пробелов
    // ѕропускаем конечные пробелы
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

// –аздел€ет строку на пол€
char** split_csv(const char* line, int* n_fields) {
    if (!line) return NULL;
    int capacity = 16;
    char** fields = malloc(capacity * sizeof(char*));
    if (!fields) return NULL;
    int count = 0;
    const char* start = line;
    const char* p = line;
    // читаем пол€ от зап€той до зап€той
    while (1) {
        if (*p == ',' || *p == '\0') {
            size_t len = p - start;
            char* field = malloc(len + 1);
            if (field) {
                memcpy(field, start, len);
                field[len] = '\0';
                // сохран€ем поле с учетом возможного переполнени€
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

// ”дал€ет из пам€ти вложенные массивы
void free_strings(char** arr, int n) {
    if (!arr) return;
    for (int i = 0; i < n; i++) free(arr[i]);
    free(arr);
}
