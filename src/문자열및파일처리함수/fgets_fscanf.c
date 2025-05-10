#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *f;
    char buffer[100] = {0};

    if ((f = fopen("sample.txt", "r")) == NULL) {
        printf("File not found\n");
        exit(1);
    }

    // Sample #1: fgets() 사용
    printf("Example 1. fgets()\n");
    while (fgets(buffer, sizeof(buffer), f) != NULL)
    {
        printf("%s", buffer);
    }
    fclose(f);

    if ((f = fopen("sample.txt", "r")) == NULL) {
        printf("File not found\n");
        exit(1);
    }

    printf("Exmple 2. fscanf()\n");
    int num = 0;
    int ret = 0;

    while ((ret = fscanf(f, "%d %s", &num, buffer)) != EOF) 
    {
        // printf("fscanf() ret=%d\n", ret)
        printf("%d %s\n", num, buffer);
    }
    fclose(f);

    return 0;
}


/*
        fgets(input_message, sizeof(BUF_SIZE), stdin);
        if (input_message[strlen(input_message) - 1] != '\0') {
            input_message[strlen(input_message) - 1] = '\0';
        }
*/