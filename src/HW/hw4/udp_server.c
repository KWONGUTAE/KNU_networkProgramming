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

char get_random_alphabet();
void printFrame(char (*strArr)[BOARD_SIZE], char (*board)[BOARD_SIZE]);
int spaceCount(char (*board)[BOARD_SIZE]);
int countAndInsertAlphabet(char (*strArr)[BOARD_SIZE], char (*board)[BOARD_SIZE], char *alphabet);
void initOriginalMatrix(char (*strArr)[BOARD_SIZE]);
void initCopyMatrix(char (*board)[BOARD_SIZE]);
void error_handling(char *message);

int main(int argc, char *argv[]) {
    int serv_sock;
    int str_len;
    socklen_t clnt_adr_sz;
    struct sockaddr_in serv_adr, clnt_adr;
    REQ_PACKET req_packet;
    RES_PACKET res_packet;
    char str_arr[BOARD_SIZE][BOARD_SIZE] = {'\0'};

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    serv_sock = socket(PF_INET, SOCK_DGRAM, 0);

    if (serv_sock == -1) {
        error_handling("UDP socket creation error");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    //memset(&res_packet.board, ' ', sizeof(res_packet.board)); -> 이렇게 초기화도 가능.
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        error_handling("bind() error");
    }

    // 좌측 행렬 랜덤하게 지정
    srand(time(NULL));

    initOriginalMatrix(str_arr);
    initCopyMatrix(res_packet.board);

    printf("------------------------------------\n");
    printf("    Finding Alphabet Game Server    \n");
    printf("------------------------------------\n");

    printFrame(str_arr, res_packet.board);

    while (1)
    {
        clnt_adr_sz = sizeof(clnt_adr);
        str_len = recvfrom(serv_sock, &req_packet, sizeof(REQ_PACKET), 0,
                        (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        printf("[Server] Rx cmd=%d, ch=%c\n", req_packet.cmd, req_packet.ch);

        // 다 채웠으면 끝내려고 얼마나 비었는지 체크
        int space_count = spaceCount(res_packet.board);
        
        if (space_count == 0) // 다 맞춤
        {

            // GAME_END 패킷 전송
            res_packet.cmd = GAME_END;
            res_packet.result = 0;

            printf("[Server] Tx cmd=%d, result=%d\n", res_packet.cmd, res_packet.result);
            printf("No empty space. Exit this program.\n");

            sendto(serv_sock, &res_packet, sizeof(RES_PACKET), 0,
                (struct sockaddr *)&clnt_adr, clnt_adr_sz);

            break;
        }

        if (req_packet.cmd == GAME_REQ)
        {
            int count = countAndInsertAlphabet(str_arr, res_packet.board, &req_packet.ch);

            res_packet.cmd = GAME_RES;
            res_packet.result = count;

            printf("[Server] Tx cmd=%d, result=%d\n", res_packet.cmd, res_packet.result);

            printFrame(str_arr, res_packet.board);

            sendto(serv_sock, &res_packet, sizeof(RES_PACKET), 0,
                (struct sockaddr *)&clnt_adr, clnt_adr_sz);
            sleep(1);
        }
        else 
        {
            printf("Invalid cmd: %d\n", req_packet.cmd);
        }
    }

    printf("Exit Server Program\n");
    close(serv_sock);


    return 0;
}

char get_random_alphabet() {
    return 'A' + (rand() % 26);
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void printFrame(char (*strArr)[BOARD_SIZE], char (*board)[BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("+-------------------+");
        printf(" ");
        printf("+-------------------+\n");
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("| %c ", strArr[i][j]);
        }
        printf("| ");

        for (int j = 0; j < BOARD_SIZE; j++) {
            if(board[i][j] != '\0') {
                printf("| %c ", board[i][j]);
            } else {
                printf("|   ");

            }
        }
        printf("|\n");
    }
    printf("+-------------------+");
    printf(" ");
    printf("+-------------------+\n\n");
}

int spaceCount(char (*board)[BOARD_SIZE]) {
    int space_count = 0;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == '\0') { // 하나라도 NULL 이면
                space_count++; // 빈공간이니까 
            }
        }
    }
    
    return space_count;
}

int countAndInsertAlphabet(char (*strArr)[BOARD_SIZE], char (*board)[BOARD_SIZE], char *alphabet) {
    int count = 0;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (strArr[i][j] == *alphabet) {
                board[i][j] = *alphabet;
                count++;
            }
        }
    }

    return count;
}

void initOriginalMatrix(char (*strArr)[BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            strArr[i][j] = get_random_alphabet();
        }
    }
}

void initCopyMatrix(char (*board)[BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = '\0';
        }
    }
}