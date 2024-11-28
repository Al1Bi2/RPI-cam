#include <stdio.h>

#include <pigpio.h>

#ifndef I2C_H
#define I2C_H
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

int16_t RPi_read_reg_I2C(unsigned int handle,  uint8_t addr){
    int16_t response;
    response = i2cWriteByte(handle, addr);
    if(response < 0){
        fprintf(stderr, "i2cWriteByte in read_reg_I2C failed\n");
        return -1;
    }
    gpioDelay(1000);
    response = i2cReadByte(handle);
    if(response < 0){
        fprintf(stderr, "i2cReadByte in read_reg_I2C failed\n");
        return -1;
    }
    return response;
}

int16_t RPi_write_reg_I2C(unsigned int handle, uint8_t addr, uint8_t data){
    int16_t response;
    response = i2cWriteByteData(handle, addr, data);
    if(response < 0){
        fprintf(stderr, "i2cWriteByte in write_reg_I2C failed\n");
        return -1;
    }

    return response;
}

#endif