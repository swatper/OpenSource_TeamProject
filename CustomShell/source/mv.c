#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mv.h"

void execute_mv(const char *source, const char *destination) {
    if (source == NULL || destination == NULL) {
        printf("명령어 인자 부족\n");
        return;
    }

    struct stat statbuf;
    //목적지가 디랙토리일 경우
    if (stat(destination, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
        char new_path[4096];
        sprintf(new_path, "%s/%s", destination, strrchr(source, '/') ? strrchr(source, '/') + 1 : source);
        destination = new_path;
    }

    if (rename(source, destination) == 0) {
        printf("mv 명령어 성공: %s -> %s\n", source, destination);
        exit(1);
    } 

    exit(1);
}
