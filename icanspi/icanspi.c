#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <jetgpio.h>
#include <jetgpio.h>
#include <signal.h>

static volatile int interrupt = 1;
long unsigned int timestamp;
volatile int temp = 0;
int SPI_init;

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
    char tx[8] = {0,};
    char rx[8] = {0,};
    spiXfer(SPI_init, tx, rx, 8);
    //cases the chat into uint 8 
    //this is used to get the length of the message
    //temp var for tx
    for(int i = 0; i < 8; i++){
        printf("%02x ", rx[i]);
    }
    printf("\n");
    char txTemp[64] = {0x69,};
    uint8_t length = (uint8_t)(rx[5]);
    printf("%d\n", length);
    spiXfer(SPI_init, txTemp, (char *)frame.combined.split.data, length);

    temp++;
    printf("%d\n", temp);
    for(int i = 0; i < length; i++){
        printf("%x ", frame.combined.split.data[i]);
    }
        printf("\n");
    //printf("edge detected with EPOCH timestamp: %lu\n", timestamp);
    // terminating while loop
    //interrupt = 0;
}

int main(int argc, char *argv[])
{
    int Init;
    int status;
    signal(SIGINT, inthandler);
    Init = gpioInitialise();
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
    status = setCB(29, FALLING_EDGE, 1000, &timestamp, &calling);
    printf("%d\n", status);
    while(interrupt){
        sleep(1);
    }

    spiClose(SPI_init);
    gpioTerminate();
    exit(0);


   
}

