#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUF_SIZE 100
#define SEQ_START 1000
typedef struct {
    int seq;
    int ack;
    int buf_len;
    char buf[BUF_SIZE + 1];
} Packet;

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;
    int str_len;
    int fp;
    char filename[BUF_SIZE];
    char *error_msg = "File Not Found";

	
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;
	
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

    Packet recv_pckt, send_pckt;

    printf("--------------------------------\n");
    printf("    File Transmission Server    \n");
    printf("--------------------------------\n");


    // 파일 이름 받기
    str_len = read(clnt_sock, &recv_pckt, sizeof(recv_pckt));
    if (str_len == -1) {
        error_handling("read() error!");
    }

    strcpy(filename, recv_pckt.buf);

    fp = open(filename, O_RDONLY);
    if (fp == -1) {
        // 파일 없거나 오픈 못했을 때
        send_pckt.ack = 0;
        send_pckt.seq = 0;
        strcpy(send_pckt.buf, error_msg);
        send_pckt.buf_len = strlen(send_pckt.buf);
        write(clnt_sock, &send_pckt, sizeof(send_pckt));

        printf("%s File Not Found\n", filename);
    } else {
        // 파일 존재함. 오픈 했을 때
        printf("[Server] sending %s\n\n", filename);

        int first = 0;
        int totalByte = 0;
        while ((str_len = read(fp, &send_pckt.buf, BUF_SIZE)) != 0) {
            send_pckt.buf[str_len] = '\0';
            send_pckt.buf_len = strlen(send_pckt.buf);

            if (first == 0) {
                // 첫시도
                send_pckt.ack = 0;
                send_pckt.seq = SEQ_START;
                write(clnt_sock, (void *)&send_pckt, sizeof(send_pckt));
                printf("[Server] Tx: SEQ: %d, %d byte data\n", send_pckt.seq, send_pckt.buf_len);
                totalByte += send_pckt.buf_len;

                str_len = read(clnt_sock, &recv_pckt, sizeof(recv_pckt));
                if (str_len == -1) {
                    printf("read() error!\n");
                }
                printf("[Server] Rx: ACK: %d\n\n", recv_pckt.ack);

                send_pckt.seq = recv_pckt.ack;

                first = 1;
            } else {
                write(clnt_sock, (void *)&send_pckt, sizeof(send_pckt));
                printf("[Server] Tx: SEQ: %d, %d byte data\n", send_pckt.seq, send_pckt.buf_len);
                totalByte += send_pckt.buf_len;

                if (send_pckt.buf_len >= BUF_SIZE) {
                    str_len = read(clnt_sock, &recv_pckt, sizeof(recv_pckt));
                    if (str_len == -1) {
                        printf("read() error!\n");
                    }
                    printf("[Server] Rx: ACK: %d\n\n", recv_pckt.ack);
    
                    send_pckt.seq = recv_pckt.ack;
                }
            }
            
        }
        printf("%s sent (%d Bytes)\n", filename, totalByte);
    }

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
