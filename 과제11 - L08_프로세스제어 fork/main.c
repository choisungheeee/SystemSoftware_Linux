#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include<fcntl.h>

typedef struct {
    int id;             // 식별자 아이디
    char email[20];     // 이메일(형식체크)
    char pw[20];        // 비밀번호
    char name[20];      // 이름(공백가능)
    char address[50];   // 주소(공백가능)
    int birth;          // 출생년도
    int admin;          // 관리자
    time_t last;        // 최근 로그인 일시
    int count;          // 로그인 횟수
} User;

int fd;
User users;

// 1. 로그인
void login() {
    char email[20];
    char pw[20];

    printf("\n--- 로그인 화면입니다. ---\n");    
    printf("사용자 ID(email)와 Password를 입력하세요.\n\n");    
    
    int check = 1;
    while(check == 1) {
        printf("ID(email): "); scanf(" %s", email);
        printf("Password: "); scanf(" %s", pw);

        lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동

        while(read(fd, &users, sizeof(users)) > 0) {
            if(strcmp(email, users.email) == 0) {
                if(strcmp(pw, users.pw) == 0) {
                    printf("\n로그인합니다.\n\n");
                    check = 0;
                    break;
                }
                else {
                    printf("\n비밀번호가 틀렸습니다. 다시 입력하세요.\n\n");
                    break;
                }
            }
            else {
                printf("\n입력한 사용자 ID(email)에 대한 정보가 없습니다.\n\n");
                return;
            }
        }
    }
    lecture();
}

// 2. 사용자 추가
void add_user() {
    char email[20];
    
    printf("\n--- 사용자 추가 화면입니다. ---\n\n");    

    int check = 1;
    while(check == 1) {
        printf("UserID(email): "); scanf(" %s", email);

        // 이메일 형식 체크    
        if((strstr(email, "@") == NULL) || (strstr(email, ".") == NULL)) {
            printf("\n=> 이메일 형식이 잘못되었습니다.\n\n");
            continue;
        }

        // 이메일 중복 체크
        int found = 0;
        lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동
        while(read(fd, &users, sizeof(users)) > 0) {
            if(strstr(users.email, email) == NULL) {
                printf("\n=> 이미 있는 사용자입니다.\n\n");
                found = 1;
                break;
            }
        }

        if(!found) {
            check = 0;
            printf("\n=> 새로운 사용자 ID(email)를 만듭니다.\n\n");
            strncpy(users.email, email, sizeof(email));
            printf("Password: "); scanf(" %s", users.pw);
            printf("식별자 아이디: "); scanf(" %d", &users.id);
            printf("이름(공백가능): "); scanf(" %[^\n]", users.name);
            printf("주소(공백가능): "); scanf(" %[^\n]", users.address);
            printf("출생년도: "); scanf(" %d", &users.birth);
            printf("관리자(0) / 일반사용자(1): "); scanf(" %d", &users.admin);
            users.last = users.count = 0; // 초기화

            lseek(fd, users.id*sizeof(users), SEEK_SET);
            if(write(fd, &users, sizeof(users)) == -1) {
                printf("\n사용자 추가 실패\n\n");
                return;   
            }
            printf("\n사용자 추가 완료\n\n");
        }
    } 
    login();
}

// 0. 종료
static void exit_handle(void) {
    printf("sunghee님이 구현한 시스템소프트웨어 데모 시스템을 이용해 주셔서 감사합니다.\n\n");
}

void main() {
    if((fd = open("user", O_RDWR | O_CREAT, 0666)) == -1) {
        perror("user");
        exit(1);
    }

    int n = -1;

    while(n != 0) {
        printf("1. 로그인\n");
        printf("2. 사용자 추가\n");
        printf("0. 종료\n\n");
        printf("메뉴를 선택하세요: "); scanf("%d", &n);

        switch(n) {
            case 1: login(); break;
            case 2: add_user(); break;
            case 0: 
                if(atexit(exit_handle) != 0) {
                    perror("exit_handle 등록할 수 없음\n\n");
                }
                close(fd); 
                return;
        }
    }
}