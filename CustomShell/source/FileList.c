/*      작성자: 박상현      */
#include "FileList.h"

void FileList(char *path){
    DIR *dir;
    struct dirent *entry;

    //디렉토리 열기
    if ((dir = opendir(path)) == NULL) {
        perror("디렉토리를 열 수 없습니다");
        exit(EXIT_FAILURE);
    }

    //디렉토리 내 파일 내용 읽기
    printf("파일 목록\n");
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; 
        printf("%s ", entry->d_name); 
    }
    printf("\n");
    closedir(dir);
    exit(1);
}
