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

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	int str_len;
    int fp;

    char filename[BUF_SIZE];

	
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

    Packet recv_pckt, send_pckt;

    memset(&recv_pckt, 0, sizeof(recv_pckt));
    memset(&send_pckt, 0, sizeof(recv_pckt));

    printf("Input file name: ");
    fgets(send_pckt.buf, BUF_SIZE, stdin);
    send_pckt.buf[strlen(send_pckt.buf) - 1] = '\0';

    strcpy(filename, send_pckt.buf);
    printf("[Client] request %s\n\n\n", filename);

    write(sock, (void *)&send_pckt, sizeof(send_pckt));
    
    str_len = read(sock, &recv_pckt, sizeof(recv_pckt));
    if(str_len == -1) {
        error_handling("read() error\n");
    }
    if (recv_pckt.ack == 0 && recv_pckt.seq == 0) {
        printf("%s File Not Found\n", filename);
    } else {
        fp = open(filename, O_CREAT | O_WRONLY, 0644);
        if (fp == -1) {
            printf("open() error\n");
        } else {
            int totalByte = 0;
            send_pckt.ack = recv_pckt.seq + recv_pckt.buf_len + 1;
            totalByte += recv_pckt.buf_len;
            printf("[Client] Tx ACK: %d\n\n", send_pckt.ack);
            write(sock, (void *)&send_pckt, sizeof(send_pckt));

            while ((str_len = read(sock, &recv_pckt, sizeof(recv_pckt))) != 0)
            {
                    printf("[Client] Rx SEQ: %d, len: %d bytes\n", recv_pckt.seq, recv_pckt.buf_len);
                    write(fp, recv_pckt.buf, recv_pckt.buf_len);
    
                    if (recv_pckt.buf_len >= BUF_SIZE) {
                        send_pckt.ack += recv_pckt.buf_len + 1;
    
                        printf("[Client] Tx ACK: %d\n\n", send_pckt.ack);
                        write(sock, (void *)&send_pckt, sizeof(send_pckt));
                    }
                }
            printf("%s received (%d Bytes)\n", filename, totalByte);
        }
    }
    

    printf("Exit client\n");
    
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
