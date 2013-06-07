#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	long num_of_children = argc>1 ? atol(argv[1]) : 5;

	pid_t child_id;
	for(int i = 0; i< num_of_children; i++){
		switch ( (child_id = fork()) ) {	
		case -1:
			perror("forking\n");
			exit(-1);
	
		case 0: 	
			execlp("sleeper", "sleeper", (char*)NULL);
			perror("sleeper"); //will never be called if exec works
			exit(-1);
			break;
	
		default:
			break;
		}
	}
	for (int i=0; i<num_of_children; i++) {
		int status;
	    wait(&status);
	}
	return 0;
}

