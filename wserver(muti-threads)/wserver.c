#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include "threadpool.h"
#include "pthread.h"

char default_root[] = ".";

//
// ./wserver [-d <basedir>] [-p <portnum>] 
// 

void task(void *args) {
	printf("connection fd %d is working, tid = %ld\n", *((int*)args), pthread_self());
	int conn_fd = *(int*)args;
	request_handle(conn_fd);
	close_or_die(conn_fd);
}
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
    
    while ((c = getopt(argc, argv, "d:p:")) != -1)
		switch (c) {
		case 'd':
			root_dir = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		default:
			fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
			exit(1);
		}

    // run out of this directory
    chdir_or_die(root_dir);

	ThreadPool *pool = threadPoolCreate(3, 10, 100);

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
    while (1) {
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int *conn_fd = (int*)malloc(sizeof(int));
		*conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
		addWork(pool, task, conn_fd);
    }
    return 0;
}


    


 
