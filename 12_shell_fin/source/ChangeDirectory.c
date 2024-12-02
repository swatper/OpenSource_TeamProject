#include "ChangeDirectory.h"

void ChangeDirectory(char *dir){
    if (dir == NULL) { // 인자가 없으면 홈 디렉토리로 이동 
        dir = getenv("HOME"); 
    }
    if (chdir(dir) == 0) {
        char curdir[1024];
        if (getcwd(curdir, sizeof(curdir)) != NULL) { 
            printf("이동 후 작업 위치: %s\n", curdir); 
        } 
    } 
    else{ 
        perror("ch 명령어 실패"); 
    }
    exit(1);
}
