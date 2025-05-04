// 2021115744 권구태

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	int str_len;
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	// 연결 요청 
	if(connect(sock, (struct sockaddr*)&serv_addr, 
				sizeof(serv_addr))==-1) 
		error_handling("connect() error!");

    printf("Connected..............\n");

    PACKET send_pckt, recv_pckt;

    int randNum;
    time_t timeNow;

    for (int i = 1; i <= 10; i++) {
        randNum = (1 + rand() % 5);  // 1 ~ 5 사이의 랜덤한 값
        sleep(randNum);

        timeNow = time(NULL);
        struct tm *p;
        p = localtime(&timeNow);
        sprintf(send_pckt.time_msg, "%d:%d:%d",
            p->tm_hour, 
            p->tm_min, 
            p->tm_sec
        );
        send_pckt.cmd = PING_MSG;

#ifdef DEBUG
        printf("%s\n", clnt_pckt.time_msg)
#endif

        write(sock, &send_pckt, sizeof(send_pckt));
        printf("[Tx] PING(%d), sleep(%d), [%2d]: %s => ", send_pckt.cmd, randNum, i, send_pckt.time_msg);

        read(sock, &recv_pckt, sizeof(recv_pckt));

        if (recv_pckt.cmd == TERMINATE_MSG) {
            printf("[Rx] TERMINATE(%d), %s\n", recv_pckt.cmd, recv_pckt.time_msg);
            break;
        } else if (recv_pckt.cmd == PONG_MSG) {
            printf("[Rx] PONG(%d), time: %s\n", recv_pckt.cmd, recv_pckt.time_msg);
        } else {
            printf("Invalid cmd\n");
        }
    }

    printf("Client close\n");
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
