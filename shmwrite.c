#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#define BUFS 0x10000

// writing and reading the buffer always starts at byte aligned to DATASIZE
#define DATASIZE 2

// TODO: take the buffer size as command line argument

// last sizeof(size_t) bytes of shm are an index to the element to be written next
#define SHM_BUFSIZE 0x4000000ULL
#define SHM_SIZE (DATASIZE*SHM_BUFSIZE + sizeof(size_t))

int main(int argc, char *argv[]) {
	void *shm_buf;
	ssize_t r;
	size_t p = 0, *shm_p;
	size_t shm_bufsize_bytes;
	int shm_fd;


	if(argc < 2) {
		fprintf(stderr, "Usage: %s /name_of_shm\n", argv[0]);
		return 1;
	}

	shm_bufsize_bytes = DATASIZE * SHM_BUFSIZE;

	shm_fd = shm_open(argv[1], O_CREAT | O_RDWR, 0644);
	if(shm_fd < 0) {
		perror("shm_open failed");
		return 1;
	}

	if(ftruncate(shm_fd, SHM_SIZE) < 0) {
		perror("ftruncate failed");
		return 1;
	}

	shm_buf = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if(shm_buf == MAP_FAILED) {
		perror("Map failed");
		return 1;
	}
	shm_p = shm_buf + DATASIZE*SHM_BUFSIZE;
	*shm_p = 0;

	p = 0; // in bytes
	for(;;) {
		size_t bytestoread = shm_bufsize_bytes - p;
		if(bytestoread > BUFS) bytestoread = BUFS;
		r = read(0, shm_buf + p, bytestoread);
		if(r <= 0) break;

		p += r;
		assert(p <= shm_bufsize_bytes);
		if(p >= shm_bufsize_bytes) p = 0;

		*shm_p = p / DATASIZE;
	}
	return 0;
}
