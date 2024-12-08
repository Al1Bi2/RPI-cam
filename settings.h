#ifndef SETTINGS_H
#define SETTINGS_H
#include "utils.h"
void set_up_VGA(int handle){
    RPi_write_reg_I2C(handle,0x12,0x00);
    RPi_write_reg_I2C(handle,0x0C,0x00);
    RPi_write_reg_I2C(handle,0x3E,0x00);    
    RPi_write_reg_I2C(handle,0x70,0x3A);
    RPi_write_reg_I2C(handle,0x71,0x35);
    RPi_write_reg_I2C(handle,0x72,0x11);
    RPi_write_reg_I2C(handle,0x73,0xF0);
    RPi_write_reg_I2C(handle,0xA2,0x02);
    
    int reg0x12  = RPi_read_reg_I2C(handle,0x12);
    printf("reg0x12 %x\n",reg0x12);
    int reg0x0c  = RPi_read_reg_I2C(handle,0x0C);
    printf("reg0x0c %x\n",reg0x0c);
    int reg0x3e  = RPi_read_reg_I2C(handle,0x3E);
    printf("reg0x3e %x\n",reg0x3e);
    int reg0x70  = RPi_read_reg_I2C(handle,0x70);
    printf("reg0x70 %x\n",reg0x70);
    int reg0x71  = RPi_read_reg_I2C(handle,0x71);
    printf("reg0x71 %x\n",reg0x71);
    int reg0x72  = RPi_read_reg_I2C(handle,0x72);
    printf("reg0x72 %x\n",reg0x72);
    int reg0x73  = RPi_read_reg_I2C(handle,0x73);
    printf("reg0x73 %x\n",reg0x73);
    int reg0xa2  = RPi_read_reg_I2C(handle,0xA2);
    printf("reg0xa2 %x\n",reg0xa2);
}
typedef struct 
{
    uint8_t h_high;
    uint8_t h_low;
} hwindow_reg;
hwindow_reg convert_hwindow(uint16_t h){
    hwindow_reg res;
    res.h_high = (h >> 3) & 0xFF;
    res.h_low = h & 0x7;
    printf("h_high %x\n h_low %x\n",res.h_high,res.h_low);
    return res;
}
//136 - 786 
void set_up_window(int handle, uint16_t h_start, uint16_t h_end, uint16_t v_start, uint16_t v_end){
    hwindow_reg h_start_reg = convert_hwindow(h_start);
    hwindow_reg h_end_reg = convert_hwindow(h_end);

    int reg0x17  = RPi_read_reg_I2C(handle,0x17);
    printf("reg0x17 %x\n",reg0x17);
    int reg0x32  = RPi_read_reg_I2C(handle,0x32);
    printf("reg0x32 %x\n",reg0x32);
    int reg0x18  = RPi_read_reg_I2C(handle,0x18);
    printf("reg0x18 %x\n",reg0x18);

    int result = RPi_write_reg_I2C(handle,0x17,h_start_reg.h_high);
    RPi_write_reg_I2C(handle,0x18,h_end_reg.h_high);
    RPi_write_reg_I2C(handle,0x32,0x80 | (h_end_reg.h_low<<3) | h_start_reg.h_low);

     reg0x17  = RPi_read_reg_I2C(handle,0x17);
    printf("reg0x17 %x\n",reg0x17);
     reg0x32  = RPi_read_reg_I2C(handle,0x32);
    printf("reg0x32 %x\n",reg0x32);
     reg0x18  = RPi_read_reg_I2C(handle,0x18);
    printf("reg0x18 %x\n",reg0x18);

    
}

#endif // SETTINGS_H