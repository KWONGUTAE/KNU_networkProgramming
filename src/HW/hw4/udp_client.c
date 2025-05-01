// 2021115744 권구태
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <stdlib.h>

#define BOARD_SIZE 5
// cmd type
#define GAME_REQ 1
#define GAME_RES 2
#define GAME_END 3
// Request Packet: Client -> Server
typedef struct
{
int cmd; // GAME_REQ
char ch; // 알파벳 대문자 하나 전송
} REQ_PACKET;
// Response Packet: Server -> Client
typedef struct
{
int cmd; // GAME_RES, GAME_END
char board[BOARD_SIZE][BOARD_SIZE]; // 맞춘 알파벳만 저장 후 클라이언트로 전송
int result; // 맞춘 알파벳의 개수 전달
} RES_PACKET;

void error_handling(char *message);
char get_random_alphabet();
void printFrame(char (*board)[5]);

int main(int argc, char *argv[]) {
    int sock;
    int str_len;
    socklen_t adr_sz;
    struct sockaddr_in serv_adr, from_adr;
    REQ_PACKET req_packet;
    RES_PACKET res_packet;

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



    printf("------------------------------------\n");
    printf("    Finding Alphabet Game Server    \n");
    printf("------------------------------------\n");


    req_packet.cmd = GAME_REQ;
    req_packet.ch = get_random_alphabet();

    sendto(sock, &req_packet, sizeof(REQ_PACKET), 0,
        (struct sockaddr *)&serv_adr, sizeof(serv_adr));
    
    printf("[Client] Tx cmd=%d, ch=%c\n", req_packet.cmd, req_packet.ch);


    while (1)
    {
        adr_sz = sizeof(from_adr);
        str_len = recvfrom(sock, &res_packet, sizeof(RES_PACKET), 0,
            (struct sockaddr *)&from_adr, &adr_sz);

        printf("[Client] Rx cmd=%d, result=%d\n", res_packet.cmd, res_packet.result);
    
        if (res_packet.cmd == GAME_RES) // 이런식으로 GAME_RES인지 비교하며 코딩하는 것은 기본임.
        // input이 무엇인지에 따라 if/else 문으로 코드를 짜는 것. input이 무엇이 올지 모르니까.
        {

            // GAME_RAQ: 알파벳 랜덤 생성 및 서버로 전송 -> 이런식으로 큰 틀에 해당하는 주석 코드를 달며 개발해야 함.
            printFrame(res_packet.board);
            req_packet.cmd = GAME_REQ;
            req_packet.ch = get_random_alphabet();

            printf("[Client] Tx cmd=%d, ch=%c\n", req_packet.cmd, req_packet.ch);
    
            sendto(sock, &req_packet, sizeof(REQ_PACKET), 0,
                (struct sockaddr *)&serv_adr, sizeof(serv_adr));
    
        }
        else if (res_packet.cmd == GAME_END)
        {
            printf("No empty space. Exit this program.\n");
            break;
        }
        else // 나의 코드의 무결성을 증명하기 위한 코드. 일종의 방어코임
        {
            printf("Invalid cmd: %d\n", res_packet.cmd);
        }
    }
    printf("Exit Client Program\n");
    close(sock);

    return 0;
}

char get_random_alphabet() {
    srand(time(NULL));
    return 'A' + (rand() % 26);
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void printFrame(char (*board)[5]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("+-------------------+\n");

        for (int j = 0; j < BOARD_SIZE; j++) {
            if(board[i][j] != '\0') {
                printf("| %c ", board[i][j]);
            } else {
                printf("|   ");

            }
        }
        printf("|\n");
    }
    printf("+-------------------+\n\n");
}