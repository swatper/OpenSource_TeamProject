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
        printf("파일 삭제 성공: %s\n", file);
    } else {
        perror("파일 삭제 실패");
    }
    exit(1);
}
