Main: gbv.c biblioteca.c biblioteca.h
	$(CC) $(FLAGS) gbv.c biblioteca.c -o gbv

FLAGS = -Wall \
	-Wextra \
	-std=c99

CC = gcc