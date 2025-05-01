#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
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

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;
    int str_len;
    char *str_ptr;
    char tmp[100][100];
    char tmparr[100][100];
    int countI = 0;
    int countN = 0;
    int countV = 0;
    int lineNum[100];

	
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
	
    REQ_PACKET req_pckt;
    RES_PACKET res_pckt;

    while (1)
    {
        // 서버는 전송된 GREP_REQ 패킷의 options[] 배열의 내용을 공백을 기준으로 분
        // 리(옵션, 검색어, 파일명) (2점)    

        read(clnt_sock, &req_pckt, sizeof(REQ_PACKET));
        if (req_pckt.cmd == GREP_REQ) {
            printf("[Rx] GREP_REQ(1), options: %s\n", req_pckt.options);
            char *token = strtok(req_pckt.options, " ");
            int i = 0;
            while (token != NULL) {
                strcpy(tmp[i++], token);
                token = strtok(NULL, " ");
            }
            
            FILE *fp = fopen(tmp[2], "r");
            if (fp == NULL) {
                // 파일 없거나 오픈 못했을 때
                printf("File not found: %s\n", tmp[2]);
                printf("-----------------------------------\n");
                res_pckt.cmd = GREP_RES;
                res_pckt.result = -1;
                write(clnt_sock, &res_pckt, sizeof(res_pckt));
                printf("[Tx] GREP_RES(2), result: %d\n", res_pckt.result);
                printf("-----------------------------------\n");
            } else {
                // 파일 오픈 성공
                if ((strcmp(tmp[0], "-n") != 0) && (strcmp(tmp[0], "-i") != 0) && (strcmp(tmp[0], "-v") != 0)) {
                    // 잘못된 옵션
                    printf("Invalid option: %s\n", tmp[0]);
                    printf("-----------------------------------\n");
                    res_pckt.cmd = GREP_RES;
                    res_pckt.result = -2;
                    write(clnt_sock, &res_pckt, sizeof(res_pckt));
                    printf("[Tx] GREP_RES(2), result: %d\n", res_pckt.result);
                    printf("-----------------------------------\n");
                } else {
                    // 제대로 된 옵션

                    char ch;
                    int i = 0;
                    int lineNumCount = 0;
                    while ((ch = fgetc(fp)) != EOF)
                    {
                        while ((ch = fgetc(fp)) != '\n') {
                            strcat(tmparr[i], &ch);
                        }
                        if(strcmp(tmp[0], "-i") == 0) {
                            // 대소문자 구분 없이 포함하면 출력
                            if (strcasestr(tmparr[i], tmp[1])) {
                                lineNum[lineNumCount++] = i + 1;
                                countI++;
                                res_pckt.result = countI;
                            }
                        } else if (strcmp(tmp[0], "-n") == 0) {
                            if(strstr(tmparr[i], tmp[1])) {
                                lineNum[lineNumCount++] = i + 1;
                                countN++;
                                res_pckt.result = countN;
                            }
                        } else if (strcmp(tmp[0], "-v") == 0) {
                            if(strstr(tmparr[i], tmp[1]) == NULL) {
                                lineNum[lineNumCount++] = i + 1;
                                countV++;
                                res_pckt.result = countV;
                            }
                        }
                        i++;
                    }
                    printf("[Tx] GREP_RES(2), result: %d\n", res_pckt.result);
                    printf("-----------------------------------\n");
                    
                    for (int j = 0; j < i; j++) {
                        printf("%d: %s\n", lineNum[j], tmparr[j]);
                        strcat(res_pckt.matched_lines, tmparr[j]);
                        strcat(res_pckt.matched_lines, "\n");
                    }
                    printf("-----------------------------------\n");
                 

                    write(clnt_sock, &res_pckt, sizeof(res_pckt));
                }
            }
        } else if (req_pckt.cmd == GREP_END) {
            printf("[Rx] GREP_END(3)\n");
            res_pckt.cmd = GREP_END_ACK;
            write(clnt_sock, &res_pckt, sizeof(res_pckt));
            printf("[Tx] GREP_END_ACK(3)\n\n\n");
            break;
        } else {
            printf("이상한 값\n");
        }
    }
    

    printf("Exit rGrep Server\n");
    
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
