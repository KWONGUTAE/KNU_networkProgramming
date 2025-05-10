// 2021115744 권구태

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/select.h>

#define BUF_SIZE 100

#define CMD_DATA 1
#define CMD_NOTIFY 2
#define CMD_TERMINATE 3

typedef struct {
    int cmd;               // 명령 종류
    char payload[BUF_SIZE]; // 시간 또는 메시지
} PACKET;


void error_handling(char *message);
void read_childproc(int sig);
void sendToClient(int sig);

int routine = 0;

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    struct timeval timeout;
    fd_set reads, cpy_reads;

    int pipe_child[2], pipe_parent[2];
    char wbuf[BUF_SIZE], rbuf[BUF_SIZE];

    pid_t pid;
    struct sigaction act;
    socklen_t adr_sz;
    int fd_max, fd_num, i, str_len = 0, state = 0;
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    state = sigaction(SIGCHLD, &act, 0);

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        error_handling("bind() error");
    }

    if (listen(serv_sock, 5) == -1) {
        error_handling("listen() error");
    }

    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;

    PACKET recv_pckt, send_pckt;

    memset(&recv_pckt, 0, sizeof(PACKET));
    memset(&send_pckt, 0, sizeof(PACKET));
    memset(&wbuf, 0, sizeof(BUF_SIZE));
    memset(&rbuf, 0, sizeof(BUF_SIZE));

    pipe(pipe_child);
    pipe(pipe_parent);

    int flag = 0;
    time_t start = 0, end = 0, timeServer;   

    signal(SIGALRM, sendToClient);
    alarm(5);

    while (1)
    {
        cpy_reads = reads;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

         // 몇개의 파일 디스크립터에서 변화가 생겼는가 = select함수
        if ((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1) {
            printf("select error\n");
            break; // 오류 발생시
        }
        if (fd_num == 0) {// 타임 아웃 시 -> 변화 없음
            continue;
        }

        for (i = 0; i < fd_max + 1; i++) { // 변화 존재
            if (FD_ISSET(i, &cpy_reads)) { // 그 중에서 어떤 파일 디스크립터에서 변화가 있었는가? i에서 변화가 있었나?
                if (i == serv_sock) { // ㅇㅇ 있었다. 근데 그게 서버 socket인가? 즉 연결 요청이 있었는가?
                    adr_sz = sizeof(clnt_adr);
                    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);

                    if (clnt_sock == -1) {
                        continue;
                    }
                    else {                    
                        FD_SET(clnt_sock, &reads);
                        if (fd_max < clnt_sock) fd_max = clnt_sock;

                        printf("new client connected: %d\n", clnt_sock);

                        pid = fork(); // 자식 프로세스 생성 -> 클라이언트                      
                    }
                } else { // ㄴㄴ 다른 곳에서 변환 존재 서버 소켓부터 넣었으니까 그 뒤부터임 -> 즉 클라이언트 소켓
                    str_len = read(i, (void *)&recv_pckt, sizeof(PACKET));
                    if (str_len == 0) {
                        FD_CLR(i, &reads);
                        close(i);
                        printf("closed client: %d\n", i);
                    } else {
                        if (pid == -1) {
                            close(clnt_sock);
                            continue;
                        }
                
                        if (pid == 0) { // 자식
                            close(serv_sock);

                            if (recv_pckt.cmd == CMD_DATA) {
                                // 클라이언트로 부터 받은 시간 정보를 파이프를 통하여 부모에게 전달
                                strcpy(wbuf, recv_pckt.payload);
                                write(pipe_child[1], wbuf, strlen(wbuf));

                                read(pipe_parent[0], rbuf, sizeof(BUF_SIZE));

                                if (strcmp(rbuf, "CMD_TERMINATE") == 0) {
                                    send_pckt.cmd = CMD_TERMINATE;
                                    write(i, (void *)&send_pckt, sizeof(PACKET));
                                } else if (strcmp(rbuf, "CMD_DATA") == 0){
                                    send_pckt.cmd = CMD_DATA;
                                    write(i, (void *)&send_pckt, sizeof(PACKET));
                                } else {
                                    printf("Invalid CMD from Parent\n");
                                }
                            }

                            return 0;
                        } else { // 부모
                            close(clnt_sock);

                            // 자식으로부터 받은 클라이언트의 시간 정보를 파이프를 통해 읽음
                            read(pipe_child[0], rbuf, BUF_SIZE);

                            if (flag == 0) {
                                start = time(NULL);
                            } else {
                                end = time(NULL);
                            }

                            double elapsed_time = difftime(end, start);
                            start = end;

                            if (elapsed_time >= 10) {
                                strcpy(wbuf, "CMD_TERMINATE");
                                write(pipe_parent[1], wbuf, BUF_SIZE);
                            } else {
                                strcpy(wbuf, "CMD_DATA");
                                write(pipe_parent[1], wbuf, BUF_SIZE);
                            }
                        }
                    }
                }
            }
            if (routine == 1) {
                send_pckt.cmd = CMD_NOTIFY;
                timeServer = time(NULL);
                struct tm *p;
                p = localtime(&timeServer);
                sprintf(send_pckt.payload,
                     "%d-%d-%d %d:%d:%d", 
                     p->tm_year,
                    p->tm_mon,
                    p->tm_mday,
                    p->tm_hour,
                    p->tm_min,
                    p->tm_sec
                );
                write(i, (void *)&send_pckt, sizeof(PACKET));
                routine = 0;
            }
        }
    }


    close(serv_sock);
    
    return 0;
}

void sendToClient(int sig) {
    if (sig == SIGALRM) {
        routine = 1;
    }
    alarm(5);
}

void read_childproc(int sig) {
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    printf("removed proc id: %d\n", pid);
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}