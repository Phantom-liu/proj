#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/sysinfo.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/mman.h>
#include<semaphore.h>

//args in the threads
typedef struct filepiece {
	int startIndex;
	int startOffset;
	int endIndex;
	int endOffset;
	int id;
}filepiece;

#define MAX_FILE_NUM 10
#define MAXSIZE 1000000
int sum[MAX_FILE_NUM];
int fd[MAX_FILE_NUM];
struct stat buf[MAX_FILE_NUM];
sem_t order[10];

void* zip(void *args) {
	filepiece *f = (filepiece*)args;
	int start = f->startOffset;
	int end = f->endOffset;
	char result[MAXSIZE];
	int result_index = 0;
	for (int i = f->startIndex; i <= f->endIndex; i++) {
		char *input = mmap(0, buf[i].st_size, PROT_READ, MAP_PRIVATE, fd[i], 0);
		int bound = (i == f->endIndex) ? f->endOffset : buf[i].st_size;
		if (start&&input[start] == input[start - 1]) {
			while (input[start] == input[start - 1])
				start++;
		}
		for (int j = start; j < bound; ) {
			char pre = input[j];
			int cnt = 1;
			while (input[++j] == pre) {
				cnt++;
			}
			char *tmp = (char*)(&cnt);
			result[result_index++] = tmp[0];
			result[result_index++] = tmp[1];
			result[result_index++] = tmp[2];
			result[result_index++] = tmp[3];
			result[result_index++] = pre;
		}
		start = 0;
		munmap(input, buf[i].st_size);
	}
	sem_wait(&order[f->id]);
	fwrite(result, result_index, 1, stdout);
	sem_post(&order[f->id + 1]);
	return NULL;
}


int main(int argc, char **argv) {
	if (argc == 1) {
		printf("wzip: file1 [file2 ...]\n");
		return 1;
	}
	int count = get_nprocs();
	for (int i = 1; i <= count; i++) {
		sem_init(&order[i], 0, 0);
	}
	sem_post(&order[1]);
	memset(sum, 0, sizeof(sum));
	int sumtmp = 0;
	for (int i = 1; i < argc; i++) {
		fd[i] = open(argv[i], O_RDONLY);
		if (fd[i] == -1) {
			perror("open");
			return 1;
		}
		stat(argv[i], &buf[i]);
		sumtmp += buf[i].st_size;
		sum[i] = sumtmp;
	}
	int perSize = sumtmp / count;
	//initial the piece information for each thread
	filepiece info[MAX_FILE_NUM];
	int i = 1, j = 1;
	int last_offset = 0;
	while (i <= count && j < argc) {
		int leftSize = perSize;
		if (i == count) {
			leftSize = sumtmp - perSize * (count - 1);
		}
		info[i].startIndex = j;
		info[i].startOffset = last_offset;
		while (leftSize) {
			if (leftSize >= (buf[j].st_size - last_offset)) {
				leftSize -= (buf[j].st_size - last_offset);
				j++;
				last_offset = 0;
			}
			else {
				last_offset += leftSize;
				leftSize = 0;
			}
		}
		info[i].endIndex = j;
		info[i].endOffset = last_offset;
		info[i].id = i;
		i++;
	}

	//now we can split the total file into some pieces(of which have the same size)
	//and assign tasks
	pthread_t tid[9];
	for (int i = 1; i <= count; i++) {
		if (pthread_create(&tid[i], NULL, zip, &info[i]) != 0) {
			perror("pthread_create");
			exit(1);
		}
	}
	for (int i = 1; i <= count; i++) {
		pthread_join(tid[i], NULL);
	}
	for (int i = 1; i < argc; i++) {
		close(fd[i]);
	}
	exit(0);
}

