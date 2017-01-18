%: %.c
	gcc $^ -o $@ -Wall -Wextra -O3 -lrt

all: fixbitorder_shmwrite shmread shmread_status

fixbitorder_shmwrite: fixbitorder_shmwrite.c

shmread: shmread.c

shmread_status: shmread_status.c

clean:
	rm fixbitorder_shmwrite shmread shmread_status

.PHONY: all clean
