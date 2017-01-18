%: %.c
	gcc $^ -o $@ -Wall -Wextra -O3 -lrt

all: shmwrite fixbitorder_shmwrite shmread shmread_status

shmwrite: shmwrite.c

fixbitorder_shmwrite: fixbitorder_shmwrite.c

shmread: shmread.c

shmread_status: shmread_status.c

clean:
	rm shmwrite fixbitorder_shmwrite shmread shmread_status

.PHONY: all clean
