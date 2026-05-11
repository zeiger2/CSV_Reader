#ifndef UTILS_H
#define UTILS_H

char* safe_copy(const char* s);
char* trim_copy(const char* s);
char** split_csv(const char* line, int* n_fields);
void free_strings(char** arr, int n);

#endif