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
void printBoard(char answerBoard[][BOARD_SIZE], char emptyBoard[][BOARD_SIZE]);
void fillAnswerBoard(char board[BOARD_SIZE][BOARD_SIZE]);
int randNUM();


int main(int argc, char *argv[])
{
	int serv_sock;
    int str_len;
    char answerBoard[BOARD_SIZE][BOARD_SIZE];
    char emptyBoard[BOARD_SIZE][BOARD_SIZE];

	
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

	
	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	/* 서버 소켓(리스닝 소켓) 생성 */
	serv_sock=socket(PF_INET, SOCK_DGRAM, 0);
	if(serv_sock == -1)
		error_handling("UDP socket creation error");
	
	/* 주소 정보 초기화 */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	
	/* 주소 정보 할당 */
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1 )
		error_handling("bind() error"); 
	
    printf("------------------------------------\n");
    printf("    Finding Alphabet Game Server    \n");
    printf("------------------------------------\n");

    srand(time(NULL));

    REQ_PACKET req;
    RES_PACKET res;

    fillAnswerBoard(answerBoard);
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            res.board[i][j] = '\0';
        }
    }
    printBoard(answerBoard, res.board);

    while (1)
    {
        //sleep(1);
        res.result = 0;
        int countEmpty = 0;
        clnt_addr_size = sizeof(clnt_addr);
        recvfrom(serv_sock, &req, sizeof(REQ_PACKET), 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        printf("[Server] Rx cmd=%d, ch=%c\n", req.cmd, req.ch);

        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (res.board[i][j] == '\0') {
                    countEmpty++;
                }
            }
        }

        if (req.cmd == GAME_REQ) {
            for (int i = 0; i < BOARD_SIZE; i++) {
                for (int j = 0; j < BOARD_SIZE; j++) {
                    if (answerBoard[i][j] == req.ch) {
                        res.result++;
                        res.board[i][j] = answerBoard[i][j];
                    }
                }
            }
            if (countEmpty == 0) {
                res.cmd = GAME_END;
                sendto(serv_sock, &res, sizeof(RES_PACKET), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);
                printf("[Server] Tx cmd=%d, ch=%d\n", res.cmd, res.result);
                printf("No empty space. Exit this program.\n");
                break;
            } else {
                res.cmd = GAME_RES;
                sendto(serv_sock, &res, sizeof(RES_PACKET), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);
                printf("[Server] Tx cmd=%d, ch=%d\n", res.cmd, res.result);
            }
            printBoard(answerBoard, res.board);
        } else {
            printf("wrong %d\n", req.cmd);
            break;
        }
    }
    
    printf("Exit Server Program\n");

	close(serv_sock);
	return 0;
}

void printBoard(char answerBoard[][BOARD_SIZE], char emptyBoard[][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("+-------------------+ ");
        printf("+-------------------+\n");
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (answerBoard[i][j] != '\0') {
                printf("| %c ", answerBoard[i][j]);
            } else {
                printf("|   ");
            }
            if (j == BOARD_SIZE - 1) {
                printf("| ");
            }
        }
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (emptyBoard[i][j] != '\0') {
                printf("| %c ", emptyBoard[i][j]);
            } else {
                printf("|   ");
            }
            if (j == BOARD_SIZE - 1) {
                printf("|\n");
            }
        }
    }
    printf("+-------------------+ ");
    printf("+-------------------+\n\n");
}

void fillAnswerBoard(char board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = randNUM();
        }
    }
}

int randNUM() {
    return ('A' + (rand() % 26));
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}


