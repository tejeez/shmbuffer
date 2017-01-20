#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#define BUFS 0x10000

int main(int argc, char *argv[]) {
	void *shm_buf;
	size_t nextp = 0, prevp = 0, *shm_p;
	size_t shm_size, bufsize_bytes;
	int shm_fd;
	struct stat shm_stat;

	if(argc < 2) {
		fprintf(stderr, "Usage: %s /name_of_shm\n", argv[0]);
		return 1;
	}

	shm_fd = shm_open(argv[1], O_RDONLY, 0644);
	if(shm_fd < 0) {
		perror("[shmread] shm_open failed");
		return 1;
	}

	// get size
	if(fstat(shm_fd, &shm_stat) < 0) {
		perror("[shmread] fstat failed");
		return 1;
	}
	shm_size = shm_stat.st_size;
	bufsize_bytes = shm_size - sizeof(size_t);

	shm_buf = mmap(0, shm_size, PROT_READ, MAP_SHARED, shm_fd, 0);
	if(shm_buf == MAP_FAILED) {
		perror("[shmread] mmap failed");
		return 1;
	}
	shm_p = shm_buf + bufsize_bytes;
	prevp = *shm_p;
	assert(prevp < bufsize_bytes);

	for(;;) {
		size_t nb, nwritten;
		nextp = *shm_p;
		assert(nextp < bufsize_bytes);

		if(nextp > prevp) {
			nb = nextp - prevp;
			nwritten = write(1, shm_buf + prevp, nb);
			prevp += nwritten;
			assert(prevp <= bufsize_bytes);
		} else if(nextp < prevp) {
			// wrapped around
			nb = bufsize_bytes - prevp;
			nwritten = write(1, shm_buf + prevp, nb);
			prevp += nwritten;
			if(prevp == bufsize_bytes) {
				// continue reading in next iteration
				prevp = 0;
			}
			assert(prevp <= bufsize_bytes);
		} else {
			// no new data
			usleep(50000);
		}
	}
	return 0;
}
