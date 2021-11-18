#include "ledgrid.h"
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>

static bool loop = true;

void loop_exit (int sig) {
	loop = false;
}	

int main () {
	struct sigaction act;
	act.sa_handler = loop_exit;
	act.sa_flags = 0;
	int s = sigaction(SIGINT, &act, NULL);
	if (s != 0) {
		printf("Error installing signal handler, aborting.\n");
		return 1;
	}

	printf("Initializing grid...");
   	s = grid_init();
	if (s == -1) {
		printf("initialization failed.\n");
		return 1;
	}
	printf("done.\n");

	while (loop) {
		printf("\nled> ");
		int addr = 0;
		int s = scanf("%d", &addr);
		if (s != 1 || addr < 0 || addr > 15) {
			printf("Please provide a decimal integer from 0-15.\n");
			continue;
		}

		s = grid_select(addr);
		if (s == -1) {
			printf("Uh oh! There was an error selecting address %x. I'm going to exit now.\n", addr);
			loop = false;
		} else {
			printf("Selected address %x\n", addr);
		}
	}

	printf("Releasing grid...");
	grid_free();
	printf("done.\n");
	return 0;
}

