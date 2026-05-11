#include "table.h"
#include "comput.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Not enough arguments. Use %s with the CSV file name\n", argv[0]);
        return 1;
    }
    if (!read_csv(argv[1])) {
        fprintf(stderr, "Failed to read CSV file.\n");
        return 1;
    }
    compute_all();
    print_csv();
    free_all();
    return 0;
}