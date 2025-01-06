#include<stdio.h>
#include<stdlib.h>

static name[50];
static void exit_handler(void);

int main(int argc, char *argv[]) {
    int n = -1;
    char **p;
    extern char **environ;

    if (argc > 1) {
        strncpy(name, argv[1], sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
    } else {
        strcpy(name, "사용자");  // 기본값 설정
    }

    while(n != 0) {
        printf("1.Print all current environ\n2.Print specific environ(argv[2])\n3.Print pid, ppid\n0.종료\n");
        printf("메뉴를 입력하세요: ");
        scanf("%d", &n);

        if(n == 1) {
            for(p=environ; *p != 0; p++) {
                printf("%s \n", *p);
            }
            printf("\n\n");
        }
        else if(n == 2) {
            p = getenv(argv[2]);

            printf("%s = %s \n", argv[2], p);

            printf("\n\n");
        }
        else if(n == 3) {
            printf("나의 프로세스 번호: [%d] \n", getpid());
            printf("내 부모 프로세스 번호: [%d] \n", getppid());

            printf("\n\n");
        }
        else if(n == 0) {
            if(atexit(exit_handler) != 0) {
                perror("exit 등록할 수 없음");
            }
        }
    }
    exit(0);
}

static void exit_handler(void) {
    printf("%s님이 프로그램을 종료했습니다!\n", name);
}