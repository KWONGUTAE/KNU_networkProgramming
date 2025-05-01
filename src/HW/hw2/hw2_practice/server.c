#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;
    int str_len;
    char *error_message = "Address conversion fail: Format error";
    char *success_message = "Address conversion success";
    char *str_ptr;

    char rcv_message[BUF_SIZE];
    char send_message[BUF_SIZE];
	
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

	
	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	/* 서버 소켓(리스닝 소켓) 생성 */
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
		error_handling("socket() error");
	
	/* 주소 정보 초기화 */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	
	/* 주소 정보 할당 */
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1 )
		error_handling("bind() error"); 
	
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	clnt_addr_size=sizeof(clnt_addr);  
	clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
	if(clnt_sock==-1)
		error_handling("accept() error");  
	
    printf("---------------------------\n");
    printf(" Address Conversion Server \n");
    printf("---------------------------\n");

    
    while (1) {
        str_len=read(clnt_sock, rcv_message, sizeof(rcv_message));
        if(str_len==-1)
            error_handling("read() error!");
        
        if (strcmp(rcv_message, "quit") == 0) {
            break;
        } else {
            printf("[Rx] Received Dotted-Decimal Address: %s\n", rcv_message);
            if (!inet_aton(rcv_message, &serv_addr.sin_addr)) {
                printf("[Tx] %s\n\n", error_message);
                strcpy(send_message, error_message);
                write(clnt_sock, send_message, sizeof(send_message));
            } else {
                printf("inet_aton: %s -> %#x\n", rcv_message, serv_addr.sin_addr.s_addr);
    
                str_ptr = inet_ntoa(serv_addr.sin_addr);
                if (str_ptr != -1) {
                    printf("inet_ntoa: %#x -> %s\n", serv_addr.sin_addr.s_addr, str_ptr);
                    printf("[Tx] %s\n\n", success_message);
                    strcpy(send_message, success_message);
                    write(clnt_sock, send_message, sizeof(send_message));
                }
            }  
        }              
    }

    printf("quit received and exit program!\n");
    
	close(clnt_sock);	
	close(serv_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
