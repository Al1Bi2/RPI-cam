#include <stdio.h>

#include <pigpio.h>

#ifndef UTILS_H
#define UTILS_H
extern volatile sig_atomic_t keep_running = 1;

static void sig_handler(int sig)
{
    if(sig  == SIGINT || sig == SIGSEGV){
    	keep_running = 0;
    	exit(EXIT_FAILURE);
    }
}

void setup_safe_sigint_handler(){
	struct sigaction sa;
	sa.sa_handler = sig_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	if(sigaction(SIGINT,&sa,NULL) == -1){
		perror("ERROR CANT HANDLE SIGINT");
		exit(EXIT_FAILURE);
	}
	if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("Error: Cannot handle SIGSEGV");
        exit(EXIT_FAILURE);
    }

}

#endif