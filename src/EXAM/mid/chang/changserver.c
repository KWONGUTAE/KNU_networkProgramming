#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>


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

int search_keyword(char *req_option, char *buf) {
    char option[3] = {0};
    char searchword[50] = {0};
    char filename[50] = {0};

    int search_count = 0; // 검색된 라인수 저장
    int line_cnt = 1; // 라인번호 출력
    char line[200]; // 파일에서 한 라인씩 읽기 위한 버퍼
    char matched_line[250]; // 검색된 결과 저장(라인번호 + 파일 내용)

    memset(line, 0, sizeof(line));
    memset(matched_line, 0, sizeof(matched_line));

    // req_packet.options 내용을 공백 기준으로 분리함
    sscanf(req_option, "%s %s %s", option, searchword, filename);

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("File not found: %s\n", filename);
        strcpy(buf, filename);
        return -1;
    } else {
        while (fgets(line, sizeof(line), fp) != NULL) 
        {
            if (strcmp(option, "-n") == 0) {
                if (strstr(line, searchword) != NULL) {
                    sprintf(matched_line, "%3d: %s", line_cnt, line);
#ifdef DEBUG
                    printf("%s", matched_line);
#endif
                    strcat(buf, matched_line);
                    search_count++;
                }
            }
            else if (strcmp(option, "-v") == 0) {
                if (strstr(line, searchword) == NULL) {
                    sprintf(matched_line, "%3d: %s", line_cnt, line);
#ifdef DEBUG
                    printf("%s", matched_line);
#endif                    
                    strcat(buf, matched_line);
                    search_count++;
                }
            }
            else if (strcmp(option, "-i") == 0) {
                if (strcasestr(line, searchword) != NULL) {
                    sprintf(matched_line, "%3d: %s", line_cnt, line);
#ifdef DEBUG
                    printf("%s", matched_line);
#endif                    
                    strcat(buf, matched_line);
                    search_count++;
                }
            } else {
                printf("Invalid option: %s\n", option);
                return -2;
            }
            line_cnt++;
        }
    }
    fclose(fp);
    return search_count;
}

int main(int argc, char *argv[]) {
    // socket 관련 변수 선언
    int serv_sock, client_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    REQ_PACKET req_packet;
    RES_PACKET res_packet;
    int result = 0;
    // grep 관련 변수 선언

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    memset(&req_packet, 0, sizeof(REQ_PACKET));

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1) {
        error_handling("bind() error");
    }

    if (listen(serv_sock, 5) == -1) {
        error_handling("listen() error");
    }

    clnt_adr_sz = sizeof(clnt_adr);
    client_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
    if (client_sock == -1) {
        error_handling("accept() error");
    }

    while (1)
    {
        // [Rx] Server <------- Client
        // GREP_REQ
        read(client_sock, (void *)&req_packet, sizeof(REQ_PACKET));
        memset(&res_packet, 0, sizeof(RES_PACKET));

        if (req_packet.cmd == GREP_REQ) {
            printf("[Rx] GREP_REQ(%d), options: %s\n", req_packet.cmd, req_packet.options);
            /**
             -----------------------------------------------
                REQ_PACKET의 options 및 결과를 저장하기 위한
                res_packet.result를 인자로 전달
             -----------------------------------------------
             */

            result = search_keyword(req_packet.options, res_packet.matched_lines);

            /*
            -----------------------------------------------
                GREP_RES 전송(Server -> Client)
            -----------------------------------------------
            */
            res_packet.cmd = GREP_RES;
            res_packet.result = result;
            write(client_sock, (void *)&res_packet, sizeof(RES_PACKET));

            printf("-----------------------------------------------\n");
            printf("[Tx] GREP_RES(%d), result: %d \n", res_packet.cmd, res_packet.result);
            printf("-----------------------------------------------\n");

            if(res_packet.result >= 0) {
                int length = strlen(res_packet.matched_lines);
                if (res_packet.matched_lines[length - 1] != '\n') {
                    res_packet.matched_lines[length - 1] = '\n';
                }
                printf("%s", res_packet.matched_lines);
                printf("-----------------------------------------------\n");
            } 
        } else if (req_packet.cmd == GREP_END) {
            printf("[Rx] GREP_END(%d) \n", req_packet.cmd);

            // GREP_END_ACK 전송
            res_packet.cmd = GREP_END_ACK;
            write(client_sock, (void *)&res_packet, sizeof(RES_PACKET));
            printf("[Tx] GREP_END_ACK(%d) \n\n", res_packet.cmd);
            break;
        } else {
            printf("Invalid cmd(%d)\n", req_packet.cmd);
        }
    }
    printf("Exit rGrep Server\n");
    close(client_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}