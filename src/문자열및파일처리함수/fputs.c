#include <stdio.h>
#include <string.h>
#define NUM_ALPHA 26

int main(void) {
    FILE *stream;
    int ret;
    // NULL('\0') 문자를 저장하기 위해 [NUM_ALPHA + 1] 사용
    char buffer[NUM_ALPHA + 1] = "abcdefghijklmnopqrstuvwxyz";
    
    if ((stream = fopen("myfile", "w")) != NULL) {
        if ((ret = fputs(buffer, stream)) != EOF) {
            // 파일에는 NULL 문자 저장 안됨
            printf("fputs() ret = %d\n", ret);
            fclose(stream);
        } else {
            perror("fputs failed");
        }
    } else {
        perror("Error opening myfile");
    }

    return 0;
}