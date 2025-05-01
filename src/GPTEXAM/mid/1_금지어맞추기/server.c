// 2021115744 권구태
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <stdlib.h>

#define MAX 100

typedef struct {
    char msg[MAX];
    int result;
} MSG_PACKET;

void error_handling(char *message);

int main(int argc, char *argv[]) {
    int serv_sock;
    int str_len;
    socklen_t clnt_adr_sz;
    struct sockaddr_in serv_adr, clnt_adr;
    MSG_PACKET recv_pckt, send_pckt;
    char wordCheckList[MAX][MAX];
    char wordList[MAX][MAX];

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    serv_sock = socket(PF_INET, SOCK_DGRAM, 0);

    if (serv_sock == -1) {
        error_handling("UDP socket creation error");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        error_handling("bind() error");
    }

    printf("------------------------\n");
    printf("  금기어 판단 프로그램  \n");
    printf("------------------------\n\n");

    while (1)
    {
        printf("금기어 갯수를 입력하세요: ");

        int n;
        scanf("%d", &n);
    
        printf("금기어 %d개를 입력하세요:\n", n);
        for (int i = 0; i < n; i++) {
            scanf("%s", wordList[i]);
        }
        printf("금기어 입력 끝\n\n");

        clnt_adr_sz = sizeof(clnt_adr);
        str_len = recvfrom(serv_sock, &recv_pckt, sizeof(MSG_PACKET), 0,
                        (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        printf("[Server] Rx msg=%s\n", recv_pckt.msg);

        if (strcmp(recv_pckt.msg, "quit\n") == 0) {
            printf("quit!\n");
            break;
        }

        char *token = strtok(recv_pckt.msg, ", .");
        int j = 0;
        while (token != NULL) {
            for (int i = 0; i < n; i++) {
                if (strcmp(token, wordList[i]) == 0) {
                    strcpy(wordCheckList[j], wordList[i]);
                    j++;
                }
            }
            token = strtok(NULL, ", .");
        }

        for (int i = 0; i <= j; i++) {
            strcat(send_pckt.msg, wordCheckList[i]);
            if (i != j) {
                strcat(send_pckt.msg, ", ");
            }
        }

        send_pckt.result = j;

        sendto(serv_sock, &send_pckt, sizeof(MSG_PACKET), 0,
        (struct sockaddr *)&clnt_adr, clnt_adr_sz);

        
        printf("[Server] Tx words=%s result=%d\n\n", send_pckt.msg, send_pckt.result);
    }

    printf("Server is Over\n");

    close(serv_sock);
    return 0;
}


void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}



