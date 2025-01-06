extern FILE *file; 
extern int fd;
extern char file_name[50]; 
extern struct student rec;

struct student {
    int id; // 학번. 0부터 시작
    char name[20]; // 이름
    char sub[25]; // 과목명
    short score; // 점수
    float mod; // 보정점수
};