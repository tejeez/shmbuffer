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

// last sizeof(size_t) bytes of shm are an index to the element to be written next
#define SHM_BUFSIZE 0x4000000ULL
#define SHM_SIZE (DATASIZE*SHM_BUFSIZE + sizeof(size_t))

int main() {
	void *shm_buf;
	ssize_t r;
	size_t p = 0, *shm_p;
	size_t shm_bufsize_bytes;
	int shm_fd;

	shm_bufsize_bytes = DATASIZE * SHM_BUFSIZE;

	shm_fd = shm_open("/sdr-shm", O_CREAT | O_RDWR, 0644);
	if(shm_fd < 0) {
		fprintf(stderr, "shm_open failed\n");
		return 1;
	}

	if(ftruncate(shm_fd, SHM_SIZE) < 0) {
		fprintf(stderr, "ftruncate failed\n");
		return 1;
	}

	shm_buf = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if(shm_buf == MAP_FAILED) {
		fprintf(stderr, "Map failed\n");
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
