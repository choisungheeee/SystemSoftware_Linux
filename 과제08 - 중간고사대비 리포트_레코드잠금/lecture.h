extern FILE *file; 
extern int fd;
extern char file_name[50]; 

struct lecture {
    int id;          // 강좌 번호
    char name[20];    // 강좌명
    char p_name[20]; // 교수명
    char d_name[15]; // 개설학과명
    int max_p;       // 최대수강인원
    char note[40];   // 비고
};