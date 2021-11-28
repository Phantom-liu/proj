#include "mapreduce.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include<dlfcn.h>

void Map(char *file_name) {
	FILE *fp = fopen(file_name, "r");
	assert(fp != NULL);

	char *line = NULL;
	size_t size = 0;
	while (getline(&line, &size, fp) != -1) {
		char *token, *dummy = line;
		while ((token = strsep(&dummy, " \t\n\r")) != NULL) {
			MR_Emit(token, "1");
		}
	}
	free(line);
	fclose(fp);
}

void Reduce(char *key, Getter get_next, int partition_number) {
	int count = 0;
	char *value;
	while ((value = get_next(key, partition_number)) != NULL)
		count++;
	//printf("%s %d\n", key, count);
}

int main(int argc, char *argv[]) {
	struct timeval s, e;
	gettimeofday(&s, 0);
	MR_Run(argc, argv, Map, 10, Reduce, 10, MR_DefaultHashPartition);
	gettimeofday(&e, 0);
	double timeuse = 1000000 * (e.tv_sec - s.tv_sec) + e.tv_usec - s.tv_usec;
	fprintf(stderr, "main: Time taken is %lf ms\n", timeuse / 1000);
}