#include<stdio.h>
#include<stdlib.h>
#include"student.h"

void create_file();
void add_student();
void update_student();
void delete_student();
void search_student();

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

// 점수파일 생성
void create_file() {
    printf("파일명을 입력하세요: ");
    scanf(" %s", file_name);

    file = fopen(file_name, "wb");

    if(file == NULL) {
        printf("%s 파일 생성 실패\n\n", file_name);
    }
    else {
        printf("%s 파일 생성 완료\n\n", file_name);
    }
    fclose(file);
}

// 신규 학생정보 추가
void add_student() {
    file = fopen(file_name, "ab");

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
    int id;
    int found = 0;

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
    int id;
    int found = 0;

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
                rec.id = -1;
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

// 학생 정보 검색
void search_student() {
    char name[20];
    int found = 0;

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
