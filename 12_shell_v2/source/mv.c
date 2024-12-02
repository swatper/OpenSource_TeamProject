#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mv.h"

void execute_mv(const char *source, const char *destination) {
    if (source == NULL || destination == NULL) {
        printf("Usage: mv [source] [destination]\n");
        return;
    }

    if (rename(source, destination) == 0) {
        printf("Moved file: %s -> %s\n", source, destination);
    } else {
        perror("Error moving file");
    }
    exit(1);
}
