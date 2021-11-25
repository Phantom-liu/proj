#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char **argv) {
	if (argc == 1) {
		printf("wzip: file1 [file2 ...]\n");
		return 1;
	}
	int i = 1;
	FILE *fp = fopen(argv[i], "r");
	int j;
	char tmp, pre;
	pre = fgetc(fp);
	while (pre == EOF) {
		fclose(fp);
		if (argv[++i] == NULL) {
			return 0;
		}
		fp = fopen(argv[i], "r");
		pre = fgetc(fp);
	}
	while (argv[i]) {
		j = 0;
		tmp = pre;
		while (tmp == pre) {
			tmp = fgetc(fp);
			j++;
			while (tmp == EOF) {
				fclose(fp);
				if (argv[++i] == NULL) {
					break;
				}
				fp = fopen(argv[i], "r");
				tmp = fgetc(fp);
			}
		}
		fwrite(&j, 4, 1, stdout);
		fwrite(&pre, 1, 1, stdout);
		pre = tmp;
	}
	return 0;
}

