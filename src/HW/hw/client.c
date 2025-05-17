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

#define CMD_DATA 1
#define CMD_NOTIFY 2
#define CMD_TERMINATE 3

typedef struct {
    int cmd;               // 명령 종류
    char payload[BUF_SIZE]; // 시간 또는 메시지
} PACKET;

void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	int str_len;
    char *KeepMsg = "Keep going!";
    char *EndMsg = "Session ended by server";

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

    PACKET recv_pckt, send_pckt;
    int randNum;
    time_t timeNow;

    while (1)
    {
        randNum = (1 + rand() % 10);
        sleep(randNum);

        timeNow = time(NULL);
        struct tm *p;
        p = localtime(&timeNow);
        sprintf(send_pckt.payload,
             "%d-%d-%d %d:%d:%d", 
             p->tm_year,
            p->tm_mon,
            p->tm_mday,
            p->tm_hour,
            p->tm_min,
            p->tm_sec
        );
        send_pckt.cmd = CMD_DATA;
        write(sock, (void *)&send_pckt, sizeof(PACKET));
        printf("[Tx] CMD_DATA: %s\n", send_pckt.payload);

        read(sock, (void *)&recv_pckt, sizeof(PACKET));

        if (recv_pckt.cmd == CMD_NOTIFY) {
            printf("[Rx] CMD_NOTIFY: %s\n", KeepMsg);
            read(sock, (void *)&recv_pckt, sizeof(PACKET));
        } else if (recv_pckt.cmd == CMD_TERMINATE) {
            printf("[Rx] CMD_TERMINATE: %s\n", EndMsg);
            break;
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
