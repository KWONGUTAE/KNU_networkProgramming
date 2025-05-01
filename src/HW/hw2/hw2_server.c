#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;
    int idx = 0;
	int read_len = 0;
	char *str_ptr;
	uint32_t temp;
	
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	struct sockaddr_in addr_inet;
	socklen_t clnt_addr_size;

	char SuccessMessage[]="[Rx] Address conversion success\n";
	char FailMessage[] = "[Rx] Address conversio fail: Format error.\n";
	char SuccessMessageInServer[]="[Tx] Address conversion success\n";
	char FailMessageInServer[] = "[Tx] Address conversio fail: Format error.\n";
	char FinishMessage[] = "quit received and exit program!\n";
	char address[BUF_SIZE];
	
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

	printf("---------------------------\n");
	printf(" Address Conversion Server \n");
	printf("---------------------------\n");
	// Data Rx -> Tx
	while (1)
	{
		// 클라이언트에서 보낸 문자열 주소를 읽어들임. 읽어들인 주소를 address 문자열에 저장
		read_len=read(clnt_sock, address, BUF_SIZE);
		if(read_len==-1) {
			error_handling("read() error!");
		} else if (strcmp(address, "quit") == 0) {
			break;
		} else {
			printf("[Rx] Received Dotted-Decimal Address: %s\n", address);
			// 문자열로 받은 주소를 32비트 숫자로 변환
			if (!inet_aton(address, &addr_inet.sin_addr)) {
				// 잘못된 주소를 입력했을 시, 오류 메세지 전송
				printf("%s\n", FailMessageInServer);
				write(clnt_sock, FailMessage, sizeof(FailMessage));
				// sizeof는 NULL문자 포함, strlen는 미포함
				// strlen 하면 strlen() + 1 해줘야
			} else {
				// 문자열 주소 -> 숫자로 변환 성공
				printf("inet aton: %s -> %#x\n", address, addr_inet.sin_addr.s_addr);

				// 32비트 숫자를 다시 문자열로 변환
				str_ptr = inet_ntoa(addr_inet.sin_addr);
				printf("inet ntoa: %#x -> %s\n", addr_inet.sin_addr.s_addr, str_ptr);
				// 따로 


				printf("%s\n", SuccessMessageInServer);
				// 주소 변환 성공했다는 메세지 전공
				write(clnt_sock, SuccessMessage, sizeof(SuccessMessage));
			}
		}
		/*
		printf("\n");
		memset(rcv_msg, 0, sizeof(rcv_msg));
		memset(send_msg, 0, sizeof(rcv_msg));
		메세지 초기화 부분 꼭 필요함.
		rcv_msg, send_msg 따로 사용
		*/
	}
	
	printf("%s\n", FinishMessage);
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
