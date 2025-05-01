#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define MAX 100

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;
    int str_len;
    int fp;
    char recv_msg[MAX], send_msg[MAX];
    char tmp[MAX];
    int wordlengths[MAX];

	
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

    // listen 함수
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
    // accept 함수
	clnt_addr_size=sizeof(clnt_addr);  
	clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
	if(clnt_sock==-1)
		error_handling("accept() error");  

    // 각 배열 초기화
    memset(recv_msg, 0, sizeof(recv_msg));
    memset(send_msg, 0, sizeof(send_msg));
    memset(tmp, 0, sizeof(tmp));

    // 맞춘 단어의 수
    int count = 0;

    // 첫 단어 이름 받기
    str_len = read(clnt_sock, recv_msg, sizeof(recv_msg));
    if (str_len == -1) {
        error_handling("read() error!");
    }

    // 파일 생성 및 열기
    fp = open("word.txt", O_CREAT | O_RDONLY | O_WRONLY, 0644);
    if (fp == -1) {
        // 파일 없거나 오픈 못했을 때
        printf("%s File Not Found\n", "word.txt");
    } else {
        // 파일 존재함. 오픈 했을 때
        printf("[Server] Writing %s in word.txt\n\n", recv_msg);
        write(fp, recv_msg, strlen(recv_msg)); // 파일쓰기
        // 각 단어의 길이를 세기(나중에 단어 불러올때 해당 단어의 바이트 수 만큼 불러오기 위함.)
        wordlengths[count++] = strlen(recv_msg); 
        strcpy(tmp, recv_msg); // 임시 배열에 저장.

        // 첫 단어는 ok 해서 보내기
        strcpy(send_msg, "OK");
        write(clnt_sock, send_msg, sizeof(send_msg));

        while (1) {
            int tmp_len = strlen(tmp);

            // 다음 단어 받기.
            str_len = read(clnt_sock, recv_msg, sizeof(recv_msg));
            // recv_msg[str_len] = '\0'; 이건 fgets로 받았을 땐 해야 함.
            printf("[Server] Rx word is %s\n", recv_msg);

            if (tmp[tmp_len - 1] == recv_msg[0]) {
                write(fp, recv_msg, strlen(recv_msg));
                wordlengths[count++] = strlen(recv_msg);
                strcpy(send_msg, "OK");
                write(clnt_sock, send_msg, sizeof(send_msg));
                printf("[Server] Tx %s is Valid Word\n\n", recv_msg);
                strcpy(tmp, recv_msg);
            } else {
                printf("[Server] Tx %s is Invalid Word\n\n", recv_msg);
                strcpy(send_msg, "Invalid");
                write(clnt_sock, send_msg, sizeof(send_msg));
                break;
            }
            memset(recv_msg, 0, sizeof(recv_msg));
            memset(send_msg, 0, sizeof(send_msg));
        }
    }
    close(fp);
    fp = open("word.txt", O_RDONLY);
    if (fp == -1) {
        // 파일 없거나 오픈 못했을 때
        printf("%s File Not Found\n", "word.txt");
    } else {
        int i = 0;
        memset(tmp, 0, sizeof(tmp));
        while ((str_len = read(fp, tmp, wordlengths[i])) != 0)
        {
            printf("%s\n", tmp);
            i++;
            memset(tmp, 0, sizeof(send_msg));
        }
    }
    close(fp);


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
