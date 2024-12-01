/*      작성자: 박상현      */
#include "FileList.h"

void FileList(char *dir){
    //"ls" 입력 시
    if (dir == NULL){
        execl("/bin/ls", "ls", "-l", (char *) 0);
    }
    //"ls 디렉토리" 입력 시
    else{
         execl("/bin/ls", "ls", "-l", dir, (char *) 0);
    }
    perror("ls 명령어 실패");
    exit(1);
}
