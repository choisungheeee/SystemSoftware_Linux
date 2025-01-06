#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "student.h"
#include"lecture.h"

// 학생 관리 함수
void student_manage();

// 디렉토리 목록 확인 함수
void directory_list();

// 강좌 관리(레코드 잠금) 함수
void lecture_manage();

int main() {
    int n = -1;

    while(n != 0) {
        printf("1.파일기반 학생점수 관리   2.파일/디렉토리 관리   3.강좌관리(레코드 잠금)   0.종료\n");
        printf("메뉴를 선택하세요: ");
        scanf("%d", &n);

        switch(n) {
            case 1:
                student_manage();
                break;
            case 2:
                directory_list();
                break;
            case 3:
                lecture_manage();
                break;
            case 0:
                return;
        }
    }   
}


