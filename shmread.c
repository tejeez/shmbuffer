#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#define BUFS 0x10000

// sizeof(uint16_t)
#define DATASIZE 2

// last sizeof(size_t) bytes of shm are an index to the element to be written next
#define SHM_BUFSIZE 0x4000000ULL
#define SHM_SIZE (DATASIZE*SHM_BUFSIZE + sizeof(size_t))


int main(int argc, char *argv[]) {
	void *shm_buf;
	size_t nextp = 0, prevp = 0, *shm_p;
	int shm_fd;

	if(argc < 2) {
		fprintf(stderr, "Usage: %s /name_of_shm\n", argv[0]);
		return 1;
	}

	shm_fd = shm_open(argv[1], O_RDONLY, 0644);
	if(shm_fd < 0) {
		perror("shm_open failed");
		return 1;
	}

	shm_buf = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
	if(shm_buf == MAP_FAILED) {
		perror("Map failed");
		return 1;
	}
	shm_p = shm_buf + DATASIZE*SHM_BUFSIZE;
	prevp = DATASIZE * *shm_p;

	for(;;) {
		size_t nb, nwritten;
		nextp = DATASIZE * *shm_p;

		if(nextp > prevp) {
			nb = nextp - prevp;
			nwritten = write(1, shm_buf + prevp, nb);
			prevp += nwritten;
			assert(prevp <= DATASIZE*SHM_BUFSIZE);
		} else if(nextp < prevp) {
			// wrapped around
			nb = DATASIZE*SHM_BUFSIZE - prevp;
			nwritten = write(1, shm_buf + prevp, nb);
			prevp += nwritten;
			if(prevp == DATASIZE*SHM_BUFSIZE) {
				// continue reading in next iteration
				prevp = 0;
			}
			assert(prevp <= DATASIZE*SHM_BUFSIZE);
		} else {
			// no new data
			usleep(50000);
		}
	}
	return 0;
}
