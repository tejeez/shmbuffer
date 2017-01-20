%: %.c
	gcc $^ -o $@ -Wall -Wextra -O3 -lrt

all: shmwrite shmread

shmwrite: shmwrite.c

shmread: shmread.c

clean:
	rm shmwrite shmread shmread_status

.PHONY: all clean
