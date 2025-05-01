#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	char message[BUF_SIZE];
    char address[BUF_SIZE];
	int str_len;
	int idx = 0;
	int read_len = 0;
	uint32_t conv_addr;
	
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
// Data Tx -> Rx
    while (1)
	{
		// 주소 입력 받기
		printf("Input dotted-decimal address: ");
		fgets(address, BUF_SIZE, stdin); // 공백이 없는 문자열을 입력할 땐 fgets이 유용
		// 입력 시 정확히 BUF_SIZE - 1 크기만큼 입력하면 해당 문자열의 인덱스에 유효한 값이 있는데, 이를 NULL로 바꿔버림. 이를 방지하기 위함.
		size_t len = strlen(address);
		if (len > 0 && address[len - 1] == '\n') {
			address[len - 1] = '\0';
		}

		// 서버 주소 전송
		write(sock, address, sizeof(address));


		/*
			서버에게 quit를 전송하고 나서 아래 if 문을 바로 실행하는게 아니라,
			read 함수로 서버에서 보내는 데이터를 먼저 기다림.
			그래서 서버에서는 quit가 실행되면 close하니까 이를 반영해서
			0이 오면 while문 종료하면 됨.
			if (strcmp(address, "quit") == 0) {
			break;
		}
		*/
		if (strcmp(address, "quit") == 0) {
			break;
		}

		read_len=read(sock, message, BUF_SIZE);
		if (read_len == 0) {  // 서버가 연결을 종료한 경우
			break;
		} else if (read_len == -1) {
			error_handling("read() error!");
		}
		
		printf("%s\n", message);
	};
	
	printf("quit!\n");
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
