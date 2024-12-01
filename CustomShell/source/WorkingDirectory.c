#include "WorkingDirectory.h"

void WorkingDirectory(){
    char curdir[1024];
    if (getcwd(curdir, sizeof(curdir)) != NULL) { 
        printf("현재 작업 위치: %s\n", curdir); 
    } 
    else { 
        perror("pwd 명령어 실패");
    }
    exit(1);
}
