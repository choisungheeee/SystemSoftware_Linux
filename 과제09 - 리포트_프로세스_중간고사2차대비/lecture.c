#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include "main.h"

typedef struct {
    int id;        // 강좌번호 
    char name[20]; // 강좌명(공백 가능)
    char pro[12];  // 교수명(공백 가능)
    char sub[15];  // 개설학과명
    int max;       // 최대수강인원
    char note[40]; // 비고(공백 가능)
} Lecture;

Lecture lec;
struct flock lock;

// 1. 강좌 파일명 지정
void create() {
    char name[20];

    printf("생성할 파일명 입력: "); scanf(" %s", name);

    if((fd = open(name, O_RDWR | O_CREAT, 0666)) == -1) {
        perror(name);
        exit(1);
    }

    printf("파일 생성 완료\n\n");
}

// 2. 강좌 목록 보기(강좌명 정렬)
void list() {
    int count = 0;

    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if(fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("잠금 실패\n\n");
        return;
    }
    
    lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동
    while((read(fd, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
        count += 1;
    }
    Lecture tmp[count];

    int i = 0;
    lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동
    while((read(fd, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
        tmp[i++] = lec;
    }

    for(int i=0; i<count-1; i++) {
        for(int j=i+1; j<count; j++) {
            if(strcmp(tmp[i].name, tmp[j].name) > 0) {
                Lecture tmp2 = tmp[i];
                tmp[i] = tmp[j];
                tmp[j] = tmp2;
            }
        }
    }

    for(int i=0; i<count; i++) {
        printf("강좌번호: %d, 강좌명: %s, 교수명: %s, ", tmp[i].id, tmp[i].name, tmp[i].pro);
        printf("개설학과명: %s, 최대수강인원: %d, 비고: %s\n\n", tmp[i].sub, tmp[i].max, tmp[i].note);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
}

// 3. 신규 강좌 생성
void new() {
    printf("강좌 정보를 입력하세요.\n\n");
    printf("강좌번호: "); scanf(" %d", &lec.id);

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = lec.id*sizeof(lec);
    lock.l_len = sizeof(lec);
    if(fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("잠금 실패\n\n");
        return;
    }
    printf("강좌명: "); scanf(" %[^\n]", lec.name);
    printf("교수명: "); scanf(" %[^\n]", lec.pro);
    printf("개설학과명: "); scanf(" %s", lec.sub);
    printf("최대수강인원: "); scanf(" %d", &lec.max);
    printf("비고: "); scanf(" %[^\n]", lec.note);

    lseek(fd, lec.id*sizeof(lec), SEEK_SET);
    if(write(fd, &lec, sizeof(lec)) == -1) {
        printf("강좌 생성 실패\n\n");
        return;
    }
    printf("강좌 생성 완료\n\n");

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
}

// 4. 강좌 정보 변경
void update() {
    int id;

    printf("변경할 강좌 번호 입력: "); scanf(" %d", &id);
    lseek(fd, id*sizeof(lec), SEEK_SET);
    if((read(fd, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = lec.id*sizeof(lec);
        lock.l_len = sizeof(lec);
        if(fcntl(fd, F_SETLKW, &lock) == -1) {
            perror("잠금 실패\n\n");
            return;
        }

        printf("현재 강좌 정보\n\n");
        printf("강좌번호: %d, 강좌명: %s, 교수명: %s, ", lec.id, lec.name, lec.pro);
        printf("개설학과명: %s, 최대수강인원: %d, 비고: %s\n\n", lec.sub, lec.max, lec.note);

        printf("변경할 강좌 정보 입력\n\n");
        printf("강좌번호: "); scanf(" %d", &lec.id);
        printf("강좌명: "); scanf(" %[^\n]", lec.name);
        printf("교수명: "); scanf(" %[^\n]", lec.pro);
        printf("개설학과명: "); scanf(" %s", lec.sub);
        printf("최대수강인원: "); scanf(" %d", &lec.max);
        printf("비고: "); scanf(" %[^\n]", lec.note);

        lseek(fd, -sizeof(lec), SEEK_CUR);
        if(write(fd, &lec, sizeof(lec)) == -1) {
            printf("변경 실패\n\n");
            return;
        }
        printf("변경 완료\n\n");

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }
}

// 5. 강좌 정보 검색(강좌명 검색)
void search() {
    char name[20];
    int found = 0;

    printf("검색할 강좌명: "); scanf(" %s", name);
    lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동
    while((read(fd, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
        if(strstr(lec.name, name) != NULL) {
            found += 1;
            printf("강좌번호: %d, 강좌명: %s, 교수명: %s, ", lec.id, lec.name, lec.pro);
            printf("개설학과명: %s, 최대수강인원: %d, 비고: %s\n\n", lec.sub, lec.max, lec.note);
        }
    }
    if(!found) {
        printf("조건에 맞는 강좌 정보가 없습니다.\n\n");
    }
}

// 6. 강좌 삭제
void del() {
    int id;

    printf("삭제할 강좌 번호 입력: "); scanf(" %d", &id);
    lseek(fd, id*sizeof(lec), SEEK_SET);
    if((read(fd, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = lec.id*sizeof(lec);
        lock.l_len = sizeof(lec);
        if(fcntl(fd, F_SETLKW, &lock) == -1) {
            perror("잠금 실패\n\n");
            return;
        }

        printf("현재 강좌 정보\n\n");
        printf("강좌번호: %d, 강좌명: %s, 교수명: %s, ", lec.id, lec.name, lec.pro);
        printf("개설학과명: %s, 최대수강인원: %d, 비고: %s\n\n", lec.sub, lec.max, lec.note);

        lec.id = -1;

        lseek(fd, -sizeof(lec), SEEK_CUR);
        if(write(fd, &lec, sizeof(lec)) == -1) {
            printf("삭제 실패\n\n");
            return;
        }
        printf("삭제 완료\n\n");

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }
}

void lecture() {
    int n = -1;

    while(n != 0) {
        printf("\n--- 강좌관리 프로그램입니다. ---\n\n");
        printf("1. 파일 생성\n");
        printf("2. 강좌 목록 보기(강좌명 정렬)\n");
        printf("3. 신규 강좌 생성\n");
        printf("4. 강좌 정보 변경\n");
        printf("5. 강좌 정보 검색(강좌명 검색)\n");
        printf("6. 강좌 삭제\n");
        printf("0. 종료\n\n");
        printf("메뉴를 선택하세요: "); scanf(" %d", &n);

        switch(n) {
            case 1: create(); break;
            case 2: list(); break;
            case 3: new(); break;
            case 4: update(); break;
            case 5: search(); break;
            case 6: del(); break;
            case 0: close(fd); return;
        }
    }
}