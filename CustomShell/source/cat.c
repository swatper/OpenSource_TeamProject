#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

void execute_cat(char **argv) {
    char buffer[4096];
    ssize_t bytes_read;

    // cat 파일 읽기 처리
    int i = 1; // 첫 번째 인자는 명령어 자체이므로 건너뜀
    if (argv[1] == NULL) {
        // 파일 인자가 없을 경우 표준 입력 읽기
        while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
            if (write(STDOUT_FILENO, buffer, bytes_read) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
    } else {
        // 파일 인자가 있는 경우 파일들 처리
        while (argv[i] != NULL) {
            int fd = open(argv[i], O_RDONLY);
            if (fd == -1) {
                fprintf(stderr, "cat: %s: %s\n", argv[i], strerror(errno));
                i++;
                continue;
            }

            // 파일 내용 출력
            while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
                if (write(STDOUT_FILENO, buffer, bytes_read) == -1) {
                    perror("write");
                    close(fd);
                    exit(EXIT_FAILURE);
                }
            }

            if (bytes_read == -1) {
                perror("read");
                close(fd);
                exit(EXIT_FAILURE);
            }

            close(fd);
            i++;
        }
    }

    exit(EXIT_SUCCESS);
}
