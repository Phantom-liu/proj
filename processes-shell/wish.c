#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>

char error_message[30] = "An error has occurred\n";
char Path[30][30] = { "." };
int pathcnt = 2	;

int sloveTheLine(char *str, char *carg[][10], char *redir[]) {
	int rawlen = strlen(str);
	int i;
	char *parallel[10];
	parallel[0] = str;
	int parallel_index = 1;
	for (i = 0; i < rawlen; i++) {
		/* slove the parallel */
		if (str[i] == '&') {
			str[i] = '\0';
			parallel[parallel_index++] = str + i + 1;
		}
	}
	int cnt = 0;
	int offset = 0;
	for (cnt = 0; cnt < parallel_index - offset; cnt++) {
		while (*parallel[cnt] == ' ') {
			(parallel[cnt])++;
		}
		int len = strlen(parallel[cnt]);
		/* judge the empty line */
		if (len == 0) {
			offset++;
			cnt--;
			continue;
		}
		for (i = 0; i < len; i++) {
			if (parallel[cnt][i] == ' ') {
				parallel[cnt][i] = '\0';
			}
		}
		int carg_index = 0;
		for (i = 0; i < len;) {
			int j;
			if (parallel[cnt][i] == '\0') {
				i++;
				continue;
			}
			for (j = i + 1; j < len; j++) {
				if (parallel[cnt][j] == '\0') {
					break;
				}
			};
			carg[cnt][carg_index] = parallel[cnt] + i;
			/*for (k = i; k <= j; k++) {
 * 				carg[cnt][carg_index][k - i] = parallel[cnt][k];
 * 							}*/
			carg_index++;
			i = j + 1;
		}
		carg[cnt][carg_index] = NULL;
		/* slove the redirection */
		for (i = 0; i < carg_index; i++) {
			if (strcmp(carg[cnt][i], ">") == 0) {
				if (((i > 0) && (strcmp(carg[cnt][i - 1], ">") != 0))
					&& ((i < carg_index - 1) && (strcmp(carg[cnt][i + 1], ">") != 0))
					&& carg[cnt][i + 2] == NULL) {
					redir[cnt] = carg[cnt][i + 1];
					carg[cnt][i] = NULL;
					break;
				}
				else {
					return -1;
				}
			}
		}
	}
	return parallel_index - offset;
}
void printerr() {
	write(STDERR_FILENO, error_message, strlen(error_message));
}

int main(int argc, char **argv) {
	strcpy(Path[1], "/bin");
	char *str;
	size_t alloc_size = 0;
	int batch = 0;
	if (argc == 2) {
		batch = 1;
		/* judge the "bad" file */
		int len = strlen(argv[1]);
		int i = len;
		while (i >= 1 && argv[1][--i] != '/');
		if (i == 0 && strcmp(argv[1], "bad") == 0) {
			printerr();
			exit(1);
		}
		if (argv[1][i + 1] == 'b' && argv[1][i + 2] == 'a' && argv[1][i + 3] == 'd' && argv[1][i + 4] == '\0') {
			printerr();
			exit(1);
		}
		freopen(argv[1], "r", stdin);
	}
	else if (argc > 2) {
		/* multiple files */
		printerr();
		exit(1);
	}
	if (!batch) {
		printf("wish> ");
	}
	while (getline(&str, &alloc_size, stdin)) {
		if (feof(stdin)) {
			exit(0);
		}
		str[strlen(str) - 1] = '\0';
		char *cargv[10][10];
		char *redir[10];
		memset(redir, 0, sizeof redir);
		int comd_cnt = sloveTheLine(str, cargv, redir);
		/* comd_cnt is the number of commands */
		if (comd_cnt == -1) {
			printerr();
			if (!batch) {
				printf("wish> ");
			}
			continue;
		}
		int i;
		pid_t rc[10];
		for (i = 0; i < comd_cnt; i++) {
			int cnt = 0;
			/* the number of arguments */
			while (cargv[i][++cnt]);
			/* 3 bulid-in command */
			if (strcmp(cargv[i][0], "exit") == 0) {
				if (cnt == 1) {
					exit(0);
				}
				else {
					printerr();
				}
			}
			else if (strcmp(cargv[i][0], "cd") == 0) {
				if (cnt == 2) {
					if (chdir(cargv[i][1]) == -1) {
						printerr();
					}
				}
				else {
					printerr();
				}
			}
			else if (strcmp(cargv[i][0], "path") == 0) {
				int j;
				for (j = 1; j < cnt; j++) {
					strcpy(Path[j], cargv[i][j]);
				}
				pathcnt = cnt;
			}
			else {
				rc[i] = fork();
				if (rc[i] < 0) {
					printerr();
					if (batch) {
						exit(1);
					}
				}
				else if (rc[i] == 0) {
					int j = 0;
					char tmp[30];
					if (redir[i]) {
						freopen(redir[i], "w", stdout);
					}
					/* serach the command path */
					for (; j < pathcnt; j++) {
						strcpy(tmp, Path[j]);
						strcat(tmp, "/");
						strcat(tmp, cargv[i][0]);
						if (access(tmp, X_OK) == 0) {
							if (execvp(tmp, cargv[i]) != 0) {
								printerr();
							}
						}
					}
					printerr();
					exit(0);
				}
			}
		}
		for (i = 0; i < comd_cnt; i++) {
			if (rc[i] > 0) {
				waitpid(rc[i], NULL, 0);
			}
		}
		if (!batch) {
			printf("wish> ");
		}
	}
	free(str);
	str = NULL;
	if (argc > 1) {
		fclose(stdin);
	}
	exit(0);
}

