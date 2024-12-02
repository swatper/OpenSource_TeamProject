#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
void execute_ln(char **argv) {
    if (argv[1] == NULL || argv[2] == NULL) {
        printf("ln: 링크를 생성할 원본 파일과 대상 파일을 지정해야 합니다.\n");
        return;
    }

    int symbolic = 0;
    if (strcmp(argv[1], "-s") == 0) { // 심볼릭 링크 옵션 확인
        symbolic = 1;
        argv++; // 옵션을 건너뛰어 원본과 대상 처리
    }

    int result;
    if (symbolic) {
        // 심볼릭 링크 생성
        result = symlink(argv[1], argv[2]);
    } else {
        // 하드 링크 생성
        result = link(argv[1], argv[2]);
    }

    if (result == -1) {
        perror("ln 실패");
        return;
    }

    printf("ln: 링크 '%s' -> '%s' 생성 완료\n", argv[2], argv[1]);
    exit(1);
}
