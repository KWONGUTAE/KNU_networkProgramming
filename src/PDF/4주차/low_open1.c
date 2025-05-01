/**
 * low_open1.c
 *  - char buf[]에서 크기를 지정하지 않고, sizeof(buf)를 사용하여 파일로 저장한 경우
 *  - buf[] 배열 마지막에 NULL 문자가 자동으로 추가됨 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    int fd;
    int size;    
    char buf[100]="Let's go!\n";
   
    fd=open("data1.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644);
    if(fd == -1)
        printf("open() error!");

	// strlen(buf): exclude NULL character
    printf("strlen(buf): %ld\n", strlen(buf));
   	printf("sizeof(buf): %ld\n", sizeof(buf));

    size = write(fd, buf, sizeof(buf));
    printf("write size: %d\n", size);    
    
    close(fd);
    return 0;
}
