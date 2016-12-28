#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#define BUFS 0x10000
#define LUTN 0x1000

// sizeof(uint16_t)
#define DATASIZE 2

// last sizeof(size_t) bytes of shm are an index to the element to be written next
#define SHM_BUFSIZE 0x4000000ULL
#define SHM_SIZE (DATASIZE*SHM_BUFSIZE + sizeof(size_t))

int main() {
	uint16_t buf[BUFS], lut[LUTN];
	uint16_t *sbuf;
	void *shm_buf;
	ssize_t r, n, bi;
	size_t p = 0, *shm_p;
	int i;
	int shm_fd;

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
	sbuf = shm_buf;
	shm_p = shm_buf + DATASIZE*SHM_BUFSIZE;
	*shm_p = 0;

	for(i = 0; i < LUTN; i++) {
		int bitn;
		int16_t r=0;
		for(bitn = 0; bitn < 12; bitn++) {
			if(i & (1<<bitn))
				r |= 1 << (11-bitn);
		}
		lut[i] = (uint16_t)(r - 0x800);
	}
	for(;;) {
		r = read(0, buf, BUFS);
		if(r <= 0) break;
		if((r&1) == 1) {
			r += read(0, ((void*)buf) + r, 1);
		}
		n = r/2;
		for(bi = 0; bi < n; bi++) {
			sbuf[p] = lut[0xFFF & buf[bi]];
			p++;
			if(p >= SHM_BUFSIZE) p = 0;
		}
		*shm_p = p;
	}
	return 0;
}
