#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>

// 시스템 소프트웨어
void menu(int);             // 메뉴 선택
void login(int);            // 1. 로그인
void general(int, char *);  // 1. 일반 사용자
void admin(int);            // 2. 관리자
void user_add(int);         // 3. 사용자 추가

// 일반사용자 - 강좌 관리
void lec_create(int);   // 1-1. 파일 생성
void lec_list(int);     // 1-2. 강좌 목록 보기
void lec_add(int);      // 1-3. 강좌 정보 생성
void lec_update(int);   // 1-4. 강좌 정보 변경
void lec_search(int);   // 1-5. 강좌 정보 검색
void lec_delete(int);   // 1-6. 강좌 정보 삭제
void lec_store(int);    // 1-7. 데이터 통계 저장

// 관리자 - 사용자 관리
void user_list(int);    // 2-1. 사용자 목록 보기
void user_update(int);  // 2-3. 사용자 정보 변경
void user_search(int);  // 2-4. 사용자 정보 검색
void user_delete(int);  // 2-5. 사용자 정보 삭제

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

typedef struct {
    int id;                 // 강좌번호
    char class[20];         // 강좌명
    char pro[12];           // 교수명
    char department[15];    // 개설학과명
    int max;                // 최대수강인원
    char note[40];          // 비고
} Lecture;

User users;
Lecture lec;
struct flock lock;
int fd, fd2, fd3;
int n;
char num[10];

// 에러 메시지 출력 함수
void error(char * msg) {
    perror(msg);
    exit(1);
}

// 메시지 전송 함수
void send_message(int client_sock, const char *msg) {
    write(client_sock, msg, strlen(msg));
}

// 스레드 사용에 필요한 함수
void* handle_client(void* arg) {
    int client_sock = *((int*)arg);
    free(arg); // 동적 메모리 해제
    menu(client_sock);
    close(client_sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    // 매개변수 확인
    if(argc < 2) {
        fprintf(stderr, "포트번호 누락");
        exit(1);
    }

    // 소켓 생성
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) error("소켓 생성 실패");

    // 서버 소켓 세팅
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // 소켓에 이름(주소) 주기
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("binding 실패");
    }

    // 소켓 큐 생성
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while(1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if(newsockfd < 0) error("accept 실패");

        int* client_sock_ptr = malloc(sizeof(int));
        if(client_sock_ptr == NULL)  {
            perror("메모리 할당 실패");
            close(newsockfd);
            continue;
        }

        *client_sock_ptr = newsockfd;

        pthread_t thread_id;
        if(pthread_create(&thread_id, NULL, handle_client, client_sock_ptr) != 0) {
            perror("스레드 생성 실패");
            free(client_sock_ptr);
            close(newsockfd);
        }

        pthread_detach(thread_id); // 스레드 해제
    }

    close(sockfd);
    return 0;
}

// 메뉴 선택
void menu(int client_sock) {
    char menu[] = "<시스템 소프트웨어>\n\n"
                    "1. 로그인\n"
                    "2. 관리자 로그인\n"
                    "3. 사용자 추가\n"
                    "0. 종료\n"
                    "선택: ";
    int m = -1;
    while(m != 0) {
        write(client_sock, menu, sizeof(menu));

        bzero(num, sizeof(num));
        n = read(client_sock, num, sizeof(num));
        if(n < 0) error("시스템소프트웨어 메뉴 읽기 실패");

        switch(atoi(num)) {
            case 1: login(client_sock); break;
            case 2: login(client_sock); break;
            case 3: user_add(client_sock); break;
            case 0: m = 0; return;
        }
    }
}

// 1. 로그인
void login(int client_sock) {
    char email[20], pw[20];

    send_message(client_sock, "로그인 화면입니다. 사용자 ID와 Password를 입력하세요.\n");

    // 파일 열기
    fd = open("user", O_RDWR, 0666);
    if(fd < 0) error("파일 열기 실패");

    // 클라이언트로부터 로그인 정보 받기
    int check = 1;
    int user_info;

    while(check == 1) {
        bzero(email, sizeof(email)); bzero(pw, sizeof(pw));

        send_message(client_sock, "사용자 ID(email): ");
        n = read(client_sock, email, sizeof(email));
        if(n < 0) error("이메일 읽기 실패");

        send_message(client_sock, "Password: ");
        n = read(client_sock, pw, sizeof(pw));
        if(n < 0) error("비밀번호 읽기 실패");

        lseek(fd, 0, SEEK_SET); // 파일 처음 위치로 이동
        while((read(fd, &users, sizeof(users)) > 0) && (users.id != -1)) {
            if(strcmp(email, users.email) == 0) {
                if(strcmp(pw, users.pw) == 0) {
                    check = 0;
                    user_info = users.admin;

                    send_message(client_sock, "로그인합니다.");

                    // 최근 로그인 일시
                    users.last = time(NULL); // 현재 시간으로 갱신

                    // 로그인 횟수
                    users.count += 1;

                    lseek(fd, -sizeof(users), SEEK_CUR);
                    if(write(fd, &users, sizeof(users)) < 0) {
                        error("최근 로그인 일시, 로그인 횟수 업데이트 실패");
                    }
                }
                else {
                    send_message(client_sock, "비밀번호가 틀렸습니다. 다시 입력하세요.");
                    break;
                }
            }
            else {
                send_message(client_sock, "입력한 사용자 ID 정보가 없습니다.");
                close(fd);
                return;
            }
        }
    }
    if(user_info == 0) admin(client_sock);  // 관리자 로그인
    else general(client_sock, &email);      // 일반사용자 로그인
}

// 1. 일반 사용자 로그인
void general(int client_sock, char *email) {
    char menu[] = "<강좌 관리>\n\n"
                    "1. 강좌 파일 생성\n"
                    "2. 강좌 목록 보기\n"
                    "3. 신규 강좌 생성\n"
                    "4. 강좌 정보 변경\n"
                    "5. 강좌 정보 검색\n"
                    "6. 강좌 정보 삭제\n"
                    "7. 강좌 정보 데이터 통계 저장\n"
                    "0. 종료\n"
                    "선택: ";
    int m = -1;
    while(m != 0) {
        write(client_sock, menu, sizeof(menu));

        bzero(num, sizeof(num));
        n = read(client_sock, num, sizeof(num));
        if(n < 0) error("강좌 관리 메뉴 읽기 실패");

        switch(atoi(num)) {
            case 1: lec_create(client_sock); break;
            case 2: lec_list(client_sock); break;
            case 3: lec_add(client_sock); break;
            case 4: lec_update(client_sock); break;
            case 5: lec_search(client_sock); break;
            case 6: lec_delete(client_sock); break;
            case 7: lec_store(client_sock); break;
            case 0: m = 0; close(fd2); return;
        }
    }
}

// 1-1. 강좌관리 파일 생성
void lec_create(int client_sock) {
    char file_name[20];

    send_message(client_sock, "생성할 파일 이름 입력: ");
    n = read(client_sock, file_name, sizeof(file_name));
    if(n < 0) error("파일 이름 읽기 실패");

    if((fd2 = open(file_name, O_RDWR | O_CREAT, 0666)) == -1) error("강좌 관리 파일 생성 실패");

    send_message(client_sock, "파일 생성 완료");
}

// 1-2. 강좌 목록 보기
void lec_list(int client_sock) {
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if(fcntl(fd2, F_SETLKW, &lock) == -1) error("레코드 잠금 실패");

    int count = 0;
    lseek(fd2, 0, SEEK_SET); // 첫번째 위치로 이동
    while((read(fd2, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
        count += 1;
    }

    if(count == 0) {
        send_message(client_sock, "강좌 정보가 없습니다.");
        return;
    }

    int i = 0;
    Lecture tmp[count];
    lseek(fd2, 0, SEEK_SET); // 첫번째 위치로 이동
    while((read(fd2, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
        tmp[i++] = lec;
    }

    for(int i=0; i<count-1; i++) {
        for(int j=i+1; j<count; j++) {
            if(strcmp(tmp[i].class, tmp[j].class) > 0) {
                Lecture tmp2 = tmp[i];
                tmp[i] = tmp[j];
                tmp[j] = tmp2;
            }
        }
    }

    char msg[200];
    for(int i=0; i<count; i++) {
        bzero(msg, sizeof(msg));
        snprintf(msg, sizeof(msg), 
        "강좌번호: %d, 강좌명: %s, 교수명: %s, 개설학과: %s, 최대수강인원: %d, 비고: %s\n",
        tmp[i].id, tmp[i].class, tmp[i].pro, tmp[i].department, tmp[i].max, tmp[i].note);

        write(client_sock, msg, strlen(msg));
    }

    lock.l_type = F_UNLCK;
    fcntl(fd2, F_SETLK, &lock);
}

// 1-3. 신규 강좌 생성
void lec_add(int client_sock) {
    int id; 

    send_message(client_sock, "추가할 강좌 정보를 입력하세요.\n");
    send_message(client_sock, "강좌번호: ");
    bzero(num, sizeof(num));
    n = read(client_sock, num, sizeof(num));
    if(n < 0) error("강좌번호 읽기 실패");
    id = atoi(num);

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = id * sizeof(lec);
    lock.l_len = sizeof(lec);
    if(fcntl(fd2, F_SETLKW, &lock) == -1) error("레코드 잠금 실패");

    send_message(client_sock, "강좌명: ");
    n = read(client_sock, lec.class, sizeof(lec.class));
    if(n < 0) error("강좌명 읽기 실패");

    send_message(client_sock, "교수명: ");
    n = read(client_sock, lec.pro, sizeof(lec.pro));
    if(n < 0) error("교수명 읽기 실패");

    send_message(client_sock, "개설학과명: ");
    n = read(client_sock, lec.department, sizeof(lec.department));
    if(n < 0) error("개설학과명 읽기 실패");

    send_message(client_sock, "최대수강인원: ");
    bzero(num, sizeof(num));
    n = read(client_sock, num, sizeof(num));
    if(n < 0) error("최대수강인원 읽기 실패");
    lec.max = atoi(num);

    send_message(client_sock, "비고: ");
    n = read(client_sock, lec.note, sizeof(lec.note));
    if(n < 0) error("비고 읽기 실패");

    lseek(fd2, lec.id*sizeof(lec), SEEK_SET);
    if(write(fd2, &lec, sizeof(lec)) == -1) {
        send_message(client_sock, "신규 강좌 추가 실패");
        return;   
    }
    send_message(client_sock, "신규 강좌 추가 완료");

    lock.l_type = F_UNLCK;
    fcntl(fd2, F_SETLK, &lock);
}

// 1-4. 강좌 정보 변경
void lec_update(int client_sock) {
    int id;

    send_message(client_sock, "강좌번호 입력: ");
    bzero(num, sizeof(num));
    n = read(client_sock, num, sizeof(num));
    if(n < 0) error("강좌번호 읽기 실패");
    id = atoi(num);

    lseek(fd2, id*sizeof(lec), SEEK_SET);
    if((read(fd2, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = id*sizeof(lec);
        lock.l_len = sizeof(lec);
        if(fcntl(fd2, F_SETLKW, &lock) == -1) error("레코드 잠금 실패");

        send_message(client_sock, "수정할 강좌 정보를 입력하세요.\n");
        send_message(client_sock, "강좌명: ");
        n = read(client_sock, lec.class, sizeof(lec.class));
        if(n < 0) error("강좌명 읽기 실패");

        send_message(client_sock, "교수명: ");
        n = read(client_sock, lec.pro, sizeof(lec.pro));
        if(n < 0) error("교수명 읽기 실패");

        send_message(client_sock, "개설학과명: ");
        n = read(client_sock, lec.department, sizeof(lec.department));
        if(n < 0) error("개설학과명 읽기 실패");

        send_message(client_sock, "최대수강인원: ");
        bzero(num, sizeof(num));
        n = read(client_sock, num, sizeof(num));
        if(n < 0) error("최대수강인원 읽기 실패");
        lec.max = atoi(num);

        send_message(client_sock, "비고: ");
        n = read(client_sock, lec.note, sizeof(lec.note));
        if(n < 0) error("비고 읽기 실패");

        lseek(fd2, -sizeof(lec), SEEK_CUR);
        if(write(fd2, &lec, sizeof(lec)) == -1) {
            send_message(client_sock, "강좌정보 수정 실패");
            return;   
        }
        send_message(client_sock, "강좌정보 수정 완료");

        lock.l_type = F_UNLCK;
        fcntl(fd2, F_SETLK, &lock);
    }
}

// 1-5. 강좌 정보 검색(강좌명) 
void lec_search(int client_sock) {
    int found = 0;
    char class[20];
    char msg[200];

    send_message(client_sock, "검색할 강좌명 입력: ");
    n = read(client_sock, class, sizeof(class));
    if(n < 0) error("강좌명 읽기 실패");

    lseek(fd2, 0, SEEK_SET); // 첫번째 위치로 이동
    while((read(fd2, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
        if(strstr(lec.class, class) != NULL) {
            found += 1;

            bzero(msg, sizeof(msg));
            snprintf(msg, sizeof(msg), 
            "강좌번호: %d, 강좌명: %s, 교수명: %s, 개설학과: %s, 최대수강인원: %d, 비고: %s\n",
            lec.id, lec.class, lec.pro, lec.department, lec.max, lec.note);

            write(client_sock, msg, strlen(msg));
        }
    }
    if(!found) {
        send_message(client_sock, "검색한 강좌 정보가 없습니다.");
        return;
    }
}

// 1-6. 강좌 정보 삭제
void lec_delete(int client_sock) {
    int id;

    send_message(client_sock, "강좌번호 입력: ");
    bzero(num, sizeof(num));
    n = read(client_sock, num, sizeof(num));
    if(n < 0) error("강좌번호 읽기 실패");
    id = atoi(num);

    lseek(fd2, id*sizeof(lec), SEEK_SET);
    if((read(fd2, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = id*sizeof(lec);
        lock.l_len = sizeof(lec);
        if(fcntl(fd2, F_SETLKW, &lock) == -1) error("레코드 잠금 실패");

        lec.id = -1;

        lseek(fd2, -sizeof(lec), SEEK_CUR);
        if(write(fd2, &lec, sizeof(lec)) == -1) {
            send_message(client_sock, "강좌정보 삭제 실패");
            return;   
        }
        send_message(client_sock, "강좌정보 삭제 완료");

        lock.l_type = F_UNLCK;
        fcntl(fd2, F_SETLK, &lock);
    }
}

// 1-7. 강좌 정보 데이터 통계 저장
void lec_store(int client_sock) {
    if(fork() == 0) {
        if((fd3 = open("data.txt", O_RDWR | O_CREAT | O_APPEND, 0666)) == -1) {
            perror("data.txt");
            exit(1);
        }

        // 파일 저장일시
        time_t file_time = time(NULL); // 현재 시간 얻음
        char time_str[20];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&file_time));

        // 전체 레코드 개수, 강좌 수강 최대인원 총합
        int count = 0;
        int result = 0;
        lseek(fd2, 0, SEEK_SET); // 첫번째 위치로 이동
        while((read(fd2, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
            count += 1;
            result += lec.max;
        }

        // 삭제 레코드 수
        int del = 0;
        lseek(fd2, 0, SEEK_SET); // 첫번째 위치로 이동
        while((read(fd2, &lec, sizeof(lec)) > 0) && (lec.id == -1)) {
            del += 1;
        }

        // 강좌당 평균 수강인원
        double average = (double)result/count;

        // 강좌 최대, 최소 인원
        int max = -1;
        int min = result;
        lseek(fd2, 0, SEEK_SET); // 첫번째 위치로 이동
        while((read(fd2, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
            if(max < lec.max) max = lec.max;
            if(min > lec.max) min = lec.max;
        }

        // 파일에 데이터 통계 저장
        dprintf(fd3, "<%s.log>\n", time_str);
        dprintf(fd3, "system software / sunghee choi\n");
        dprintf(fd3, "전체 레코드 개수: %d\n", count);
        dprintf(fd3, "삭제 레코드 개수: %d\n", del);
        dprintf(fd3, "강좌 수강 최대인원 총합: %d\n", result);
        dprintf(fd3, "강좌당 평균 수강 인원: %.1f\n", average);
        dprintf(fd3, "강좌 최대인원: %d\n", max);
        dprintf(fd3, "강좌 최소인원: %d\n\n", min);

        close(fd3);
        exit(0);
    }
    else {
        return;
    }
}

// 2. 관리자 로그인
void admin(int client_sock) {
    char menu[] = "<사용자 관리>\n\n"
                    "1. 사용자 목록 보기\n"
                    "2. 신규 사용자 생성\n"
                    "3. 사용자 정보 변경\n"
                    "4. 사용자 정보 검색\n"
                    "5. 사용자 삭제\n"
                    "0. 로그아웃\n"
                    "선택: ";
    int m = -1;
    while(m != 0) {
        write(client_sock, menu, sizeof(menu));

        bzero(num, sizeof(num));
        n = read(client_sock, num, sizeof(num));
        if(n < 0) error("사용자 관리 메뉴 읽기 실패");

        // 파일 열기
        fd = open("user", O_RDWR, 0666);
        if(fd < 0) error("파일 열기 실패");

        switch(atoi(num)) {
            case 1: user_list(client_sock); break;
            case 2: user_add(client_sock); break;
            case 3: user_update(client_sock); break;
            case 4: user_search(client_sock); break;
            case 5: user_delete(client_sock); break;
            case 0: m = 0; close(fd); return;
        }
    }
}

// 2-1. 사용자 목록 보기
void user_list(int client_sock) {
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if(fcntl(fd, F_SETLKW, &lock) < 0) error("레코드 잠금 실패");

    lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동
    char user_info[200];
    while((read(fd, &users, sizeof(users)) > 0) && (users.id != -1)) {
        bzero(user_info, sizeof(user_info));
        snprintf(user_info, sizeof(user_info),
        "식별자 ID: %d, Email: %s, PW: %s, 이름: %s, 주소:%s, 출생년도: %d, 관리자 여부: %d, 로그인 횟수: %d, 최근 로그인: %s\n", 
        users.id, users.email, users.pw, users.name, users.addr, users.birth, users.admin, users.count, ctime(&users.last));

        write(client_sock, user_info, strlen(user_info));
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
}

// 2-3. 사용자 정보 변경
void user_update(int client_sock) {
    int id;

    send_message(client_sock, "변경할 식별자 아이디: ");
    bzero(num, sizeof(num));
    n = read(client_sock, num, sizeof(num));
    if(n < 0) error("식별자 아이디 읽기 실패");
    id = atoi(num);

    lseek(fd, id*sizeof(users), SEEK_SET);
    if((read(fd, &users, sizeof(users)) > 0) && (users.id != -1)) {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = users.id * sizeof(users);
        lock.l_len = sizeof(users);
        if(fcntl(fd, F_SETLKW, &lock) < 0) error("레코드 잠금 실패");

        char email[20];
        send_message(client_sock, "수정할 email 입력: ");
        n = read(client_sock, email, sizeof(email));
        if(n < 0) error("수정할 이메일 읽기 실패");

        lseek(fd, -sizeof(users), SEEK_CUR);
        if(write(fd, &users, sizeof(users)) < 0) {
            error("사용자 정보 수정 실패");
             return;
        }
        send_message(client_sock, "사용자 정보 수정 완료");

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
    }
}

// 2-4. 사용자 정보 검색
void user_search(int client_sock) {
    int found = 0;
    char email[20];

    send_message(client_sock, "검색할 User ID(email): ");
    n = read(client_sock, email, sizeof(email));
    if(n < 0) error("이메일 읽기 실패");

    lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동
    while((read(fd, &users, sizeof(users)) > 0) && (users.id != -1)) {
        if(strstr(users.email, email) != NULL) {
            found += 1;

            char user_info[200];
            snprintf(user_info, sizeof(user_info), 
            "식별자 ID: %d, Email: %s, PW: %s, 이름: %s, 주소: %s, 출생년도: %d, 관리자 여부: %d, 로그인 횟수: %d, 최근 로그인: %s\n",
            users.id, users.email, users.pw, users.name, users.birth, users.admin, users.count, ctime(&users.last));

            write(client_sock, user_info, sizeof(user_info));
        }
    }
    if(!found) {
        send_message(client_sock, "검색한 사용자 정보가 없습니다.");
        return;
    }
}

// 2-5. 사용자 삭제
void user_delete(int client_sock) {
    int found = 0;
    char email[20];

    send_message(client_sock, "삭제할 User ID(email): ");

    n = read(client_sock, email, sizeof(email));
    if(n < 0) error("이메일 읽기 실패");

    lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동
    while((read(fd, &users, sizeof(users)) > 0) && (users.id != -1)) {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 0;
        if(fcntl(fd, F_SETLKW, &lock) < 0) error("레코드 잠금 실패");

        if(strstr(users.email, email) != NULL) {
            found = 1;

            users.id = -1;

            lseek(fd, -sizeof(users), SEEK_CUR);
            if(write(fd, &users, sizeof(users)) < 0) {
                send_message(client_sock, "사용자 정보 삭제 실패");

                lock.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &lock);
                return;
            }
            send_message(client_sock, "사용자 정보 삭제 완료");

            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);
        }
    }
    if(!found) {
        send_message(client_sock, "입력한 사용자 정보가 없습니다.");
        return;
    }
}

// 3. 사용자 추가
void user_add(int client_sock) {
    char msg[200];

    send_message(client_sock, "사용자 추가 화면입니다.");

    // 파일 열기
    fd = open("user", O_RDWR | O_CREAT, 0666);
    if(fd < 0) error("파일 열기 실패");

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if(fcntl(fd, F_SETLKW, &lock) < 0) error("레코드 잠금 실패");

    char email[20];
    int check = 1;
    while(check == 1) {
        send_message(client_sock, "User ID(email): ");

        bzero(email, sizeof(email));
        n = read(client_sock, email, sizeof(email));
        if(n < 0) error("이메일 읽기 실패");

        // 이메일 형식 체크
        if((strstr(email, "@") == NULL) || (strstr(email, ".") == NULL)) {
            send_message(client_sock, "이메일 형식이 잘못되었습니다.");

            continue;
        }

        // 이메일 중복 체크
        int found = 0;
        lseek(fd, 0, SEEK_SET); // 첫번째 위치로 이동
        while(read(fd, &users, sizeof(users)) > 0) {
            if(strcmp(users.email, email) == 0) {
                send_message(client_sock, "이미 있는 사용자입니다.");

                found = 1;
                break;
            }
        }

        if(!found) {
            check = 0;

            send_message(client_sock, "새로운 사용자 ID(email)을 만듭니다.");
            strncpy(users.email, email, sizeof(email));

            send_message(client_sock, "Password: ");
            n = read(client_sock, users.pw, sizeof(users.pw));
            if(n < 0) error("비밀번호 읽기 실패");

            send_message(client_sock, "식별자 아이디: ");
            bzero(num, sizeof(num));
            n = read(client_sock, num, sizeof(num));
            if(n < 0) error("식별자 아이디 읽기 실패");
            users.id = atoi(num);

            send_message(client_sock, "이름: ");
            n = read(client_sock, users.name, sizeof(users.name));
            if(n < 0) error("이름 읽기 실패");

            send_message(client_sock, "주소: ");
            n = read(client_sock, users.addr, sizeof(users.addr));
            if(n < 0) error("주소 읽기 실패");

            send_message(client_sock, "출생년도: ");
            bzero(num, sizeof(num));
            n = read(client_sock, num, sizeof(num));
            if(n < 0) error("출생년도 읽기 실패");
            users.birth = atoi(num);

            send_message(client_sock, "관리자(0) / 일반사용자(1): ");
            bzero(num, sizeof(num));
            n = read(client_sock, num, sizeof(num));
            if(n < 0) error("관리자 여부 읽기 실패");
            users.admin = atoi(num);

            users.count = users.last = 0; 

            lseek(fd, users.id*sizeof(users), SEEK_SET);
            if(write(fd, &users, sizeof(users)) < 0) {
                send_message(client_sock, "사용자 추가 실패");
                return;
            }
            send_message(client_sock, "사용자 추가 완료");
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    close(fd);
}