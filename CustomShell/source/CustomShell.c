#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

#pragma region CommandFunc
#include "FileList.h"
#include "WorkingDirectory.h"
#include "ChangeDirectory.h"
#include "MakeDirectory.h"
#include "ex_rmdir.h"
#include "ex_ln.h"
#include "ex_cp.h"
#pragma endregion

void Background(char **argv);
void RedirectInputOutput(char **argv, int *in_fd, int *out_fd);
void SignalHandler(int signo);
int getargs(char *cmd, char **argv);
int RunCommand(int narg, char **argv);

pid_t chidPID;
bool isQuit = false;

int main(){
    char buf[256];      //사용자 입력 받는 배열
    char *argv[50];     //명령어 인자 저장 배열
    int narg;           //명령어 인자 개수
    pid_t pid;          //fork용 PID
    bool isBackground;  //백그라운드 여부
    int i;              //명령어 인자 위치 확인용
    int fd;             //파일 디스크립션 변수
    int result;         //구현한 명령어 사용 여부

    //시그널 설정
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_handler = SignalHandler;
    sigaction(SIGINT, &sa, NULL);  // Ctrl+C 인터럽트
    sigaction(SIGQUIT, &sa, NULL); // Ctrl+Z 인터럽트

    //입력 받기
    printf("**********커스텀 쉘 실행**********\n");
    while (1) {
        printf("CustomShell> ");
        gets(buf);
        clearerr(stdin);
        narg = getargs(buf, argv); 

        //exit 명령어
        if (narg > 0 && strcmp(argv[0], "exit") == 0) {
            printf("**********커스텀 쉘 종료**********\n");
            exit(1);
        }

        //명령어 끝에 '&'가 있으면 백그라운드 실행 
        if (narg > 0 && strcmp(argv[narg - 1], "&") == 0) { 
            printf("명령어 수: %d \n",narg);
            argv[narg - 1] = NULL; // '&' 제거 
            isBackground = true; 
        } 
        else { 
            isBackground = false;
        }

        pid = fork();
        //--------------------------자식 프로세스--------------------------
        if (pid == 0){
            //pwd 명령어
            if (narg > 0 && strcmp(argv[0], "pwd") == 0) {
                WorkingDirectory();
                exit(1);
            }

            //라다이렉션
            for(i = 0; i < narg; i++){
                if(strcmp(argv[i], ">") == 0){
                    if((fd = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1){
					    perror("파일 열기 실패");
					    exit(1);
				    }
				    if((dup2(fd, STDOUT_FILENO) == -1)){
					    perror("dup2 실패");
					    exit(1);
				    }
                    close(fd);
                    argv[i] = NULL; // > 제거
                    break;
                }
                else if(strcmp(argv[i], "<") == 0){
                    if((fd = open(argv[i + 1], O_RDONLY, 0644)) == -1){
					    perror("파일 열기 실패");
					    exit(1);
				    }
				    if((dup2(fd, STDIN_FILENO) == -1)){
					    perror("dup2");
					    exit(1);
				    }
                    //디스크립터 닫기
                    close(fd);
                    argv[i] = NULL; // < 제거
                    break;
                }
            }
            //기타 다른 명령어 실행
            if((result = RunCommand(narg,argv)) == 1){
                execvp(argv[0], argv);
                perror("자식 명령어 실행 실패");
                exit(1);
            }
        }
        //--------------------------부모 프로세스--------------------------
        else if (pid > 0){
            if(!isBackground){
                //자식 프로세스를 기다림
                wait((int *) 0);
            }
            else{
                printf("pid : %d\n", pid);
            }
        }
        else perror("fork 실패");
    }
}

//입력 받은 명령어 파싱
int getargs(char *cmd, char **argv){
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

/* 프로세스 데몬화
void Background(char **argv){
    fflush(stdout);
    //실행 결과를 log 파일에 저장
    int logFile = open("log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);

    if (dup2(logFile, STDOUT_FILENO) == -1) { 
        perror("stdout 리디렉션 실패"); 
        exit(1);
    }

    if (dup2(logFile, STDERR_FILENO) == -1) { 
        perror("stderr 리디렉션 실패"); 
        exit(1);
    }
    close(logFile);

    //daemon 함수를 이용하여 데몬 프로세스 생성
      if (daemon(0, 0) == -1) { // daemon 실패 처리
        perror("데몬 프로세스 전환 실패");
        exit(1);
    }
    execvp(argv[0], argv);
    perror("백그라운드 실패");
    exit(1);
}
*/

void SignalHandler(int signo) {
    if (signo == SIGINT) {
        printf("작업 종료\n");
        kill(chidPID, SIGKILL);
    } else if (signo == SIGQUIT) {
        if(isQuit){
            printf("작업 정지\n");
            kill(chidPID, SIGCONT);
            isQuit = false;
        }
        else{
            printf("작업 재개\n");
            kill(chidPID, SIGSTOP);
            isQuit = true;
        }
    }
}


//구현한 명령어 작동 시 0번환, 기타 다른 명령어면 1을 반환
int RunCommand(int narg, char **argv){
    //ls 명령어
    if (narg > 0 && strcmp(argv[0], "ls") == 0) {
        if(narg == 1){
            FileList(NULL);
        }
        else{
            FileList(argv[1]);
        }
        return 0;            
    }
    //cd 명령어
    if (narg > 0 && strcmp(argv[0], "cd") == 0) {
        if(narg == 1){
                ChangeDirectory(NULL);
        }
        else {
            ChangeDirectory(argv[1]);
        }
        return 0;            
    }
    //mkdir 명령어
    if (narg > 0 && strcmp(argv[0], "mkdir") == 0) {
        if(narg == 1){
            MakeDirectory(NULL);
        }
        else {
            MakeDirectory(argv[1]);
        }
        return 0;            
    }
    if (strcmp(argv[0], "rmdir") == 0) {// rmdir 함수 호출
        execute_rmdir(argv);
        return 0 ;
        exit(0);
    }
    if (strcmp(argv[0], "ln") == 0) {// ln 함수 호출
        execute_ln(argv);
        return 0 ;
    }
    if (strcmp(argv[0], "cp") == 0) { //cp 함수 호출
        execute_cp(argv);
        return 0 ;
    }
    //기타 명령어 실행 시 반환
    return 1;
}