// 2021115744 권구태

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

#define TTL 64
#define BUF_SIZE 120
#define TEXT_SIZE 100

void error_handling(char *message);
void close_child(int sig);
int flag = 0;
int recv_sock;
struct ip_mreq join_adr;
char *drop_mbs_msg = "Multicast drop Membership and Exit\n";


int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <GroupIP> <PORT> <Name>\n", argv[0]);
        exit(1);
    }

    pid_t pid;
    pid = fork();

    if (pid == -1) {
        error_handling("fork error");        
    }
    if (pid == 0) {
        // 자식
        struct sockaddr_in recv_adr;
        
        char recv_buf[BUF_SIZE];
        int option, str_len = 0;
        socklen_t optlen;

        struct sigaction act;
        socklen_t adr_sz;
        int state = 0;

        act.sa_handler = close_child;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        state = sigaction(SIGTERM, &act, 0);

        optlen = sizeof(option);
        option = 1;

        recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
        setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, &option, optlen);

        memset(&recv_adr, 0, sizeof(recv_adr));
        recv_adr.sin_family = AF_INET;
        recv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
        recv_adr.sin_port = htons(atoi(argv[2]));

        if (bind(recv_sock, (struct sockaddr*)&recv_adr, sizeof(recv_adr)) == -1) {
            error_handling("bind() error");
        }

        // 가입할 멀티캐스트 그룹 주소 및 자신의 IP 주소 설정
        join_adr.imr_multiaddr.s_addr = inet_addr(argv[1]);
        join_adr.imr_interface.s_addr = htonl(INADDR_ANY);


        
        setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));

        while (1)
        {
            memset(&recv_buf, 0, sizeof(recv_buf));
            str_len = recvfrom(recv_sock, recv_buf, BUF_SIZE - 1, 0, NULL, 0);
            if (str_len < 0) {
                break;
            } else if ((strlen(recv_buf) == 2) && ((strncmp(recv_buf, "q", 1) == 0) || (strncmp(recv_buf, "Q", 1) == 0))) {
                return 0;
            } else {
                recv_buf[str_len] = 0;
                fputs(recv_buf, stdout);
            }
        }

        printf("[Child] recv_sock closed\n");

    } else {
        // 부모
        int send_sock;
        struct sockaddr_in mul_adr;
        int time_live = TTL;
        char send_buf[BUF_SIZE], temp_buf[TEXT_SIZE];
        time_t timeNow;
        struct tm *p;        
        int hour = 0, min = 0, sec = 0;

        send_sock = socket(PF_INET, SOCK_DGRAM, 0);
        memset(&mul_adr, 0, sizeof(mul_adr));
        mul_adr.sin_family = AF_INET;
        mul_adr.sin_addr.s_addr = inet_addr(argv[1]);
        mul_adr.sin_port = htons(atoi(argv[2]));

        setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));

        while (1)
        {
            memset(&send_buf, 0, sizeof(send_buf));
            memset(&temp_buf, 0, sizeof(temp_buf));
            fgets(temp_buf, TEXT_SIZE, stdin);
            if ((strlen(temp_buf) == 2) && ((strncmp(temp_buf, "q", 1) == 0) || (strncmp(temp_buf, "Q", 1) == 0))) {
                printf("SIGTERM: Multicast Receiver terminate!\n");
                 // 부모 입장에서는 pid가 자식 프로세스의 pid값을 들고 있는 것임. 본인 것은 getpid()로 구할 수 있다.
                kill(pid, SIGTERM);
                close(send_sock);
                printf("[Parent] send_sock closed\n");                
                
                sendto(send_sock, temp_buf, strlen(temp_buf), 0, (struct sockaddr*)&mul_adr, sizeof(mul_adr));
                return 0;
            } else 
            timeNow = time(NULL);
            p = localtime(&timeNow);
            sprintf(send_buf, "%02d:%02d:%02d [%s] %s",
                p->tm_hour,
                p->tm_min,
                p->tm_sec,
                argv[3],
                temp_buf
            );
            sendto(send_sock, send_buf, strlen(send_buf), 0, (struct sockaddr*)&mul_adr, sizeof(mul_adr));
        }
    }
    
    return 0;
}

void close_child(int sig) {
    setsockopt(recv_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));
    printf("%s", drop_mbs_msg);
    close(recv_sock);
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}