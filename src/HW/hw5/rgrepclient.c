// 2021115744 권구태
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define GREP_REQ 1
#define GREP_RES 2
#define GREP_END 3
#define GREP_END_ACK 4

// client -> server
typedef struct {
    int cmd;
    char options[100];
} REQ_PACKET;

// server -> client
typedef struct {
    int cmd;
    int result;

    char matched_lines[2048];
} RES_PACKET;


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
	

    REQ_PACKET req;
    RES_PACKET res;

    while (1)
    {
        printf("Type [option] [keyword] [filename]: ");
        fgets(req.options, sizeof(req.options), stdin);

        if ((strncmp(req.options, "quit", 4)) == 0) {
            req.cmd = GREP_END;
            write(sock, &req, sizeof(req));
            printf("[Tx] GREP_END(3)\n");
        } else {
            req.cmd = GREP_REQ;
            write(sock, &req, sizeof(req));
            printf("[Tx] GREP_REQ(1) options: %s\n", req.options);
            printf("---------------------------------\n");
        }


        read(sock, &res, sizeof(res));

        if (res.cmd == GREP_RES) {
            // 계속 진행
            if (res.result == -2) {
                // 옵션 잘못됨
                printf("[Rx] GREP_RES(2), result: %d\n", res.result);
                printf("---------------------------------\n");
                printf("Invalid options\n");
            } else if (res.result == -1) {
                // 파일 없음
                printf("[Rx] GREP_RES(2), result: %d\n", res.result);
                printf("---------------------------------\n");
                printf("File not found!\n");
            } else {
                // 정상
                printf("[Rx] GREP_RES(2), result: %d\n", res.result);
                printf("---------------------------------\n");
                printf("%s", res.matched_lines);
                printf("---------------------------------\n");
            }
        } else if (res.cmd == GREP_END_ACK) {
            printf("[Rx] GREP_END_ACK(4)\n");
            break;
        } else {
            printf("Invalid value\n");
        }
    }

	close(sock);

    printf("\nExit rGrep Client\n");

	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
