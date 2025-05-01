// 2021115744 권구태

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>


#define BUF_SIZE 100
#define SEQ_START 1000
typedef struct {
    int seq; // SEQ number
    int ack; // ACK number
    int buf_len; // File read/write bytes
    char buf[BUF_SIZE + 1]; // 파일 이름, "File Not Found" 또는 파일 내용 전송
} Packet;


void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;
	int read_len = 0;
	
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	struct sockaddr_in addr_inet;
	socklen_t clnt_addr_size;

    char recv_msg[BUF_SIZE];
	
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


    printf("--------------------------------\n");
    printf("    File Transmission Server    \n");
    printf("--------------------------------\n");

    Packet pckt;
    ssize_t bytes_read;
    pckt.seq = SEQ_START;
    int sentLegth = 0;
    int fd;

    // 파일 이름을 읽고 파일 이름이 있으면 열고, 없으면 에러 발생 표시
    read_len=read(clnt_sock, recv_msg, BUF_SIZE);
    if (read_len == -1) {
        error_handling("read() error!");
    } else {
        fd = open(recv_msg, O_RDONLY);

        if (fd == -1) {
            printf("%s File Not Found\n", recv_msg);
            strcpy(pckt.buf, "File Not Found");
            pckt.seq = 0;
            pckt.ack = 0;
            pckt.buf_len = strlen(pckt.buf);
            //"File Not Found" client로 전송
            write(clnt_sock, pckt.buf, pckt.buf_len + 1);
    
            close(clnt_sock);	
            close(serv_sock);
            printf("Exit server\n");
            return 0;  // 디렉토리를 열 수 없음
        }
    }


    printf("[Server] sending %s\n\n", recv_msg);

	while ((pckt.buf_len = read(fd, pckt.buf, BUF_SIZE)) > 0) { // 파일 내의 데이터를 읽음 
        printf("[Server] Tx: SEQ: %d, %d byte data\n", pckt.seq, pckt.buf_len);
        // 파일 내의 데이터를 client로 전송
        write(clnt_sock, pckt.buf, pckt.buf_len);
        // 지금까지 읽은 바이트 수 업데이트
        sentLegth += pckt.buf_len;

        // 현재의 SEQ 값을 client로 전송
        write(clnt_sock, &pckt.seq, sizeof(pckt.seq));

        // 클라리언트가 ack 값 read
        read(clnt_sock, &pckt.ack, sizeof(int));

        if (pckt.buf_len < BUF_SIZE) {
            break;
        } else {
            printf("[Server] Rx ACK: %d\n\n", pckt.ack);
            // seq 값 업데이트
            pckt.seq = pckt.ack;
        }
	}
	
    printf("%s sent (%d Bytes)\n", recv_msg, sentLegth);
    printf("Exit server\n");
	close(clnt_sock);	
	close(serv_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

