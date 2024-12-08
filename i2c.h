#include <pigpio.h>

#ifndef I2C_H
#define I2C_H

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

#endif // I2C_H