CC=gcc

check_clones: check_clones.o check_clones.c
	$(CC) -o check_clones check_clones.c


