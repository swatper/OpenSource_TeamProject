#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

void execute_cp(char **argv) {
    if (argv[1] == NULL || argv[2] == NULL) {
        printf("cp: 원본 파일과 대상 파일을 지정해야 합니다.\n");
        return;
    }

    int src_fd = open(argv[1], O_RDONLY); // 원본 파일 열기
    if (src_fd == -1) {
        perror("cp 실패: 원본 파일 열기");
        return;
    }

    int dest_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644); // 대상 파일 열기
    if (dest_fd == -1) {
        perror("cp 실패: 대상 파일 열기");
        close(src_fd);
        return;
    }

    char buffer[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        if (write(dest_fd, buffer, bytes_read) == -1) {
            perror("cp 실패: 쓰기 오류");
            close(src_fd);
            close(dest_fd);
            return;
        }
    }

    if (bytes_read == -1) {
        perror("cp 실패: 읽기 오류");
    }

    close(src_fd);
    close(dest_fd);
    printf("cp: '%s' -> '%s' 복사 완료\n", argv[1], argv[2]);
    exit(1);
}
