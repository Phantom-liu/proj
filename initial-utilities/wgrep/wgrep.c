#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char **argv) {
	size_t line_buf_size;
	int i = 0, j = 0;
	if (argv[1] == NULL) {
		printf("wgrep: searchterm [file ...]\n");
		return 1;
	}
	int origin_len = strlen(argv[1]);
	if (argv[2] == NULL) {
		char line_buf[1024];
		while (fgets(line_buf, 1024, stdin) != NULL) {
			int buf_len = strlen(line_buf);
			for (i = 0; i <= buf_len - origin_len; i++) {
				for (j = 0; j < origin_len; j++) {
					if (line_buf[i + j] != argv[1][j]) {
						break;
					}
				}
				if (j == origin_len) {
					printf("%s", line_buf);
					break;
				}
			}
		}
		return 0;
	}
	char *line_buf = NULL;
	FILE *fp = fopen(argv[2], "r");
	if (fp == NULL) {
		printf("wgrep: cannot open file\n");
		return 1;
	}
	int buf_len = getline(&line_buf, &line_buf_size, fp);
	while (buf_len >= 0) {
		for (i = 0; i <= buf_len - origin_len; i++) {
			for (j = 0; j < origin_len; j++) {
				if (line_buf[i + j] != argv[1][j]) {
					break;
				}
			}
			if (j == origin_len) {
				printf("%s", line_buf);
				break;
			}
		}
		buf_len = getline(&line_buf, &line_buf_size, fp);
	}
	free(line_buf);
	line_buf = NULL;
	fclose(fp);
	return 0;
}
