#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>

// 시스템소프트웨어
void login();
void admin_login();
void user_add();
static void exit_handle(void);

// 관리자 - 사용자 관리
void list();
void create();
void update();
void search();
void delete();

typedef struct {
    int id;         // 식별자 아이디
    char email[20]; // 이메일(형식 체크)
    char pw[20];    // 비밀번호
    char name[20];  // 이름(공백 가능)
    char addr[50];  // 주소(공백 가능)
    int birth;      // 출생년도
    int admin;      // 관리자 0, 일반사용자 1
    time_t last;    // 최근 로그인 일시
    int count;      // 로그인 횟수
} User;

User users;
struct flock lock;
int fd;

void error(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char email[20], pw[20], buffer[256];

    // 매개변수 확인
    if(argc < 3) {
        fprintf(stderr, "Usage %s hostname port\n", argv[0]);
        exit(1);
    }

    // 포트번호 int형으로 변환
    portno = atoi(argv[2]);

    // 소켓 생성
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        error("ERROR opening socket");
    }

    // 호스트 주소를 IP 주소로 변환
    server = gethostbyname(argv[1]);
    if(server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(1);
    }

    // 서버 소켓 세팅
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    // 연결 요청
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }    
    
    bzero(buffer, sizeof(buffer));
    n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n < 0) error("ERROR reading from socket");        
    printf("%s", buffer); // 서버 메시지 출력

    int m = -1;
    while(m != 0) {
        printf("<시스템소프트웨어>\n\n");
        printf("1. 로그인\n");
        printf("2. 관리자 로그인\n");
        printf("3. 사용자 추가\n");
        printf("0. 종료\n\n");
        printf("메뉴를 선택하세요: "); scanf("%d", &m);

        switch(m) {
            case 1: login(); break;         // 로그인
            case 2: admin_login(); break;   // 관리자 로그인
            case 3: user_add(); break;      // 사용자 추가
            case 0: 
                if(atexit(exit_handle) != 0) {
                        perror("exit_handle 등록할 수 없음\n\n");
                }
                return;
        }
    }

    close(sockfd);
    return 0;
}

// 1. 로그인
void login() {
    char email[20];
    char pw[20];

    // 사용자 관리 파일 열기
    if((fd = open("user.txt", O_RDWR | O_CREAT, 0666)) == -1) {
        perror("user.txt");
        exit(1);
    }

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
                close(fd); return;
            }
        }
    }
    if(!check) {
        time_t now = time(NULL); // 현재 시간을 가져옴
        users.last = now;        // 최근 로그인 시간 업데이트
        users.count += 1;        // 로그인 횟수 증가
        // 파일 업데이트
        lseek(fd, -sizeof(users), SEEK_CUR);
        if (write(fd, &users, sizeof(users)) == -1) {
            perror("\n사용자 정보 업데이트 실패\n");
            close(fd); return;
        }

        printf("일반 사용자 로그인 화면입니다.\n\n");
        printf("일반 사용자 %s님이 입장하셨습니다.\n\n", users.email);
        
        int n;
        printf("0. 로그아웃\n선택: "); scanf(" %d", &n);
        if(!n) close(fd); return;
    }
    close(fd);
}

// 2. 관리자 로그인
void admin_login() {
    char email[20];
    char pw[20];

    // 사용자 관리 파일 열기
    if((fd = open("user.txt", O_RDWR | O_CREAT, 0666)) == -1) {
        perror("user.txt");
        exit(1);
    }

    printf("\n--- 로그인 화면입니다. ---\n");    
    printf("사용자 ID(email)와 Password를 입력하세요.\n\n");    
    
    int check = 1;
    while(check == 1) {
        printf("ID(email): "); scanf("%s", email);
        printf("Password: "); scanf("%s", pw);

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
                close(fd); return;
            }
        }
    }
    if(!check) {
        time_t now = time(NULL); // 현재 시간을 가져옴
        users.last = now;        // 최근 로그인 시간 업데이트
        users.count += 1;        // 로그인 횟수 증가
        // 파일 업데이트
        lseek(fd, -sizeof(users), SEEK_CUR);
        if (write(fd, &users, sizeof(users)) == -1) {
            perror("\n사용자 정보 업데이트 실패\n");
            close(fd); return;
        }
        
        int n = -1;

        while(n != 0) {
            printf("1. 사용자 목록 보기\n");
            printf("2. 신규 사용자 생성\n");
            printf("3. 사용자 정보 변경\n");
            printf("4. 사용자 정보 검색(Email 검색)\n");
            printf("5. 사용자 삭제\n");
            printf("0. 로그아웃\n\n");

            printf("메뉴를 선택하세요: "); scanf("%d", &n);

            switch(n) {
                case 1: list(); break;
                case 2: user_add(); break;
                case 3: update(); break;
                case 4: search(); break;
                case 5: delete(); break;
                case 0: close(fd); return;
            }
        }
    }
    close(fd);
}

// 2-1. 사용자 목록 보기
void list() {
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if(fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("\n레코드 잠금 실패\n\n");
        exit(1);
    }

    lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동
    while((read(fd, &users, sizeof(users)) > 0) && (users.id != -1)) {
        printf("식별자 아이디: %d, 사용자 아이디(Email): %s, 비밀번호: %s, ", users.id, users.email, users.pw);
        printf("이름: %s, 주소: %s, 출생년도: %d, ", users.name, users.addr, users.birth);
        printf("관리자 여부(관리자 0 / 일반사용자 1): %d, ", users.admin);
        printf("최근 로그인 일시: %s", ctime(&users.last));
        printf("로그인 횟수: %d\n", users.count);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
}

// 2-3. 사용자 정보 변경
void update() {
    int id;

    printf("식별자 번호 입력: "); scanf(" %d", &id);
    lseek(fd, id*sizeof(users), SEEK_SET);
    if((read(fd, &users, sizeof(users)) > 0) && (users.id != -1)) {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = users.id*sizeof(users);
        lock.l_len = sizeof(users);
        if(fcntl(fd, F_SETLKW, &lock) == -1) {
            perror("\n레코드 잠금 실패\n\n");
            exit(1);
        }

        printf("\n현재 사용자 정보\n");
        printf("식별자 아이디: %d, 사용자 아이디(Email): %s, 비밀번호: %s, ", users.id, users.email, users.pw);
        printf("이름: %s, 주소: %s, 출생년도: %d, ", users.name, users.addr, users.birth);

        printf("수정할 사용자 정보를 입력하세요.\n\n");
        printf("UserID(email): "); scanf(" %s", users.email);
        printf("Password: "); scanf(" %s", users.pw);
        printf("식별자 아이디: "); scanf(" %d", &users.id);
        printf("이름(공백가능): "); scanf(" %[^\n]", users.name);
        printf("주소(공백가능): "); scanf(" %[^\n]", users.addr);
        printf("출생년도: "); scanf(" %d", &users.birth);

        lseek(fd, -sizeof(users), SEEK_CUR);
        if(write(fd, &users, sizeof(users)) == -1) {
            printf("\n사용자 정보 수정 실패\n\n");
            return;   
        }
        printf("\n사용자 정보 수정 완료\n\n");

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }
}

// 2-4. 사용자 정보 검색(Email 검색)
void search() {
    int found = 0;
    char email[20];

    printf("검색할 사용자ID(email) 입력: "); scanf(" %s", email);
    lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동
    while((read(fd, &users, sizeof(users)) > 0) && (users.id != -1)) {
        if(strstr(users.email, email) != NULL) {
            found += 1;

            printf("식별자 아이디: %d, 사용자 아이디(Email): %s, 비밀번호: %s, ", users.id, users.email, users.pw);
            printf("이름: %s, 주소: %s, 출생년도: %d, ", users.name, users.addr, users.birth);
            printf("관리자 여부(관리자 0 / 일반사용자 1): %d, ", users.admin);
            printf("최근 로그인 일시: %s", ctime(&users.last));
            printf("로그인 횟수: %d\n", users.count);
        }
    }

    if(!found) {
        printf("\n검색한 사용자 정보가 없습니다.\n\n");
        return;
    }
}

// 2-5. 사용자 삭제
void delete() {
    int found = 0;
    char email[20];

    printf("삭제할 사용자ID(email) 입력: "); scanf(" %s", &email);
    lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동
    while((read(fd, &users, sizeof(users)) > 0) && (users.id != -1)) {
        if(strcmp(users.email, email) == 0) {
            found = 1;

            users.id = -1;

            lseek(fd, -sizeof(users), SEEK_CUR);
            if(write(fd, &users, sizeof(users)) == -1) {
                printf("사용자 정보 삭제 실패\n\n");
                return;   
            }
            printf("사용자 정보 삭제 완료\n\n");        
            break;
        }
    }
    if(!found) {
        printf("\n검색한 사용자 정보가 없습니다.\n\n");
        return;
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
}

// 3. 사용자 추가
void user_add() {
    // 사용자 관리 파일 열기
    if((fd = open("user.txt", O_RDWR | O_CREAT, 0666)) == -1) {
        perror("user.txt");
        exit(1);
    }

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if(fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("\n레코드 잠금 실패\n\n");
        close(fd); exit(1);
    }

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
            if(strcmp(users.email, email) == NULL) {
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
            printf("주소(공백가능): "); scanf(" %[^\n]", users.addr);
            printf("출생년도: "); scanf(" %d", &users.birth);
            printf("관리자(0) / 일반사용자(1): "); scanf(" %d", &users.admin);
            users.last = users.count = 0; // 초기화

            lseek(fd, users.id*sizeof(users), SEEK_SET);
            if(write(fd, &users, sizeof(users)) == -1) {
                printf("\n사용자 추가 실패\n\n");
                close(fd); return;   
            }
            printf("\n사용자 추가 완료\n\n");
        }
    } 
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    close(fd);
}

// 0. 종료
static void exit_handle(void) {
    printf("sunghee님이 구현한 시스템소프트웨어 데모 시스템을 이용해 주셔서 감사합니다.\n\n");
}