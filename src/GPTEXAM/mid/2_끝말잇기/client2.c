#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>


#define MAX 100

void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	int str_len;
    char recv_msg[MAX], send_msg[MAX];

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



    while (1)
    {
        memset(&recv_msg, 0, sizeof(recv_msg));
        memset(&send_msg, 0, sizeof(send_msg));

        printf("Input word: ");
        fgets(send_msg, sizeof(send_msg), stdin);
        
        write(sock, send_msg, sizeof(send_msg));
        printf("\n[Client] Tx word is %s\n", send_msg);

        read(sock, recv_msg, sizeof(recv_msg));
        printf("[Client] Rx word is %s\n\n", recv_msg);

        if(strcmp(recv_msg, "Invalid") == 0) {
            printf("Game is Over.\n");
            break;
        }

    }
    printf("Exit client\n");
    
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
