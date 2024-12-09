CC=aarch64-linux-gnu-gcc
TARGET=cam
SRC=main.c
RPI_PROFILE=repka
RPI_PATH=/home/pi/DIPLOM/ov7670
LOCAL_PATH=./
CFLAGS = -I/home/pigpio-master
LDFLAGS = -L/home/pigpio-master -lpigpio -lrt

# Compile the source code
all: $(SRC)
	$(CC) -O3 $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

# Deploy the binary to the Raspberry Pi
deploy: all
	scp $(TARGET) $(RPI_PROFILE):$(RPI_PATH)

# Run the program on the Raspberry Pi
run:deploy
	ssh $(RPI_PROFILE) "cd $(RPI_PATH) && sudo  ./$(TARGET)"

debug:deploy
	ssh $(RPI_PROFILE) "cd $(RPI_PATH) && sudo gdb ./$(TARGET)"

get:
	scp $(RPI_PROFILE):$(RPI_PATH)/output.bmp $(LOCAL_PATH)