#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include "ex_rmdir.h" //rmdir 함수 헤더
#include "ex_ln.h"
#include "ex_cp.h"

#define MAX_BUF 256
#define MAX_ARG 50

// ----------------------------------------------------
// SIGINT 핸들러 (Ctrl-C)
// ----------------------------------------------------
void handle_sigint(int sig) {
    printf("\nInterrupt signal (SIGINT) received. Use 'exit' to quit.\n");
}

// ----------------------------------------------------
// SIGQUIT 핸들러 (Ctrl-Z)
// ----------------------------------------------------
void handle_sigquit(int sig) {
    printf("\nQuit signal (SIGQUIT) received. Use 'exit' to quit.\n");
}

// ----------------------------------------------------
// 사용자 입력을 공백으로 분리하여 인자 배열에 저장
// ----------------------------------------------------
int getargs(char *cmd, char **argv) {
    int narg = 0;
    while (*cmd) {
        if (*cmd == ' ' || *cmd == '\t')
            *cmd++ = '\0';
        else {
            argv[narg++] = cmd++;
            while (*cmd != '\0' && *cmd != ' ' && *cmd != '\t')
                cmd++;
        }
    }
    argv[narg] = NULL;
    return narg;
}


// ----------------------------------------------------
// 명령어 실행 함수 (파일 재지향 및 파이프 처리 포함)
// ----------------------------------------------------
void execute_command(char **argv, int input_fd, int output_fd) {
    if (strcmp(argv[0], "rmdir") == 0) {// rmdir 함수 호출
        execute_rmdir(argv);
        exit(0);
    }
    if (strcmp(argv[0], "ln") == 0) {// ln 함수 호출
        execute_ln(argv);             
        exit(0);
    }
    if (strcmp(argv[0], "cp") == 0) { //cp 함수 호출
        execute_cp(argv);             
        exit(0);
    }


    if (input_fd != 0) {
        dup2(input_fd, STDIN_FILENO); // 표준 입력 변경
        close(input_fd);
    }
    if (output_fd != 1) {
        dup2(output_fd, STDOUT_FILENO); // 표준 출력 변경
        close(output_fd);
    }
    execvp(argv[0], argv); // 명령 실행
    perror("명령어 실행 실패");
    exit(1);
}

// ----------------------------------------------------
// 명령어 파이프 및 파일 재지향 처리
// ----------------------------------------------------
void process_command(char *cmd) {
    char *commands[MAX_ARG];
    int n_commands = 0;
    int pipes[MAX_ARG][2];
    int input_fd = 0, output_fd = 1;

    // 파이프 기준으로 명령어 분리
    char *token = strtok(cmd, "|");
    while (token != NULL) {
        commands[n_commands++] = token;
        token = strtok(NULL, "|");
    }

    for (int i = 0; i < n_commands; i++) {
        // 현재 명령어에 대한 파일 재지향 확인
        char *argv[MAX_ARG];
        char *redir_file;
        input_fd = (i == 0) ? 0 : pipes[i - 1][0]; // 이전 파이프의 읽기 끝
        output_fd = 1;

        // 파이프 생성 (다음 명령어가 있다면)
        if (i < n_commands - 1) {
            pipe(pipes[i]);
            output_fd = pipes[i][1]; // 쓰기 끝
        }

        // 파일 재지향 처리
        char *cmd_with_redir = strtok(commands[i], ">");
        redir_file = strtok(NULL, ">");
        if (redir_file != NULL) {
            redir_file = strtok(redir_file, " ");
            output_fd = open(redir_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd == -1) {
                perror("파일 열기 실패");
                return;
            }
        }

        cmd_with_redir = strtok(cmd_with_redir, "<");
        char *input_file = strtok(NULL, "<");
        if (input_file != NULL) {
            input_file = strtok(input_file, " ");
            input_fd = open(input_file, O_RDONLY);
            if (input_fd == -1) {
                perror("파일 열기 실패");
                return;
            }
        }

        // 명령어 인자 분리
        getargs(cmd_with_redir, argv);

        pid_t pid = fork();
        if (pid == 0) {
            // 자식 프로세스
            if (i < n_commands - 1) {
                close(pipes[i][0]); // 읽기 닫기
            }
            execute_command(argv, input_fd, output_fd);
        } else if (pid > 0) {
            // 부모 프로세스
            if (input_fd != 0) close(input_fd); // 이전 입력 닫기
            if (output_fd != 1) close(output_fd); // 이전 출력 닫기
            if (i < n_commands - 1) close(pipes[i][1]); // 쓰기 닫기
        } else {
            perror("포크 실패");
            return;
        }
    }

    // 모든 자식 프로세스 종료 대기
    for (int i = 0; i < n_commands; i++) {
        wait(NULL);
    }
}

// ----------------------------------------------------
// 메인 함수
// ----------------------------------------------------
int main() {
    char buf[MAX_BUF];
    int is_background;

    // SIGINT 및 SIGQUIT 핸들러 설정
    signal(SIGINT, handle_sigint);
    signal(SIGQUIT, handle_sigquit);

    while (1) {
        printf("shell> "); // 쉘 프롬프트 출력
        if (fgets(buf, MAX_BUF, stdin) == NULL) { // 사용자 입력 받기
            printf("\n");
            break; // 루프 종료
        }

        buf[strcspn(buf, "\n")] = '\0'; // '\n' 제거

        // "exit" 명령 처리
        if (strcmp(buf, "exit") == 0) {
            printf("쉘 종료.\n");
            break;
        }

        // 백그라운드 실행 여부 확인
        is_background = 0;
        if (buf[strlen(buf) - 1] == '&') {
            is_background = 1;
            buf[strlen(buf) - 1] = '\0'; // '&' 제거
        }

        // 명령어 처리
        process_command(buf);
    }

    return 0;
}
