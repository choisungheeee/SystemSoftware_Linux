#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include "student.h"

// 학생 관리 함수
void student_manage();
void create_file();    // 파일 생성
void add_student();    // 학생 정보 추가
void update_student(); // 학생 정보 수정(학번)
void delete_student(); // 학생 정보 삭제(학번)
void search_student(); // 학생 정보 검색(이름)

// 디렉토리 목록 확인 함수
void directory_list();
char type(mode_t);  // 파일 타입
char *perm(mode_t); // 파일 사용권한
void printStat(char*, char*, struct stat*); // 파일 상태 정보

FILE *file;
struct student rec;
char file_name[50];

int main() {
    int n = -1;

    while(n != 0) {
        printf("1.파일기반 학생점수 관리   2.파일/디렉토리 관리   0.종료\n");
        printf("메뉴를 선택하세요: ");
        scanf("%d", &n);
        printf("\n");

        switch(n) {
            case 1:
                student_manage();
                break;
            case 2:
                directory_list();
                break;
            case 0:
                return;
        }
    }
    
}

// 학생 관리
void student_manage() {
    int n = -1;

    while(n != 0) {
        printf("1.파일생성  2.학생정보추가  3.수정  4.삭제  5.검색  0.종료\n");
        printf("메뉴를 선택하세요: ");
        scanf("%d", &n);
        printf("\n");

        switch(n) {
            case 1:
                create_file();
                break;
            case 2:
                add_student();
                break;
            case 3:
                update_student();
                break;
            case 4:
                delete_student();
                break;
            case 5:
                search_student();
                break;
            case 0:
                return 0;
        }
    } 
}

// 파일 생성
void create_file() {
    printf("파일명을 입력하세요: ");
    scanf(" %s", file_name);

    file = fopen(file_name, "wb");

    if(file == NULL) {
        printf("%s 파일 생성 실패\n\n", file_name);
    }
    else {
        printf("%s 파일 생성 완료\n\n", file_name);
        fclose(file);
    }    
}

// 학생정보 추가
void add_student() {
    file = fopen(file_name, "ab"); // ab: 추가 쓰기 모드

    if(file == NULL) {
        printf("파일이 없습니다.\n\n");
        return;
    }

    printf("%s %3s %7s %s %s\n", "학번", "이름", "과목명", "점수", "보정점수");

    if(scanf("%d %s %s %d %f", &rec.id, rec.name, rec.sub, &rec.score, &rec.mod) == 5) {
        fwrite(&rec, sizeof(rec), 1, file);
    }

    fclose(file);

    printf("\n");
}

// 학생정보 수정
void update_student() {
    int id; // 학번 검색 받을 변수
    int found = 0; // 학생 존재 여부 확인 변수

    file = fopen(file_name, "rb+");

    if(file == NULL) {
        printf("파일이 없습니다.\n\n");
        return;
    }

    printf("수정할 학생의 학번을 입력하세요: ");

    if(scanf("%d", &id) == 1) {
        while(fread(&rec, sizeof(rec), 1, file) == 1) {
            if(rec.id == id) {
                found = 1;
                printf("현재 학생 정보\n");
                printf("학번: %d  이름: %s  과목명: %s  점수: %d  보정점수: %.2f\n\n", rec.id, rec.name, rec.sub, rec.score, rec.mod);

                printf("수정할 학생 정보를 입력하세요.\n");
                if(scanf("%d %s %s %d %f", &rec.id, rec.name, rec.sub, &rec.score, &rec.mod) == 5) {
                    fseek(file, -sizeof(rec), SEEK_CUR);
                    fwrite(&rec, sizeof(rec), 1, file);
                    printf("학생 정보가 수정되었습니다.\n\n");
                } else {
                    printf("입력 오류\n\n");
                }
                break; 
            }
        }

        if (!found) {
            printf("학번 %d 없음\n\n", id);
        }

    fclose(file);
    }
}

// 학생정보 삭제
void delete_student() {
    int id; // 학번 검색 받을 변수
    int found = 0; // 학생 존재 여부 확인 변수

    file = fopen(file_name, "rb+");
    if(file == NULL) {
        printf("파일이 없습니다.\n\n");
        return;
    }

    printf("삭제할 학생의 학번을 입력하세요: ");
    if(scanf("%d", &id) == 1) {
        while(fread(&rec, sizeof(rec), 1, file) == 1) {
            if(rec.id == id) { 
                found = 1;
                rec.id = -1; // 삭제하는 학생의 학번은 -1로 변경
                fseek(file, -sizeof(rec), SEEK_CUR);
                fwrite(&rec, sizeof(rec), 1, file);
                printf("학생 정보가 삭제되었습니다.\n\n");
                break; 
            }
        }
        if (!found) {
            printf("학번 %d 없음\n\n", id);
        }
    }

    fclose(file);
}

// 학생정보 검색
void search_student() {
    char name[20]; // 이름 검색 받을 배열 변수
    int found = 0; // 학생 존재 여부 확인 변수

    printf("검색할 학생의 이름을 입력하세요: ");
    scanf("%s", name);

    file = fopen(file_name, "rb");
    
    if(file == NULL) {
        printf("파일이 없습니다.\n\n");
        return;
    }

    while(fread(&rec, sizeof(rec), 1, file) == 1) {
        if((strcmp(name, rec.name) == 0) && rec.id != -1) {
            printf("학번: %2d  이름: %s  과목명: %s  점수: %3d  보정점수:%.2f\n\n", rec.id, rec.name, rec.sub, rec.score, rec.mod);
            found = 1;
            break;
        }
    }

    if(!found) {
        printf("이름이 %s인 학생은 없습니다.\n\n", name);
    }

    fclose(file);
}

// 디렉토리 목록 확인
void directory_list() {
    DIR *dp;
    char dir_name[BUFSIZ]; // 디렉토리 이름 검색 받을 배열 변수
    struct stat st;
    struct dirent *d;
    char path[BUFSIZ + 1];

    printf("확인하고싶은 디렉토리 이름을 입력하세요: ");
    scanf("%s", dir_name);

    // 디렉토리 열기
    if((dp = opendir(dir_name)) == NULL) {
        perror(dir_name);
    }

    // 디렉토리 내 파일/디렉토리 목록 읽기
    while((d = readdir(dp)) != NULL) {
        // 문자열을 path 배열변수에 저장하여 파일 경로명 만들기
        sprintf(path, "%s/%s", dir_name, d->d_name); 

        // lstat 호출 시 파일 경로 확인
        if(lstat(path, &st) < 0) {
            perror(path);
        }
        else {
            printStat(path, d->d_name, &st);
            printf("\n");
        }        
    }
    printf("\n");
    closedir(dp);
}

// 파일 상태 정보 출력
void printStat(char *pathname, char *file, struct stat *st) {
    printf("%5d ", st->st_blocks); // 파일의 블록 수
    printf("%c%s ", type(st->st_mode), perm(st->st_mode)); // 접근 권한
    printf("%3d ", st->st_nlink); // 링크 수
    printf("%s %s ", getpwuid(st->st_uid)->pw_name, getgrgid(st->st_gid)->gr_name); // 소유자이름, 그룹이름
    printf("%9d ", st->st_size); // 파일 크기
    printf("%.12s ", ctime(&st->st_mtime) + 4); // 수정시간
    printf("%s", file); // 파일 이름
}

// 파일 타입 리턴
char type(mode_t mode) {
    if(S_ISREG(mode)) {
        return('-');
    }
    if(S_ISDIR(mode)) {
        return('d');
    }
    if(S_ISCHR(mode)) {
        return('c');
    }
    if(S_ISBLK(mode)) {
        return('b');
    }
    if(S_ISLNK(mode)) {
        return('I');
    }
    if(S_ISFIFO(mode)) {
        return('p');
    }
    if(S_ISSOCK(mode)) {
        return('s');
    }
}

// 파일 사용 권한 리턴
char* perm(mode_t mode) {
    static char perms[10];
    strcpy(perms, "---------");

    for(int i = 0; i < 3; i++) {
        if(mode & (S_IRUSR >> i*3)) {
            perms[i*3] = 'r';
        }
        if(mode & (S_IWUSR >> i*3)) {
            perms[i*3 + 1] = 'w';
        }
        if(mode & (S_IXUSR >> i*3)) {
            perms[i*3 + 2] = 'x';
        }
    }

    return(perms);
}