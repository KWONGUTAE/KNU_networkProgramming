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

#define BUF_SIZE 100
#define PING_MSG 1
#define PONG_MSG 2
#define TERMINATE_MSG 3

typedef struct {
    int cmd;
    char time_msg[BUF_SIZE];
} PACKET;


void error_handling(char *message);
void read_childproc(int sig);

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    pid_t pid;
    struct sigaction act;
    socklen_t adr_sz;
    int str_len = 0, state = 0;
    char buf[BUF_SIZE];
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

    clnt_sock = 0;
    PACKET recv_pckt, send_pckt;

    while (1)
    {
        adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
        if (clnt_sock == -1) {
            continue;
        } else {
            printf("new client connected: %d\n", clnt_sock);
        }
        pid = fork();
        if (pid == -1) {
            close(clnt_sock);
            continue;
        }

        if (pid == 0) {
            // 통신 용도 processor
            close(serv_sock);
            int flag = 0;
            time_t start, end, timeSend;    
            struct tm *p;        
            int hour = 0, min = 0, sec = 0;

            while ((str_len = read(clnt_sock, &recv_pckt, sizeof(PACKET))) != 0)
            {
                if (recv_pckt.cmd == PING_MSG) {
                    printf("[Rx] PING(%d) time: %s from Port(%u)=> ", recv_pckt.cmd, recv_pckt.time_msg, clnt_adr.sin_port);
                    sscanf(recv_pckt.time_msg, "%d:%d:%d", &hour, &min, &sec);
                    if (flag == 0) {
                        start = time(NULL);
                        flag = 1;
                    } 
                    end = time(NULL);
                    double elapsed_time = difftime(end, start);
                    start = end;

                    if (elapsed_time > 4) {
                        send_pckt.cmd = TERMINATE_MSG;
                        strcpy(send_pckt.time_msg, "Connection close");
                        printf("[Tx] TERMINATE(%d): %s to Port(%u)\n", send_pckt.cmd, send_pckt.time_msg, clnt_adr.sin_port);
                        write(clnt_sock, &send_pckt, sizeof(send_pckt));
                        break;
                    } else {
                        send_pckt.cmd = PONG_MSG;
                        timeSend = time(NULL);
                        p = localtime(&timeSend);
                        sprintf(send_pckt.time_msg, "%d:%d:%d",
                            p->tm_hour, 
                            p->tm_min, 
                            p->tm_sec
                        );
                        write(clnt_sock, &send_pckt, sizeof(send_pckt));
                        printf("[Tx] PONG(%d) %s to Port(%u)\n", send_pckt.cmd, send_pckt.time_msg, clnt_adr.sin_port);
                    
                    }                    
                } else {
                    printf("Invalid cmd\n");
                }
            }
            close(clnt_sock);
            puts("client disconnected...");
            return 0;
        } else {
            //  accept 용도 processor
            printf("close clnt sock: %d\n", clnt_sock);
            close(clnt_sock);
        }
    }
    close(serv_sock);
    
    return 0;
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