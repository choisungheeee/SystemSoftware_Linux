#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<fcntl.h>
#include<unistd.h>

typedef struct {
    int id;         // 아이디
    char email[50]; // 이메일
    char pw[20];    // 비밀번호
    char name[20];  // 이름(공백 가능)
    char address[20];// 주소(공백 가능)
    int birth;      // 출생년도
    int admin;      // 0: 관리자, 1: 일반사용자
    time_t time;    // 최근 로그인 일시
    int count;      // 로그인 횟수
} User;

int fd;
User users;

// 1. 로그인
void login() {
    int check = 1;
    char email[20];
    char pw[20];

    printf("로그인 화면입니다.\n사용자 ID와 Password를 입력하세요.\n\n");

    while(check == 1) {        
        printf("ID: "); scanf(" %s", email);
        printf("Password: "); scanf(" %s", pw);

        lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동

        while(read(fd, &users, sizeof(users)) > 0) {
            if(strcmp(email, users.email) == 0) {
                if(strcmp(pw, users.pw) == 0) {
                    printf("로그인합니다.\n\n");
                    check = 0;
                    break;
                }
                else {
                    printf("비밀번호가 틀렸습니다. 다시 입력하세요.\n\n");
                    break;
                }
            }
            else {
                printf("사용자 ID가 틀렸습니다. 다시 입력하세요.\n\n");
                break;
            }
        }
    }    
    manage();
}

// 2. 사용자 추가
void add() {
    if((fd = open("userfile", O_RDWR | O_CREAT, 0666)) == -1) {
        perror("파일 생성 실패\n\n");
        exit(1);
    }

    char new[20];
    printf("사용자 추가 화면입니다.\n\n");
    printf("Userid(email) : "); scanf(" %s", new);

    // 이메일 형식 체크
    if((strstr(new, "@") == NULL) || (strstr(new, ".") == NULL)) {
        printf("이메일 형식에 맞지 않습니다.\n\n");
        return;
    }

    // 사용자 ID 중복 체크
    lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동
    while(read(fd, &users, sizeof(users)) > 0) {
        if (strcmp(users.email, new) == 0) {
            printf("=> 이미 있는 사용자 입니다.\n\n");
            return;
        }
    }

    strncpy(users.email, new, sizeof(new));

    // 새로운 사용자 정보 입력
    printf("=> 새로운 사용자 id를 만듭니다.\n");
    printf("아이디: "); scanf(" %d", &users.id);
    printf("비밀번호: "); scanf(" %s", users.pw);
    printf("이름: "); scanf(" %[^\n]", users.name);
    printf("주소: "); scanf(" %[^\n]", users.address);
    printf("출생년도: "); scanf(" %d", &users.birth);
    printf("관리자 여부(0: 관리자, 1: 일반사용자): "); scanf(" %d", &users.admin);
    users.time = 0; // 초기값
    users.count = 0;    

    // 사용자 추가
    lseek(fd, users.id*sizeof(users), SEEK_SET);
    if(write(fd, &users, sizeof(users)) != -1) {
        printf("새로운 사용자 추가 완료\n\n");
    }
    else {
        printf("새로운 사용자 추가 실패\n\n");
    }
}

static void exit_handle(void) {
    printf("sunghee님이 구현한 시스템소프트웨어 데모 시스템을\n");
    printf("이용해 주셔서 감사합니다.\n\n");
}

void main() {
    int n = -1;

    while(n != 0) {
        printf("\n--- 로그인/사용자 추가 프로그램입니다. ---\n\n");
        printf("1. 로그인\n");
        printf("2. 사용자 추가\n");
        printf("0. 종료\n");
        printf("\n메뉴를 선택하세요: "); scanf("%d", &n);

        switch(n) {
            case 1: login(); break;
            case 2: add(); break;
            case 0: 
                if(atexit(exit_handle) != 0) {
                    perror("exit_handle 등록할 수 없음\n\n");
                }
                exit(0);
        }
    }
}