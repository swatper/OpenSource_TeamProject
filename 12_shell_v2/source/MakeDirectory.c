#include "MakeDirectory.h"

void MakeDirectory(char *dir){
    if (mkdir(dir, 0755) == 0) { // 디렉토리 생성, 권한 설정 (rwxr-xr-x) 
        printf("디렉토리 '%s' 생성 완료\n", dir); 
    } 
    else { 
        perror("디렉토리 생성 실패");
    }
    exit(1);
}