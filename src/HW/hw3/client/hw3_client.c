// 2021115744 권구태

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>

#define BUF_SIZE 100
typedef struct {
    int seq; // SEQ number
    int ack; // ACK number
    int buf_len; // File read/write bytes
    char buf[BUF_SIZE + 1]; // 파일 이름, "File Not Found" 또는 파일 내용 전송
} Packet;


void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	// 연결 요청 
	if(connect(sock, (struct sockaddr*)&serv_addr, 
				sizeof(serv_addr))==-1) 
		error_handling("connect() error!");


    Packet pckt;

    // 파일 이름 받기
    printf("Input file name: ");
    fgets(pckt.buf, BUF_SIZE, stdin); // 공백이 없는 문자열을 입력할 땐 fgets이 유용
    size_t len = strlen(pckt.buf);
    if (len > 0 && pckt.buf[len - 1] == '\n') {
        pckt.buf[len - 1] = '\0';
    }
    char title[BUF_SIZE];
    strcpy(title, pckt.buf); // 입력받은 파일명 복사 해놓기. 나중에 써야 함.
    pckt.buf_len = strlen(pckt.buf); 
    pckt.seq = 0;
    pckt.ack = 0;
    if (pckt.buf_len > 0 && pckt.buf[pckt.buf_len - 1] == '\n') {
        pckt.buf[pckt.buf_len - 1] = '\0';
    } 

    printf("[Client] request %s\n\n", pckt.buf);
    // 서버 파일 이름 전송
    write(sock, pckt.buf, pckt.buf_len + 1);
    int recvLegth = 0; // 총 읽어들인 바이트 수를 저장하기 위함.

    while (1)
	{
		pckt.buf_len=read(sock, pckt.buf, BUF_SIZE); // 파일의 데이터를 읽는 read 함수
		if (pckt.buf_len == 0) {  // 서버가 연결을 종료한 경우
			break;
		} else if (pckt.buf_len == -1) {
			error_handling("read() error!");
            break;
		} else if (strcmp(pckt.buf, "File Not Found") == 0) {
            printf("File Not Found\n");
            printf("Exit client\n");
            return 0;
        } 
        // SEQ의 넘버를 읽는 read 함수
        read(sock, &pckt.seq, sizeof(pckt.seq));  // &pckt.seq

        printf("[Client] Rx SEQ: %d, len: %d bytes\n", pckt.seq, pckt.buf_len);
        // 지금까지 읽은 바이트 수 업데이트
        recvLegth += pckt.buf_len;

        if (pckt.buf_len < BUF_SIZE) {
            break;
        } else {
            // ack 값 업데이트
            pckt.ack = pckt.seq + pckt.buf_len + 1;
            // ack를 계산하고 서버로 보내는 write 함수
            write(sock, &pckt.ack, sizeof(pckt.ack)); // &pckt.ack
            printf("[Client] Tx ACK: %d\n\n", pckt.ack);
        }		
	};
    printf("%s recieved (%d Bytes)\n", title, recvLegth);
	close(sock);
    printf("Exit client\n");
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
