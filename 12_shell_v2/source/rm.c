#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "rm.h"

void execute_rm(const char *file) {
    if (file == NULL) {
        printf("Usage: rm [filename]\n");
        return;
    }

    if (remove(file) == 0) {
        printf("Deleted file: %s\n", file);
    } else {
        perror("Error deleting file");
    }
    exit(1);
}
