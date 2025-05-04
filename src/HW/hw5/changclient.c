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

int main(int argc, char *argv[]) {
    // socket 관련 변수 선언
    int sock;
    struct sockaddr_in serv_addr;

    REQ_PACKET req_packet;
    RES_PACKET res_packet;

    char line_input[100] = {0};

    if (argc != 3) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        error_handling("socket() error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("connect error");
    }

    while (1)
    {
        memset(&req_packet, 0, sizeof(REQ_PACKET));
        memset(&res_packet, 0, sizeof(RES_PACKET));

        // Tx
        printf("Type [option] [keyword] [filename]: ");
        fgets(line_input, sizeof(line_input), stdin);
        line_input[strlen(line_input) - 1] = '\0';

        // "quit" 입력 시 종료
        if (strcmp(line_input, "quit") == 0) {
            // [Tx] Client ----> Server
            // GREP_END
            // 전송 후 client 종료
            req_packet.cmd = GREP_END;
            write(sock, (void *)&req_packet, sizeof(REQ_PACKET));
            printf("[Tx] GREP_END(%d)\n", req_packet.cmd);
            //break;
        } else {
            // [Tx] Client -----------> Server
            // GREP_REQ
            req_packet.cmd = GREP_REQ;
            strcpy(req_packet.options, line_input);
            write(sock, (void *)&req_packet, sizeof(REQ_PACKET));
            printf("[Tx] GREP_REQ(%d) options: %s\n", req_packet.cmd, req_packet.options);
        }

        read(sock, (void *)&res_packet, sizeof(RES_PACKET));
        if (res_packet.cmd == GREP_RES) {
            printf("--------------------------------------\n");
            printf("[Rx] GREP_RES(%d), result: %d \n", res_packet.cmd, res_packet.result);
            printf("--------------------------------------\n");

            if (res_packet.result == -1) {
                printf("File not found!\n");
            } else if (res_packet.result == -2) {
                printf("Invalid option\n");
            } else {
                int length = strlen(res_packet.matched_lines);
                if (res_packet.matched_lines[length - 1] != '\n') {
                    res_packet.matched_lines[length - 1] = '\n';
                }

                printf("%s", res_packet.matched_lines);
                printf("--------------------------------------\n");
            }
        } else if (res_packet.cmd == GREP_END_ACK) {
            printf("[Rx] GREP_END_ACK(%d) \n\n", res_packet.cmd);
            break;
        } else {
            printf("[Rx] Invalid cmd(%d)\n", res_packet.cmd);
        }
    }
    

    printf("Exit rGrep Client\n");
    close(sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}