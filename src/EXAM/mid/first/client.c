#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
// 2021115744 권구태

#define GREP_REQ 1 // Client -> Server
#define GREP_RES 2 // Server -> Client
#define GREP_END 3 // Client -> Server
#define GREP_END_ACK 4 // Server -> Client
// Remote Grep Reqeust (Client -> Server)
typedef struct
{
int cmd; // GREP_REQ, GREP_END
char options[100]; // "옵션 검색어 파일이름"
} REQ_PACKET;
// Remote Grep Response (Server -> Client)
typedef struct
{
int cmd; // GREP_RES, GREP_END_ACK
int result; // 검색된 라인 수
// -1: file not found
// -2: invalid option(-n, -v, -i 가 아닌 경우)
char matched_lines[2048]; // 검색 결과(라인번호 및 검색된 파일 내용)
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
	
    REQ_PACKET req_pckt;
    RES_PACKET res_pckt;

    while (1)
    {
        // 클라이언트는 화면에서 “옵션 검색어 파일명”을 입력 받고 REQ_PACKET의 
        // options[] 배열에 저장 후 서버로 전송(cmd=GREP_REQ) (2점)

        printf("Type [option] [keyword] [filename]: ");
        fgets(req_pckt.options, sizeof(req_pckt.options), stdin);
        size_t len = strlen(req_pckt.options);
        if (len > 0 && req_pckt.options[len - 1] == '\n') {
            req_pckt.options[len - 1] = '\0';
        }

        if (strcmp(req_pckt.options, "quit") == 0) {
            req_pckt.cmd = GREP_END;
            printf("[Tx] GREP_END(3)\n");
        } else {
            req_pckt.cmd = GREP_REQ;
        }
        write(sock, &req_pckt, sizeof(REQ_PACKET));
        printf("[Tx] GREP_REQ(1) options: %s\n", req_pckt.options);
        printf("-----------------------------------\n");

        read(sock, &res_pckt, sizeof(RES_PACKET));

        if (res_pckt.cmd == GREP_RES) {
            printf("[Rx] GREP_RES(2), result: %d\n", res_pckt.cmd);
            printf("-----------------------------------\n");
            if (res_pckt.cmd == -1) {
                printf("File not found!\n");
            } else if (res_pckt.cmd == -2) {
                printf("Invalid option\n");
            } else {
                printf("%s\n", res_pckt.matched_lines);
                printf("-----------------------------------\n");
            }
        } else if (res_pckt.cmd == GREP_END_ACK) {
            printf("[Tx] GREP_END_ACK(3)\n\n\n");
            break;
        } else {
            printf("이상한 값\n");
        }

        
    }
    
    printf("Exit rGrep Client\n");

	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
