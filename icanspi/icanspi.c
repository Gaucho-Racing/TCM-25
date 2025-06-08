#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <jetgpio.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "circularBuffer.h"

static volatile int interrupt = 1;
long unsigned int timestamp;
volatile int temp = 0;
int SPI_init;
CircularBuffer *cb = NULL;

struct CAN{
    union{
      uint16_t buffer[35];
      struct{
        uint32_t ID;
        uint8_t bus;
        uint8_t length;
        uint8_t data[64];
      }split;
    }combined;
  };

void inthandler(int signum) 
{
  usleep(1000);
  printf("\nCaught Ctrl-c, coming out ...\n");
  interrupt = 0;
}



int startSPI(int port, int speed, int mode, int lsb){
    int SPI_init;
    SPI_init = spiOpen(port, speed, mode, 0, 8, lsb, 1);
    if (SPI_init < 0)
    {
        printf("Port SPI2 opening failed. Error code:  %d\n", SPI_init);
        exit(1);
    }
    else
    {
        printf("Port SPI2 opened OK. Return code:  %d\n", SPI_init);
    }
    return SPI_init;
}

int spiTransfer(int handle, uint8_t *txBuf, uint8_t *rxBuf, unsigned len){
    int SPI_stat;
    SPI_stat = spiXfer(handle, (char *)txBuf, (char *)rxBuf, len);
    if (SPI_stat < 0)
    {
        printf("SPI transfer failed. Error code:  %d\n", SPI_stat);
        exit(1);
    }
    else
    {
        printf("SPI transfer OK. Return code:  %d\n", SPI_stat);
    }
    return SPI_stat;
}

int enableGPIO(int pin, int mode){
    int stat = gpioSetMode(pin, mode);
    if (stat < 0)
    {
        printf("GPIO setting up failed. Error code:  %d\n", stat);
        exit(1);
    }
    else
    {
        printf("GPIO setting up OK. Return code:  %d\n", stat);
    }
    return stat;
}

int setCB(int pin, int edge, int delay, long unsigned int *timestamp, void *calling){
    int stat2 = gpioSetISRFunc(pin, edge, delay, timestamp, calling);
    if (stat2 < 0)
    {
        printf("GPIO edge setting up failed. Error code:  %d\n", stat2);
        exit(1);
    }
    else
    {
        printf("GPIO edge setting up OK. Return code:  %d\n", stat2);
    }
    return stat2;
}

void calling()
{
    struct CAN frame = {0,};
    char tx[6] = {0,};
    char rx[6] = {0,};
    spiXfer(SPI_init, tx, rx, 6);
    //cases the chat into uint 8 
    //this is used to get the length of the message
    //temp var for tx
    uint32_t temp  = 0;
    temp |= (uint32_t)(rx[1]);
    temp |= ((uint32_t)(rx[0]) << 8);
    temp |= (uint32_t)(rx[3] << 16);
    temp |= (uint32_t)(rx[2] << 24);

    frame.combined.split.ID =  temp;
    frame.combined.split.bus = rx[5];
    frame.combined.split.length = rx[4]; 
    char txTemp[64] = {0x69,};
    uint8_t length = (uint8_t)(rx[4]);
    // circularBufferPush(cb, frame.combined.buffer, sizeof(frame.combined.buffer));
    
    spiXfer(SPI_init, txTemp, (char *)frame.combined.split.data, length);
    circularBufferPush(cb, frame.combined.buffer, sizeof(frame.combined.buffer));
    //printf("edge detected with EPOCH timestamp: %lu\n", timestamp);
    // terminating while loop
    //interrupt = 0;
}

int main(int argc, char *argv[])
{
    int Init;
    int status;
    cb = circular_buffer_init(64, 72);
    signal(SIGINT, inthandler);
    Init = gpioInitialise();
    uint32_t counter = 0;
    if (Init < 0)
    {
        printf("Jetgpio initialisation failed. Error code:  %d\n", Init);
        exit(Init);
    }
    else
    {
        printf("Jetgpio initialisation OK. Return code:  %d\n", Init);
    }
    SPI_init = startSPI(1, 500000, 0, 0);
    status = enableGPIO(29, JET_INPUT);

    /*
    BK stuff
    */

    int sockfd;
    struct sockaddr_in servaddr;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
     
    // Set server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi("8000"));
    servaddr.sin_addr.s_addr = inet_addr("0.0.0.0");

    /*
    END BK stuff
    */

    //enable interupt after everything is good.
    status = setCB(29, FALLING_EDGE, 1000, &timestamp, &calling);
    printf("%d\n", status);
    while(interrupt){
        struct CAN *frame = circularBufferPop(cb);
        if(frame != NULL){
            counter = 0;
            printf("LENGTH: %d\n", frame->combined.split.length);
            printf("ID: %x\n", frame->combined.split.ID);
            printf("BUS: %x\n", frame->combined.split.bus);
            printf("LOAD: %d\n", cb->size);
            printf("|=====|\n");
            if (sendto(sockfd, frame, sizeof(struct CAN), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
                perror("Send failed, message lost");
                continue;
            }
        } else if(counter >= 2000){
            printf("STATE: RESET\n");
            if (gpioRead(29) == 0){
                calling();
                printf("reset2\n");
            }
            counter = 0;
        } else {
            usleep(500);
            counter ++;
        }
    }
    spiClose(SPI_init);
    gpioTerminate();
    exit(0);
}

