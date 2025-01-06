#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>

// 에러 메시지 출력 함수
void error(char *msg) {
    perror(msg);
    exit(1);
}

void communicate_with_server(int sockfd) {
    char buffer[1024];
    int n;    
    int current_menu = 0;

    while(1) {
        bzero(buffer, sizeof(buffer));
        n = read(sockfd, buffer, sizeof(buffer));
        if(n < 0) error("서버 메시지 읽어오기 실패");

        printf("%s", buffer); // 서버 메시지 출력

        // 메뉴 상태 업데이트
        if(strstr(buffer, "사용자 추가 화면입니다.") != NULL) current_menu = 1;
        if(strstr(buffer, "일반 사용자 로그인 화면입니다.") != NULL) current_menu = 1;
        if(strstr(buffer, "<사용자 관리>") != NULL) current_menu = 1;

        bzero(buffer, sizeof(buffer));
        fgets(buffer, sizeof(buffer), stdin); // 입력 받기
        buffer[strcspn(buffer, "\n")] = '\0'; // 개행 문자 제거

        // 종료 조건
        if(current_menu == 0 && strncmp(buffer, "0", 1) == 0) break;

        n = write(sockfd, buffer, strlen(buffer));
        if(n < 0) error("메시지 서버 전송 실패");
    }
}

// 종료 함수
static void exit_handle(void) {
    printf("sunghee님이 구현한 시스템 소프트웨어 데모 시스템을 이용해주셔서 감사합니다.");
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    // 매개변수 확인
    if(argc < 3) {
        fprintf(stderr, "Usage %s hostname port", argv[0]);
        exit(1);
    }

    // 포트번호 int형으로 변환
    portno = atoi(argv[2]);

    // 소켓 생성
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) error("소켓 생성 실패");

    // 호스트 주소를 IP 주소로 변환
    server = gethostbyname(argv[1]);
    if(server == NULL) {
        fprintf(stderr, "호스트 주소 오류");
        exit(1);
    }

    // 서버 소켓 세팅
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    // 연결 요청
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("연결 오류");
    }

    communicate_with_server(sockfd);

    if(atexit(exit_handle) != 0) error("exit_handle 등록 오류");

    close(sockfd);
    return 0;
}