#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STUDENTS 100
#define MAX_FILENAME 100

typedef struct {
    int id; // 학번. 0부터 시작
    char name[20]; // 이름
    char sub[25]; // 과목명
    int score; // 점수
    float mod; // 보정점수
} Student;

Student students[MAX_STUDENTS]; // 학생 정보 저장할 구조체 배열 변수
int student_count = 0; // 학생 인원 체크
char filename[MAX_FILENAME]; // 파일 이름 저장할 변수


// 파일 생성
void create_file() {
    printf("파일명을 입력하세요: ");
    scanf("%s", filename);
    FILE* file = fopen(filename, "w");
    if (file) {
        fclose(file);
        printf("%s 파일이 생성되었습니다\n\n", filename);
    }
    else printf("파일 생성 실패\n\n");
}

// 학생 정보 추가
void add_student() {
    if (student_count >= MAX_STUDENTS) {
        printf("학생 정보를 더 이상 추가할 수 없습니다.\n\n");
        return;
    }
    Student new_student; // 새로운 구조체 변수 생성
    printf("학번: ");
    scanf("%d", &new_student.id);
    printf("이름: ");
    scanf("%s", new_student.name);
    printf("과목명: ");
    scanf("%s", new_student.sub);
    printf("점수: ");
    scanf("%d", &new_student.score);
    printf("보정점수: ");
    scanf("%f", &new_student.mod);

    students[student_count++] = new_student;
    save_students(); // 파일에 저장
    printf("학생 정보가 추가되었습니다.\n\n");
}

// 파일에 학생 정보 저장
void save_students() {
    FILE* file = fopen(filename, "w"); // 쓰기 모드로 파일 열기
    if (file) { // 파일이 있다면
        for (int i = 0; i < student_count; i++) { // 학생 정보가 있을 때 까지 조회
            if (students[i].id != -1) { // 삭제된 학생 제외
                fprintf(file, "%2d, %s, %s, %2d, %.2f\n",
                    students[i].id,
                    students[i].name,
                    students[i].sub,
                    students[i].score,
                    students[i].mod);
            }
        }
        fclose(file);
    }
}

// 학생 정보 수정
void modify_student() {
    int id;
    printf("수정할 학생의 학번을 입력하세요: ");
    scanf("%d", &id);

    for (int i = 0; i < student_count; i++) { // 학생 정보가 있을 때 까지 조회
        if (students[i].id == id) {
            printf("현재 정보 - 학번: %d, 이름: %s, 과목명: %s, 점수: %d, 보정점수: %.2f\n\n",
                students[i].id, students[i].name, students[i].sub, students[i].score, students[i].mod);
            printf("수정할 이름: ");
            scanf("%s", students[i].name);
            printf("수정할 과목명: ");
            scanf("%s", students[i].sub);
            printf("수정할 점수: ");
            scanf("%d", &students[i].score);
            printf("수정할 보정점수: ");
            scanf("%f", &students[i].mod);

            // 파일에 수정된 정보 저장
            FILE* file = fopen(filename, "r+");
            if (file) {
                fseek(file, i * sizeof(Student), SEEK_SET);
                fwrite(&students[i], sizeof(Student), 1, file);
                fclose(file);
            }
            printf("학생 정보가 수정되었습니다.\n\n");
            return;
        }
    }
    printf("해당 학번의 학생이 없습니다.\n\n");
}

// 학생 정보 삭제
void delete_student() {
    int id;
    printf("삭제할 학생의 학번: ");
    scanf("%d", &id);

    for (int i = 0; i < student_count; i++) { // 학생 정보가 있을 때 까지 조회
        if (students[i].id == id) {
            students[i].id = -1; // 학번을 -1로 변경
            strcpy(students[i].name, ""); // 이름 공란으로 변경
            strcpy(students[i].sub, ""); // 과목명 공란으로 변경
            students[i].score = 0; // 점수 0으로 변경
            students[i].mod = 0.0; // 보정점수 0으로 변경

            // 파일에 삭제된 정보 저장
            FILE* file = fopen(filename, "r+");
            if (file) {
                fseek(file, i * sizeof(Student), SEEK_SET);
                fwrite(&students[i], sizeof(Student), 1, file);
                fclose(file);
            }
            printf("학생 정보가 삭제되었습니다.\n\n");
            return;
        }
    }
    printf("해당 학번의 학생이 없습니다.\n\n");
}

// 학생 정보 검색
void search_student() {
    char name[20];
    printf("검색할 학생의 이름: ");
    scanf("%s", name);

    int found = 0;
    for (int i = 0; i < student_count; i++) { // 학생 정보가 있을 때 까지 조회
        if (strcmp(students[i].name, name) == 0) {
            printf("학번: %2d, 이름: %s, 과목명: %s, 점수: %3d, 보정점수: %.2f\n\n",
                students[i].id, students[i].name, students[i].sub, students[i].score, students[i].mod);
            found = 1;
        }
    }
    if (!found) {
        printf("해당 이름의 학생이 없습니다.\n\n");
    }
}

int main() {
    int n;

    while (1) {
        printf("0. 종료  1. 파일 생성  2. 학생 정보 추가  3. 학생 정보 수정  4. 학생 정보 삭제  5. 학생 이름 검색\n");
        printf("메뉴를 선택하세요 (0 ~ 5): ");
        scanf("%d", &n);

        switch (n) {
        case 0:
            printf("프로그램을 종료합니다.\n");
            return 0;
        case 1:
            create_file();
            break;
        case 2:
            add_student();
            break;
        case 3:
            modify_student();
            break;
        case 4:
            delete_student();
            break;
        case 5:
            search_student();
            break;
        default:
            printf("메뉴를 다시 선택하세요. (0 ~ 5): \n\n");
            break;
        }
    }

    return 0;
}
