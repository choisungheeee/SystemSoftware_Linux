#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include"lecture.h"
#define MAX_LEC 100

void create_file(); // 파일 생성
void sort_lec(); // 강좌명 정렬
void add_lec();  // 신규 강좌 생성
void change_lec(); // 강좌정보 변경
void search_lec(); // 강좌정보 검색
void delete_lec(); // 강좌정보 삭제

FILE *file;
int fd;
struct lecture rec;
struct flock lock;
char file_name[50]; // 파일 생성 시
char call_file[20]; // 파일 불러올 때 
static int check = 0; // 강좌정보 생성 시 갯수 체크 

void lecture_manage() {
    int n = -1;

    while(n != 0) {
        printf("1.파일생성  2.강좌목록 보기  3.신규 강좌생성  4.강좌정보 변경  5 강좌정보 검색  6.강좌정보 삭제  0.종료\n");
        printf("메뉴를 선택하세요: ");
        scanf("%d", &n);

        switch(n) {
            case 1:
                create_lec_file();
                break;
            case 2:
                sort_lec();
                break;
            case 3:
                add_lec();
                break;
            case 4:
                change_lec();
                break;
            case 5:
                search_lec();
                break;
            case 6:
                delete_lec();
                break;
            case 0:
                return;
        }
    }
}

// 파일 생성
void create_lec_file() {
    printf("생성할 파일명을 입력하세요: ");
    scanf("%s", file_name);

    // O_RDWR는 읽기, 쓰기 모드
    // O_CREAT를 해줘야 파일이 없으면 새로 생성한다.
    // 0666은 소유자, 그룹, 기타에게 읽기, 쓰기 권한 부여
    if((fd = open(file_name, O_RDWR | O_CREAT, 0666)) == -1) {
        printf("%s 파일 생성 실패\n\n", file_name);
    }
    else {
        printf("%s 파일 생성 완료\n\n", file_name);
        close(fd);
    }    
}

// 강좌 목록 보기(강좌명 정렬)
void sort_lec() {
    struct lecture lec[100]; // 정렬할 때 입력받을 구조체 배열 변수
    int count = 0; // 강좌 정보 갯수 파악
    
    printf("불러올 파일명을 입력하세요: ");
    scanf(" %s", call_file);
    if((fd = open(call_file, O_RDONLY)) == -1) {
        perror(call_file);
        exit(1);
    }

    lock.l_type = F_RDLCK; // 읽기 잠금
    lock.l_whence = SEEK_SET; // 파일 시작을 기준으로
    lock.l_start = 0; // 처음부터
    lock.l_len = 0; // 전체 잠금

    // F_SETLKW: file set lock wait, 
    //           잠금이 해제될 때까지 일정시간동안 기다린다.
    if(fcntl(fd, F_SETLKW, &lock) == -1) { 
        perror(call_file);
        exit(2);
    }

    // 파일에서 강좌 정보를 읽어 배열에 저장
    while (read(fd, &rec, sizeof(rec)) > 0) {
        if (rec.id != -1) { // 삭제된 강좌 제외
            lec[count++] = rec;
        }
    }
    
    // 강좌명으로 정렬
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (strcmp(lec[j].name, lec[j + 1].name) > 0) {
                struct lecture temp = lec[j];
                lec[j] = lec[j + 1];
                lec[j + 1] = temp;
            }
        }
    }

    // 정렬된 강좌 목록 출력
    printf("정렬된 강좌 목록:\n");
    for (int i = 0; i < count; i++) {
        printf("강좌번호: %d, 강좌명: %s, 교수명: %s, 개설학과명: %s, 최대수강인원: %d, 비고: %s\n",
               lec[i].id, lec[i].name, lec[i].p_name, lec[i].d_name, lec[i].max_p, lec[i].note);
    }
    printf("\n");

    lock.l_type = F_UNLCK; 
    fcntl(fd, F_SETLK, &lock);

    close(fd);    
}

// 신규 강좌 생성
void add_lec() {
    if(check == MAX_LEC) {
        printf("강좌 정보를 더이상 추가할 수 없습니다.\n\n");
        return;
    }

    printf("불러올 파일명을 입력하세요: ");
    scanf(" %s", call_file);
    
    // O_APPEND: 파일 끝에 데이터가 첨부되어 기존 내용이 삭제되지 않음
    if((fd = open(call_file, O_RDWR | O_APPEND)) == -1) {
        perror(call_file);
        exit(1);
    }

    printf("생성할 강좌번호를 입력하세요.\n");
    printf("강좌번호: "); 
    if(scanf("%d", &rec.id) == 1) {
        lock.l_type = F_WRLCK; // 쓰기 잠금
        lock.l_whence = SEEK_SET; // 파일 시작을 기준으로
        lock.l_start = 0; // 시작위치에서
        lock.l_len = 0; // 전체 잠금

        if(fcntl(fd, F_SETLKW, &lock) == -1) {
            perror(call_file);
            close(fd);
            exit(2);
        }

        // %[^\n]: "줄 바꿈 문자를 제외한 모든 문자"를 읽어들이라는 의미
        printf("강좌명: "); scanf(" %[^\n]", rec.name);
        printf("교수명: "); scanf(" %[^\n]", rec.p_name);
        printf("개설학과명: "); scanf(" %[^\n]", rec.d_name);
        printf("최대수강인원: "); scanf("%d", &rec.max_p);
        printf("비고: "); scanf(" %[^\n]", rec.note);

        write(fd, (char *)&rec, sizeof(rec));
        printf("강좌정보가 추가되었습니다.\n\n");
        check++;

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    } 

    close(fd);
}

// 강좌정보 변경
void change_lec() {
    int id;
    int found = 0;
    
    printf("불러올 파일명을 입력하세요: ");
    scanf(" %s", call_file);
    if((fd = open(call_file, O_RDWR)) == -1) {
        perror(call_file);
        exit(1);
    }

    printf("정보를 변경할 강좌 번호를 입력하세요: ");
    if(scanf("%d", &id) == 1) {
        while((read(fd, &rec, sizeof(rec)) > 0) && rec.id != -1) {
            if(rec.id == id) {
                found = 1;

                lock.l_type = F_WRLCK; // 쓰기잠금
                lock.l_whence = SEEK_SET; // 파일시작을 기준으로
                lock.l_start = id*sizeof(rec); 
                lock.l_len = sizeof(rec);

                if(fcntl(fd, F_SETLKW, &lock) == -1) {
                    perror(file_name);
                    exit(2);
                }

                printf("현재 강좌 정보\n");
                printf("강좌번호: %d, 강좌명: %s, 교수명: %s, 개설학과명: %s, 최대수강인원: %d, 비고: %s\n\n", rec.id, rec.name, rec.p_name, rec.d_name, rec.max_p, rec.note);

                printf("수정할 강좌 정보를 입력하세요.\n");
                printf("강좌번호: "); scanf("%d", &rec.id);
                printf("강좌명: "); scanf(" %[^\n]", rec.name);
                printf("교수명: "); scanf(" %[^\n]", rec.p_name);
                printf("개설학과명: "); scanf(" %[^\n]", rec.d_name);
                printf("최대수강인원: "); scanf("%d", &rec.max_p);
                printf("비고: "); scanf(" %[^\n]", rec.note);
                
                lseek(fd, (long)-sizeof(rec), SEEK_CUR);
                write(fd, (char *)&rec, sizeof(rec));
                printf("강좌 정보가 수정되었습니다.\n\n");

                lock.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &lock);
                break;
            }
        }
        if(!found) {
            printf("강좌번호 %d 없음\n\n", id);
        }
    }
    close(fd);
}

// 강좌정보 검색
void search_lec() {
    char name[20];
    int found = 0;

    printf("불러올 파일명을 입력하세요: ");
    scanf(" %s", call_file);
    if((fd = open(call_file, O_RDWR)) == -1) {
        perror(call_file);
        exit(1);
    }

    printf("검색할 강좌명을 입력하세요: ");
    scanf(" %[^\n]", name);

    while(read(fd, &rec, sizeof(rec)) > 0) {
        if(rec.id != -1 && strcmp(rec.name, name) == 0) {
            printf("강좌번호: %d, 강좌명: %s, 교수명: %s, 개설학과명: %s, 최대수강인원: %d, 비고: %s\n\n", 
                   rec.id, rec.name, rec.p_name, rec.d_name, rec.max_p, rec.note);
            found++;
        }
    }

    if(!found) {
        printf("강좌명 %s은 없습니다.\n\n", name);
    }

    close(fd);
}

// 강좌정보 삭제
void delete_lec() {
    int id;
    int found = 0;

    printf("불러올 파일명을 입력하세요: ");
    scanf(" %s", call_file);
    if((fd = open(call_file, O_RDWR)) == -1) {
        perror(call_file);
        exit(1);
    }

    printf("삭제할 강좌번호를 입력하세요: ");
    if(scanf("%d", &id) == 1) {
        while(read(fd, &rec, sizeof(rec)) > 0) {
            if(rec.id == id) {
                found = 1;
            
                lock.l_type = F_WRLCK; // 쓰기잠금
                lock.l_whence = SEEK_SET; // 파일시작을 기준으로
                lock.l_start = id*sizeof(rec); 
                lock.l_len = sizeof(rec);

                if(fcntl(fd, F_SETLKW, &lock) == -1) {
                    perror(file_name);
                    exit(2);
                }

                rec.id = -1; // 삭제된 강좌 번호는 -1로 설정

                lseek(fd, (long)-sizeof(rec), SEEK_CUR);
                write(fd, (char *)&rec, sizeof(rec));
                printf("강좌 정보가 삭제되었습니다.\n\n");
                check--;

                lock.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &lock);
                break;
            }
        }
        if(!found) {
            printf("강좌번호 %d 없음\n\n", id);
        }
    }
    close(fd);    
}