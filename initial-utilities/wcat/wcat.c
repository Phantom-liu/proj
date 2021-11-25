#include<stdio.h>

#define MAXLINE 1024

int main(int argc, char **args) {
	int i = 1;
	while (args[i]) {
		FILE *input = fopen(args[i++], "r");
		if (input == NULL) {
			printf("wcat: cannot open file\n");
			return 1;
		}
		char str[MAXLINE];
		while (fgets(str, MAXLINE, input) != NULL) {
			printf("%s", str);
		}
		fclose(input);
	}
	return 0;

}


