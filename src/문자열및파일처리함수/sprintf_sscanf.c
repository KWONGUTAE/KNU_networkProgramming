#include <stdio.h>

int main() {
    char *token = "25 54.32 Thompson 56789";
    int n1, n2;
    char name1[20];
    double d1;

    // token 문자열을 띄어쓰기 기준으로 형식에 맞게 나눈다.
    // n1 d1 name1 n2로 각각 들어감.
    // sscanf 한다고 내가 직접 입력 해야 하는게 아니라, 이미 만들어진 문자열을 분해하는 것임.
    sscanf(token, "%d %lf %s %d", &n1, &d1, name1, &n2);
    printf("%d, %lf, %s, %d\n", n1, d1, name1, n2);

    char buf[256];
    char name[20] = "홍길동";
    int num = 23;
    double level = 2.37;

    // 분해된 변수들을 하나의 문자열로 합침.
    // sprintf를 단순히 사용한다고 출력되는 것이 아니라, 합치고자 하는 buf에 써지는 것.
    // printf를 사용해야 터미널에 써짐.
    sprintf(buf, "Name: %s, No %d, Level: %f", name, num, level);
    printf("%s\n", buf);
}