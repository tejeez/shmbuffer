%: %.c
	gcc $^ -o $@ -Wall -Wextra -O3 -lrt

all: fixbitorder_shmwrite shmread

fixbitorder_shmwrite: fixbitorder_shmwrite.c

shmread: shmread.c
