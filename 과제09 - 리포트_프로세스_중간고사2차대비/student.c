#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include"main.h"

typedef struct {
    int id;        // 학번(0부터 시작)
    char name[20]; // 이름
    char sub[25];  // 과목명
    short score;   // 점수
    float mod;     // 보정점수
} Student;

Student rec;

// 1. 점수파일 생성
void create_student_file() {
    char name[20];
    
    printf("\n생성할 파일명 입력: "); scanf(" %s", name);

    if((fd = open(name, O_RDWR | O_CREAT, 0666)) == -1) {
        perror(name);
        exit(1);
    }

    printf("파일 생성 완료\n\n");
}

// 2. 신규 학생 정보 추가
void new_student() {
    printf("\n학생정보를 입력하세요.\n\n");
    printf("학번: "); scanf(" %d", &rec.id);
    printf("이름: "); scanf(" %s", rec.name);
    printf("과목명: "); scanf(" %s", rec.sub);
    printf("점수: "); scanf(" %d", &rec.score);
    printf("보정점수: "); scanf(" %f", &rec.mod);

    lseek(fd, rec.id*sizeof(rec), SEEK_SET);
    if(write(fd, &rec, sizeof(rec)) != -1) {
        printf("추가 완료\n\n");
    }
    else {
        printf("추가 실패\n\n");
    }    
}

// 3. 학생 정보 수정
void update_student() {
    int id; 

    printf("수정할 학생의 학번 입력: "); scanf(" %d", &id);

    lseek(fd, id*sizeof(rec), SEEK_SET);

    if((read(fd, &rec, sizeof(rec)) > 0) && (rec.id != -1)) {
        printf("현재 학생 정보\n\n");
        printf("학번: %d, 이름: %s, 과목명: %s, ", rec.id, rec.name, rec.sub);
        printf("점수: %d, 보정점수: %.2f\n\n", rec.score, rec.mod);

        printf("수정할 학생 정보 입력\n\n");
        printf("학번: "); scanf(" %d", &rec.id);
        printf("이름: "); scanf(" %s", rec.name);
        printf("과목명: "); scanf(" %s", rec.sub);
        printf("점수: "); scanf(" %d", &rec.score);
        printf("보정점수: "); scanf(" %f", &rec.mod);

        lseek(fd, -sizeof(rec), SEEK_CUR);
        if(write(fd, &rec, sizeof(rec)) != -1) {
            printf("수정 완료\n\n");
        }
        else {
            printf("수정 실패\n\n");
        }    
    }
}

// 4. 학생 정보 삭제
void del_student() {
    int id; 

    printf("삭제할 학생의 학번 입력: "); scanf(" %d", &id);

    lseek(fd, id*sizeof(rec), SEEK_SET);

    if((read(fd, &rec, sizeof(rec)) > 0) && (rec.id != -1)) {
        printf("현재 학생 정보\n\n");
        printf("학번: %d, 이름: %s, 과목명: %s, ", rec.id, rec.name, rec.sub);
        printf("점수: %d, 보정점수: %.2f\n\n", rec.score, rec.mod);

        rec.id = -1;

        lseek(fd, -sizeof(rec), SEEK_CUR);
        if(write(fd, &rec, sizeof(rec)) != -1) {
            printf("삭제 완료\n\n");
        }
        else {
            printf("삭제 실패\n\n");
        }    
    }
}

// 5. 학생정보 검색(이름 검색)
void search_student() {
    int found = 0;
    char name[20];

    printf("검색할 학생의 이름 입력: "); scanf(" %s", name);

    lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동

    while((read(fd, &rec, sizeof(rec)) > 0) && (rec.id != -1)) {
        if(strcmp(rec.name, name) == 0) {
            found = 1;

            printf("학번: %d, 이름: %s, 과목명: %s, ", rec.id, rec.name, rec.sub);
            printf("점수: %d, 보정점수: %.2f\n\n", rec.score, rec.mod);

            break;
        }
    }
    if(!found) {
        printf("검색한 학생의 정보가 없습니다.\n\n");
    }
}

void student() {
    int n = -1;

    while(n != 0) {
        printf("\n--- 학생 점수 관리 프로그램입니다. ---\n\n");
        printf("1. 점수파일 생성\n");
        printf("2. 신규 학생정보 추가\n");
        printf("3. 학생정보 수정\n");
        printf("4. 학생정보 삭제\n");
        printf("5. 학생정보 검색(이름 검색)\n");
        printf("0. 종료\n\n");

        printf("메뉴를 선택하세요: "); scanf(" %d", &n);

        switch(n) {
            case 1: create_student_file(); break;
            case 2: new_student(); break;
            case 3: update_student(); break;
            case 4: del_student(); break;
            case 5: search_student(); break;
            case 0: close(fd); return;
        }
    }
}