#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include<fcntl.h>

typedef struct {
    int id;                 // 강좌번호
    char class[20];         // 강좌명
    char pro[12];           // 교수명
    char department[15];    // 개설학과명
    int max;                // 최대수강인원
    char note[40];          // 비고
} Lecture;

int fd1, fd2;
Lecture lec;
struct flock lock;

// 1. 강좌관리 파일 생성
void create_file() {
    char file_name[20];

    printf("생성할 파일 이름 입력: "); scanf(" %s", file_name);

    if((fd1 = open(file_name, O_RDWR | O_CREAT, 0666)) == -1) {
        perror(file_name);
        exit(1);
    }

    printf("\n파일 생성 완료\n\n");
}

// 2. 강좌 목록 보기
void list_lec() {
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if(fcntl(fd1, F_SETLKW, &lock) == -1) {
        perror("\n레코드 잠금 실패\n\n");
        exit(1);
    }

    int count = 0;
    lseek(fd1, 0, SEEK_SET); // 첫번째 위치로 이동
    while((read(fd1, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
        count += 1;
    }

    if(count == 0) {
        printf("\n강좌 정보가 없습니다.\n\n");
        return;
    }

    int i = 0;
    Lecture tmp[count];
    lseek(fd1, 0, SEEK_SET); // 첫번째 위치로 이동
    while((read(fd1, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
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

    for(int i=0; i<count; i++) {
        printf("강좌번호: %d, 강좌명: %s, 교수명: %s, ", tmp[i].id, tmp[i].class, tmp[i].pro);
        printf("개설학과: %s, 최대수강인원: %d, ", tmp[i].department, tmp[i].max);
        printf("비고: %s\n\n", tmp[i].note);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd1, F_SETLK, &lock);
}

// 3. 신규 강좌 생성
void new_lec() {
    printf("\n추가할 강좌 정보를 입력하세요.\n\n");
    printf("강좌번호: "); scanf(" %d", &lec.id);

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = lec.id * sizeof(lec);
    lock.l_len = sizeof(lec);
    if(fcntl(fd1, F_SETLKW, &lock) == -1) {
        perror("\n레코드 잠금 실패\n\n");
        exit(1);
    }

    printf("강좌명(공백가능): "); scanf(" %[^\n]", lec.class);
    printf("교수명(공백가능): "); scanf(" %[^\n]", lec.pro);
    printf("개설학과명: "); scanf(" %s", lec.department);
    printf("최대수강인원: "); scanf(" %d", &lec.max);
    printf("비고(공백가능): "); scanf(" %[^\n]", lec.note);

    lseek(fd1, lec.id*sizeof(lec), SEEK_SET);
    if(write(fd1, &lec, sizeof(lec)) == -1) {
        printf("\n신규 강좌 추가 실패\n\n");
        return;   
    }
    printf("\n신규 강좌 추가 완료\n\n");

    lock.l_type = F_UNLCK;
    fcntl(fd1, F_SETLK, &lock);
}

// 4. 강좌 정보 변경
void update_lec() {
    int id;

    printf("강좌번호 입력: "); scanf(" %d", &id);
    lseek(fd1, id*sizeof(lec), SEEK_SET);
    if((read(fd1, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = lec.id*sizeof(lec);
        lock.l_len = sizeof(lec);
        if(fcntl(fd1, F_SETLKW, &lock) == -1) {
            perror("\n레코드 잠금 실패\n\n");
            exit(1);
        }

        printf("\n현재 강좌 정보\n");
        printf("강좌번호: %d, 강좌명: %s, 교수명: %s, ", lec.id, lec.class, lec.pro);
        printf("개설학과: %s, 최대수강인원: %d, ", lec.department, lec.max);
        printf("비고: %s\n\n", lec.note);

        printf("수정할 강좌 정보를 입력하세요.\n\n");
        printf("강좌명(공백가능): "); scanf(" %[^\n]", lec.class);
        printf("교수명(공백가능): "); scanf(" %[^\n]", lec.pro);
        printf("개설학과명: "); scanf(" %s", lec.department);
        printf("최대수강인원: "); scanf(" %d", &lec.max);
        printf("비고(공백가능): "); scanf(" %[^\n]", lec.note);

        lseek(fd1, -sizeof(lec), SEEK_CUR);
        if(write(fd1, &lec, sizeof(lec)) == -1) {
            printf("\n강좌정보 수정 실패\n\n");
            return;   
        }
        printf("\n강좌정보 수정 완료\n\n");

        lock.l_type = F_UNLCK;
        fcntl(fd1, F_SETLK, &lock);
    }
}

// 5. 강좌 정보 검색(강좌명) 
void search_lec() {
    int found = 0;
    char class[20];

    printf("검색할 강좌명 입력: "); scanf(" %s", class);
    lseek(fd1, 0, SEEK_SET); // 첫번째 위치로 이동
    while((read(fd1, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
        if(strstr(lec.class, class) != NULL) {
            found += 1;

            printf("강좌번호: %d, 강좌명: %s, 교수명: %s, ", lec.id, lec.class, lec.pro);
            printf("개설학과: %s, 최대수강인원: %d, ", lec.department, lec.max);
            printf("비고: %s\n\n", lec.note);
        }
    }

    if(!found) {
        printf("\n검색한 강좌 정보가 없습니다.\n\n");
        return;
    }
}

// 6. 강좌 정보 삭제
void del_lec() {
    int id;

    printf("강좌번호 입력: "); scanf(" %d", &id);
    lseek(fd1, id*sizeof(lec), SEEK_SET);
    if((read(fd1, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = id*sizeof(lec);
        lock.l_len = sizeof(lec);
        if(fcntl(fd1, F_SETLKW, &lock) == -1) {
            perror("\n레코드 잠금 실패\n\n");
            exit(1);
        }

        printf("\n현재 강좌 정보\n");
        printf("강좌번호: %d, 강좌명: %s, 교수명: %s, ", lec.id, lec.class, lec.pro);
        printf("개설학과: %s, 최대수강인원: %d, ", lec.department, lec.max);
        printf("비고: %s\n\n", lec.note);

        lec.id = -1;

        lseek(fd1, -sizeof(lec), SEEK_CUR);
        if(write(fd1, &lec, sizeof(lec)) == -1) {
            printf("강좌정보 삭제 실패\n\n");
            return;   
        }
        printf("강좌정보 삭제 완료\n\n");

        lock.l_type = F_UNLCK;
        fcntl(fd1, F_SETLK, &lock);
    }
}

// 7. 강좌 정보 데이터 통계 저장
void data_store() {
    printf("\n");

    if(fork() == 0) {
        if((fd2 = open("data.txt", O_RDWR | O_CREAT | O_APPEND, 0666)) == -1) {
            perror("data.txt");
            exit(1);
        }

        // 파일 저장일시
        time_t file_time = time(NULL); // 현재 시간 얻음
        char time_str[20];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&file_time));

        // 레코드 개수, 강좌 수강 최대인원 총합
        int count = 0;
        int result = 0;
        lseek(fd1, 0, SEEK_SET); // 첫번째 위치로 이동
        while((read(fd1, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
            count += 1;
            result += lec.max;
        }

        // 강좌당 평균 수강인원
        double average = (double)result/count;

        // 강좌 최대, 최소 인원
        int max = -1;
        int min = result;
        lseek(fd1, 0, SEEK_SET); // 첫번째 위치로 이동
        while((read(fd1, &lec, sizeof(lec)) > 0) && (lec.id != -1)) {
            if(max < lec.max) max = lec.max;
            if(min > lec.max) min = lec.max;
        }

        // 파일에 데이터 통계 저장
        dprintf(fd2, "<%s.log>\n", time_str);
        dprintf(fd2, "레코드 개수: %d\n", count);
        dprintf(fd2, "강좌 수강 최대인원 총합: %d\n", result);
        dprintf(fd2, "강좌당 평균 수강 인원: %.1f\n", average);
        dprintf(fd2, "강좌 최대인원: %d\n", max);
        dprintf(fd2, "강좌 최소인원: %d\n\n", min);

        close(fd2);
        exit(0);
    }
    else {
        return;
    }
}

void lecture() {
    int n = -1;

    while(n != 0) {
        printf("1. 강좌관리 파일 생성\n");
        printf("2. 강좌 목록 보기\n");
        printf("3. 신규 강좌 생성\n");
        printf("4. 강좌 정보 변경\n");
        printf("5. 강좌 정보 검색(강좌명)\n");
        printf("6. 강좌 정보 삭제\n");
        printf("7. 강좌 정보 데이터 통계 저장\n");
        printf("0. 종료\n\n");

        printf("메뉴를 선택하세요: "); scanf("%d", &n);

        switch(n) {
            case 1: create_file(); break;
            case 2: list_lec(); break;
            case 3: new_lec(); break;
            case 4: update_lec(); break;
            case 5: search_lec(); break;
            case 6: del_lec(); break;
            case 7: data_store(); break;
            case 0: close(fd1); return;
        }
    }
}