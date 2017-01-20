#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#define BUFS 0x10000

/* Writing and reading the buffer always starts at byte aligned to DATASIZE.
   Alignment of 16 bytes works for any 128-bit, 64-bit, 32-bit, 16-bit or 8-bit data type. */
#define DATASIZE 16

int main(int argc, char *argv[]) {
	void *shm_buf;
	ssize_t r;
	size_t p = 0, *shm_p;
	size_t bufsize_bytes = 0x400000, shm_size;
	int shm_fd;

	if(argc < 2) {
		fprintf(stderr, "Usage: %s /name_of_shm [size_in_bytes]\n", argv[0]);
		return 1;
	}

	if(argc >= 3)
		bufsize_bytes = atoll(argv[2]);

	bufsize_bytes = (bufsize_bytes / DATASIZE) * DATASIZE; // ensure alignment

	// last sizeof(size_t) bytes of shm are an index to the byte to be written next
	shm_size = bufsize_bytes + sizeof(size_t);

	shm_fd = shm_open(argv[1], O_CREAT | O_RDWR, 0644);
	if(shm_fd < 0) {
		perror("[shmwrite] shm_open failed");
		return 1;
	}

	if(ftruncate(shm_fd, shm_size) < 0) {
		perror("[shmwrite] ftruncate failed");
		return 1;
	}

	shm_buf = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if(shm_buf == MAP_FAILED) {
		perror("[shmwrite] mmap failed");
		return 1;
	}
	shm_p = shm_buf + bufsize_bytes;

	/* Get old value of *shm_p and start from that point if it seems valid.
	   This allows restarting shmwrite so that shmreads continue working. */
	p = *shm_p;
	if(p >= bufsize_bytes) p = 0; // set to zero if not valid

	p = (p / DATASIZE) * DATASIZE;
	*shm_p = p;

	for(;;) {
		size_t bytestoread = bufsize_bytes - p;
		if(bytestoread > BUFS) bytestoread = BUFS;
		r = read(0, shm_buf + p, bytestoread);
		if(r <= 0) break;

		p += r;
		assert(p <= bufsize_bytes);
		if(p >= bufsize_bytes) p = 0;

		*shm_p = (p / DATASIZE) * DATASIZE; // alignment
	}
	return 0;
}
