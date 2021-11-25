#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char **argv) {
	if (argc == 1) {
		printf("wunzip: file1 [file2 ...]\n");
		return 1;
	}
	int i = 1;
	FILE *fp = fopen(argv[i], "r");
	int count;
	char tmp;
	while (1) {
		int k = fread(&count, 4, 1, fp);
		while (k < 1) {
			fclose(fp);
			if (argv[++i] == NULL) {
				return 0;
			}
			fp = fopen(argv[i], "r");
			k = fread(&count, 4, 1, fp);
		}
		fread(&tmp, 1, 1, fp);
		while (count--) {
			printf("%c", tmp);
		}
	}
	return 0;
}

