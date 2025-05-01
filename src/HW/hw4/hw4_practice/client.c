#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define BOARD_SIZE 5

#define GAME_REQ 1
#define GAME_RES 2
#define GAME_END 3

// Request Packet: Client -> Server
typedef struct {
    int cmd;
    char ch;
} REQ_PACKET;
// Response Packet: Server -> Client
typedef struct {
    int cmd;
    char board[BOARD_SIZE][BOARD_SIZE];
    int result;
} RES_PACKET;

void error_handling(char *message);
int randNUM();
void printBoard(char board[BOARD_SIZE][BOARD_SIZE]);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_adr, from_adr;
	int str_len;
    socklen_t addr_sz;
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));

    printf("------------------------------------\n");
    printf("    Finding Alphabet Game Client    \n");
    printf("------------------------------------\n");


    srand(time(NULL));

    REQ_PACKET req;
    RES_PACKET res;

    req.ch = randNUM();
    req.cmd = GAME_REQ;

    sendto(sock, &req, sizeof(REQ_PACKET), 0,
    (struct sockaddr*)&serv_adr, sizeof(serv_adr));
    printf("[Client] Tx cmd=%d, ch=%c\n", req.cmd, req.ch);

    while (1)
    {
        req.ch = randNUM();
        req.cmd = GAME_REQ;

        addr_sz = sizeof(from_adr);
        recvfrom(sock, &res, sizeof(RES_PACKET), 0, (struct sockaddr*)&from_adr, &addr_sz);
        printf("[Client] Rx cmd=%d, result=%d\n", res.cmd, res.result);

        if (res.cmd == GAME_RES) {
            // 계속
            printBoard(res.board);
            
            sendto(sock, &req, sizeof(REQ_PACKET), 0,
            (struct sockaddr*)&serv_adr, sizeof(serv_adr));
            printf("[Client] Tx cmd=%d, ch=%c\n", req.cmd, req.ch);
        } else if (res.cmd == GAME_END) {
            // stop
            printf("No empty space. Exit this program.\n");
            break;
        } else {
            printf("wrong %d\n", res.cmd);
            break;
        }
    }
    
    printf("Exit Client Program\n");

	close(sock);
	return 0;
}

int randNUM() {
    return ('A' + (rand() % 26));
}

void printBoard(char board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("+-------------------+\n");
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] != '\0') {
                printf("| %c ", board[i][j]);
            } else {
                printf("|   ");
            }
            if (j == BOARD_SIZE - 1) {
                printf("|\n");
            }
        }
    }
    printf("+-------------------+\n\n");
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
