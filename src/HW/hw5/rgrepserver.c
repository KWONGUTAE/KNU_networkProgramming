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

typedef struct {
    int cmd;
    char options[100];
} REQ_PACKET;

typedef struct {
    int cmd;
    int result;

    char matched_lines[2048];
} RES_PACKET;


void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;
    int str_len;

	
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

    REQ_PACKET req;
    RES_PACKET res;
    FILE *f = NULL;
    char option[100], keyword[100], filename[100];
    char buf[1024];
    int lineNum, count;
    char lineNumToStr[100];

    while (1)
    {
        lineNum = 0;
        count = 0;
        memset(&res, 0, sizeof(res));
        memset(&req, 0, sizeof(req));

        read(clnt_sock, &req, sizeof(req));

        if (req.cmd == GREP_REQ) {
            // 진행
            res.cmd = GREP_RES;
            printf("[Rx] GREP_REQ(1), options: %s\n", req.options);
            sscanf(req.options, "%s %s %s", option, keyword, filename);

            if ((f = fopen(filename, "r")) == NULL) {
                printf("File not found: %s\n", filename);
                printf("---------------------------------\n");
                res.result = -1;
                write(clnt_sock, &res, sizeof(res));
                printf("[Tx] GREP_RES(2), result: %d\n", res.result);
                printf("---------------------------------\n");
            } else {
                if ((strncmp(option, "-n", 2)) == 0) {
                    while (fgets(buf, sizeof(buf), f) != NULL)
                    {
                        lineNum++;
                        if ((strstr(buf, keyword)) != NULL) {
                            count++;
                            sprintf(lineNumToStr, "%d", lineNum);
                            strcat(res.matched_lines, "  ");
                            strcat(res.matched_lines, lineNumToStr);
                            strcat(res.matched_lines, ": ");
                            strcat(res.matched_lines, buf);
                        }
                    }
                    res.result = count;
                    printf("[Tx] GREP_RES(2), result %d\n", res.result);
                    printf("---------------------------------\n");
                    printf("%s", res.matched_lines);
                    printf("---------------------------------\n");
                    write(clnt_sock, &res, sizeof(res));

                } else if ((strncmp(option, "-v", 2)) == 0) {
                    while (fgets(buf, sizeof(buf), f) != NULL)
                    {
                        lineNum++;
                        sprintf(lineNumToStr, "%d", lineNum);
                        if ((strstr(buf, keyword)) == NULL) {
                            count++;
                            strcat(res.matched_lines, "  ");
                            strcat(res.matched_lines, lineNumToStr);
                            strcat(res.matched_lines, ": ");
                            strcat(res.matched_lines, buf);
                        }
                    }
                    res.result = count;
                    printf("[Tx] GREP_RES(2), result %d\n", res.result);
                    printf("---------------------------------\n");
                    printf("%s", res.matched_lines);
                    printf("---------------------------------\n");
                    write(clnt_sock, &res, sizeof(res));

                } else if ((strncmp(option, "-i", 2)) == 0) {
                    while (fgets(buf, sizeof(buf), f) != NULL)
                    {
                        lineNum++;
                        if ((strcasestr(buf, keyword)) != NULL) {
                            count++;
                            sprintf(lineNumToStr, "%d", lineNum);
                            strcat(res.matched_lines, "  ");
                            strcat(res.matched_lines, lineNumToStr);
                            strcat(res.matched_lines, ": ");
                            strcat(res.matched_lines, buf);
                        }
                    }
                    res.result = count;
                    printf("[Tx] GREP_RES(2), result %d\n", res.result);
                    printf("---------------------------------\n");
                    printf("%s", res.matched_lines);
                    printf("---------------------------------\n");
                    write(clnt_sock, &res, sizeof(res));
                } else {
                    printf("Invalid option: %s\n", option);
                    printf("---------------------------------\n");
                    res.result = -2;
                    write(clnt_sock, &res, sizeof(res));
                    printf("[Tx] GREP_RES(2), result: %d\n", res.result);
                    printf("---------------------------------\n");
                }
            }
        } else if (req.cmd == GREP_END) {
            // 끝
            printf("[Rx] GREP_END(3)\n");
            res.cmd = GREP_END_ACK;
            write(clnt_sock, &res, sizeof(res));
            memset(&res, 0, sizeof(res));
            memset(&req, 0, sizeof(req));
            printf("[Tx] GREP_END_ACK(4)\n");

            break;
        } else {
            // Invalid value
            res.cmd = 0;
            write(clnt_sock, &res, sizeof(res));
            memset(&res, 0, sizeof(res));
            memset(&req, 0, sizeof(req));

            printf("Invalid value");
        }
    }

    if (f != NULL) {
        fclose(f);
    }
	close(clnt_sock);	
	close(serv_sock);

    printf("\nExit rGrep Server\n");

	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
