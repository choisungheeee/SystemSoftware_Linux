#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>

void student();
void dir();
void lecture();

void manage() {
    int n = -1;

    while(n != 0) {
        printf("\n--- 메인 프로그램입니다. ---\n\n");
        printf("1. 파일기반 학생점수 관리\n");
        printf("2. 파일/디렉토리 관리\n");
        printf("3. 강좌 관리(레코드 잠금)\n");
        printf("0. 종료\n\n");
        printf("메뉴를 선택하세요: "); scanf("%d", &n);

        switch(n) {
            case 1: student(); break;
            case 2: dir(); break;
            case 3: lecture(); break;
            case 0: return;
        }
    }
}