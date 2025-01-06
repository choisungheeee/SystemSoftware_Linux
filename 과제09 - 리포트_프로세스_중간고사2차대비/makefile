main: main.o manage.o student.o dir.o lecture.o
	gcc -o main main.o manage.o student.o dir.o lecture.o
main.o: main.c
	gcc -c main.c
manage.o: manage.c
	gcc -c manage.c
student.o: student.c
	gcc -c student.c
dir.o: dir.c
	gcc -c dir.c
lecture.o: lecture.c main.h
	gcc -c lecture.c