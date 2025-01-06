#include<stdio.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

char type(mode_t);
char *perm(mode_t);
void printStat(char*, char*, struct stat*);

void directory_list() {
    DIR *dp;
    char dir_name[BUFSIZ];
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
        sprintf(path, "%s/%s", dir_name, d->d_name);

        // lstat 호출 시 파일 경로 확인
        if(lstat(path, &st) < 0) {
            perror(path);
        }
        else {
            printStat(path, d->d_name, &st);
            printf("\n\n");
        }        
    }

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