#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// rmdir 명령어 실행 함수 정의
void execute_rmdir(char **argv) {
    if (argv[1] == NULL) {
        printf("rmdir: 삭제할 디렉토리를 지정해야 합니다.\n");
        return;
    }

    if (rmdir(argv[1]) == -1) { // rmdir 실패 시 처리
        perror("rmdir 실패");
        return;
    }

    printf("rmdir: 디렉토리 '%s' 삭제 완료\n", argv[1]);
    exit(1);
}
