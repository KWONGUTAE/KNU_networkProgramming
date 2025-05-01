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
    int sock;
    int str_len;
    socklen_t adr_sz;
    struct sockaddr_in serv_adr, from_adr;
    MSG_PACKET recv_pckt, send_pckt;

    if (argc != 3) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    sock = socket(PF_INET, SOCK_DGRAM, 0);

    if (sock == -1) {
        error_handling("UDP socket creation error");
    }

    memset(&serv_adr, 0, sizeof(serv_adr)); // from_adr도 초기화 해야 함.
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    printf("------------------------\n");
    printf("  금기어 판단 프로그램  \n");
    printf("------------------------\n\n");

    while (1)
    {
        printf("문장를 입력하세요: ");

        fgets(send_pckt.msg, MAX, stdin);
        sendto(sock, &send_pckt, sizeof(MSG_PACKET), 0, (struct sockaddr *)&serv_adr, sizeof(serv_adr));
        printf("[Client] Tx msg=%s\n", send_pckt.msg);

        if (strcmp(send_pckt.msg, "quit\n") == 0) {
            printf("quit!\n");
            break;
        }
        adr_sz = sizeof(from_adr);
        str_len = recvfrom(sock, &recv_pckt, sizeof(MSG_PACKET), 0,
            (struct sockaddr *)&from_adr, &adr_sz);

        printf("[Client] Rx: word=%s, result=%d\n\n", recv_pckt.msg, recv_pckt.result);
    }
    
    printf("Client is Over\n");

    close(sock);

    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}