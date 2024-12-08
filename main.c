#include <stdio.h>
#include <pigpio.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

#include "ov7670.h"
#include "i2c.h"
#include "utils.h"
#include "yuv_to_bmp.h"
#include "settings.h"

#define PCLK_PIN 17
#define VSYNC_PIN 22
#define HSYNC_PIN 27

#define DATA_PIN_0 -1
#define DATA_PIN_1 -1
#define DATA_PIN_2 -1
#define DATA_PIN_3 -1
#define DATA_PIN_4 21
#define DATA_PIN_5 20
#define DATA_PIN_6 10
#define DATA_PIN_7 9

#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480
#define BUFFER_SIZE IMAGE_WIDTH*IMAGE_HEIGHT*20

//clear array
#define CLEAR(x) memset(x,0,sizeof(x))



typedef struct {
	unsigned char pin;
    unsigned int tick;
    unsigned char level;
} PinDataBuffer;

volatile PinDataBuffer data_buffer[BUFFER_SIZE];
volatile unsigned  long write_index={0};  // Индексы для записи в буферы данных
volatile unsigned  long read_index = {0};             // Индексы для чтения, обновляем при чтении в PCLK

unsigned char image[IMAGE_HEIGHT][IMAGE_WIDTH];
volatile int capturing = 0;
volatile int pixel_x = 0, pixel_y = 0;

void print_buffer_info();
int j=0;
// Колбэк для сбора данных с каждого пина
void data_callback(int level, uint32_t tick, int pin_index) {
		data_buffer[write_index].tick = tick;
		data_buffer[write_index].level = level;
		data_buffer[write_index].pin = pin_index;
		write_index +=1;
		
}
// Колбэки для каждого пина данных
void data0_callback(int gpio, int level, uint32_t tick) { data_callback( level, tick, 0); }
void data1_callback(int gpio, int level, uint32_t tick) { data_callback( level, tick, 1); }
void data2_callback(int gpio, int level, uint32_t tick) { data_callback( level, tick, 2); }
void data3_callback(int gpio, int level, uint32_t tick) { data_callback( level, tick, 3); }
void data4_callback(int gpio, int level, uint32_t tick) { data_callback( level, tick, 4); }
void data5_callback(int gpio, int level, uint32_t tick) { data_callback( level, tick, 5); }
void data6_callback(int gpio, int level, uint32_t tick) { data_callback( level, tick, 6); }
void data7_callback(int gpio, int level, uint32_t tick) { data_callback( level, tick, 7); }

const signed char DATAPINS[8] = {DATA_PIN_0, DATA_PIN_1,DATA_PIN_2, DATA_PIN_3, DATA_PIN_4, DATA_PIN_5, DATA_PIN_6, DATA_PIN_7};
typedef void (*f)(int, int,uint32_t);  
const f DATACALLBACKS[8]={&data0_callback, &data1_callback, &data2_callback, &data3_callback, &data4_callback, &data5_callback, &data6_callback, &data7_callback};

int build_byte(int pins[8]);
void buffer_to_image();



volatile int i=0;
volatile int pclk_last = 1;
// Колбэк для PCLK
void pclk_callback(int gpio, int level, uint32_t tick) {
    i++;
    if (level == 0) {
     //data_callback(level,tick,10);
    }else{
	data_callback(level,tick,10);
		
	}
}

// Колбэк для HSYNC
void hsync_callback(int gpio, int level, uint32_t tick) {
    if (level == 1 ) {
 		data_callback(level,tick,11);
    }
}
 clock_t start, end;
struct timespec start_n, end_n;
volatile uint32_t vsync_last = 1;
// Колбэк для VSYNC
void vsync_callback(int gpio, int level, uint32_t tick) {
    if (level ==1){
        vsync_last = tick;
    }
    if (level == 0) {
     if(tick - vsync_last < 500) return;
     data_callback(level, tick, 12);
     write_index = 0;
    	if(capturing==0){
    		capturing  = 1;
            start = clock();
            clock_gettime(CLOCK_MONOTONIC, &start_n); 

    		//return;
    	}else{
    		capturing  = 0;
            end = clock();
            clock_gettime(CLOCK_MONOTONIC, &end_n); 
    	}
    	
		//print_buffer_info();
		//printf("%d",pixel_y);
		
    }
}


void I2C_settings(){
    printf("START I2C\n");
	int handle  = i2cOpen(1,0x21,0);
	printf("Open Result: %d\n",handle);
	int regwrite  = RPi_write_reg_I2C(handle,0x12,0x80);
    time_sleep(0.01); // Wait for the device to be ready

    int reg0x11  = RPi_read_reg_I2C(handle,0x11);
	printf("reg0x11 %x\n",reg0x11);
    regwrite  = RPi_write_reg_I2C(handle,0x11,0x9F);  //freqency down 32 times
	reg0x11  = RPi_read_reg_I2C(handle,0x11);
    printf("reg0x11 %x\n",reg0x11);

    RPi_write_reg_I2C(handle,0x15,0x20); // no pclk when no href


    set_up_VGA(handle);

    //set_up_window(handle,143,583,0,0);

	printf("CLOSING\n");
	i2cClose(handle);
	printf("CLOSING\n");
}

void set_up_pins(){
    gpioCfgBufferSize(100);
	gpioCfgClock(1,1,1);
	printf("WTF");
	if (gpioInitialise() < 0) {
        perror("ERROR: Can't init pigpio");
        exit(EXIT_FAILURE);
    }
    printf("INIT\n");
    gpioSetMode(PCLK_PIN, PI_INPUT);
    gpioSetMode(VSYNC_PIN, PI_INPUT);
    gpioSetMode(HSYNC_PIN, PI_INPUT);
    
    gpioSetAlertFunc(PCLK_PIN, pclk_callback);
    gpioSetAlertFunc(VSYNC_PIN, vsync_callback);
    gpioSetAlertFunc(HSYNC_PIN, hsync_callback);
    gpioHardwarePWM(12, 8000000, 500000);
    gpioDelay(1000);
    
    for(int i = 0; i<8;i++){
		if(DATAPINS[i]!=-1){
			gpioSetMode(DATAPINS[i], PI_INPUT);
			gpioSetAlertFunc(DATAPINS[i], DATACALLBACKS[i]);
			
		}
    }
	printf("Finished setting upm pins\n");

}
void cleanup(){
    gpioHardwarePWM(12, 0, 0);
    gpioDelay(10000);
    gpioTerminate();
}
void do_it(){
 printf("Start\n");
    atexit(cleanup);
    printf("set atexit\n");
	setup_safe_sigint_handler();

	set_up_pins();
    I2C_settings();
    
    printf("start\n");
    
    time_sleep(1);

	printf("start\n");
    printf("Начало захвата изображения...\n");
    capturing=1;
    printf("Start capturing");
    while (capturing && keep_running) {
        gpioDelay(2);
    }
    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Процессорное время исполнения: %.6f секунд\n", elapsed_time);

    elapsed_time = (end_n.tv_sec - start_n.tv_sec) +
                          (end_n.tv_nsec - start_n.tv_nsec) / 1e9;
    printf("Реальное время исполнения: %.6f секунд\n", elapsed_time);

    printf("Захват завершён. Обработка изображения...\n");
    buffer_to_image();
    save_as_bmp("output.bmp", image,IMAGE_WIDTH,IMAGE_HEIGHT);
    //cleanup();
}
void test_bmp(){
 // Пример заполнения изображения тестовыми данными (градиент яркости)
    unsigned char image[IMAGE_HEIGHT][IMAGE_WIDTH];
    for (int y = 0; y < IMAGE_HEIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            image[y][x] = (unsigned char)((x + y) % 256);
        }
    }

    // Сохранение в файл
    save_as_bmp("output.bmp", image,IMAGE_WIDTH,IMAGE_HEIGHT);

    printf("Image saved as output.bmp\n");
}

int main() {
    do_it();
    //test_bmp();

   
    return 0;
}

int build_byte(int pins[8]){
    int byte = 0;
    for(int i = 0;i<8;i++){
        byte += pins[i]<<(i);
    }
    return byte;
}

void buffer_to_image(){
    int pins[8] = {0};
    float px = 0;
    float py = 0;
    pixel_x=0;
    pixel_y=0;
    char byte = 0;
    printf("Startread");
    for(int i = 0;i<BUFFER_SIZE;i++){
        //printf("+%d\n",i);
        PinDataBuffer event = data_buffer[i];
        if(event.pin<8){
            pins[event.pin] = event.level;
            //printf("%d!",pins[0]);
        }
        if(event.pin==10){
            if(byte == 1){
                image[pixel_y][pixel_x] = build_byte(pins);
                int out  = build_byte(pins);
                //printf("%d-%d|",pixel_x,out);
                pixel_x++;

            }
            byte++;
            byte = byte%2;     
        }
            if(event.pin==11){
           //printf("%d\n",pixel_x);
            px+=pixel_x;
            if(pixel_x!=IMAGE_WIDTH){
                //printf("!%d\n",pixel_x);
            }
            pixel_x=0;
            pixel_y++;
            byte = 0;
        }
        if(event.pin==12){
            printf("!!!!!%d",pixel_y);
            printf("|%u\n",event.tick);
            py+=pixel_y;
            px = px/pixel_y;
            printf("%f %f\n",px,py);
            px=0;
            py=0;

            pixel_x=0;
            pixel_y=0;
            byte = 0;
            //return;
        }
    }

    //read_index = 0;  // Сбрасываем read_index для следующего кадра
    write_index = 0; // Очищаем write_index для нового захвата
}

