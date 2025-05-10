#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);
void write_routine(int sock, char *buf);
void read_routine(int sock, char *buf);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
    char input_message[BUF_SIZE];
    char serv_message[BUF_SIZE];
	int str_len;
    pid_t pid;
	
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

    printf("Connected...........\n");

    pid = fork();
    while (1)
    {
        if (pid == 0) { // 입출력 스트림 분리
            write_routine(sock, input_message);
        } else {
            read_routine(sock, input_message)
        }
    }

	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void write_routine(int sock, char *buf) {
    while (1)
    {
        printf("Input message(Q to quit): ");
        fgets(buf, BUF_SIZE, stdin);
        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n") || !strcmp(buf, "quit\n")) {
            shutdown(sock, SHUT_WR);
            return;
        }
        write(sock, buf, strlen(buf));
    }
}

void read_routine(int sock, char *buf) {
    while (1)
    {
        int str_len = read(sock, buf, BUF_SIZE);
        if (str_len == 0) {
            return;
        }

        buf[str_len] = 0;
        printf("Message from server: %s", buf);
    }
}
