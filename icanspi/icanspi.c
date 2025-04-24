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
int SPI_init;
volatile uint8_t jetOut[8] = {0x00};
volatile bool jetOut_rdy = FALSE;
volatile CAN_rdy = FALSE;
volatile struct CAN frame = {0,};


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
    char tx[8] = {0,};
    char rx[8] = {0,};
    //check to see if there is data availible
    //if it is trasmit, otherwise transmit some fuck ass 0 val
    //first spixfer is meant to get the params for the second spiXfer
    if(jetOut_rdy){
        spiXfer(SPI_init, jetOut, rx, 8);
        jetOut_rdy = FALSE;
    }else{
        spiXfer(SPI_init, tx, rx, 8);
    }
    //txTemp is a temporary buffer for transmit that handles the maximum
    // amount of data that can be sent via a can tx
    //reason we do a rx and a tx is because the jetson spi 
    //controller has no support for slave, and as such we do 
    //a software slave implementation
    char txTemp[64] = {0x69,};
    uint8_t length = (uint8_t)(rx[5]);
    spiXfer(SPI_init, txTemp, (char *)frame.combined.split.data, length);
    //now the data is in frame CAN struct
    //this then needs to be sent to the query.
    //we do the sending in main in downtime.
    CAN_rdy = true;
}

void sendCAN(struct CAN frame){
    printf("TODO\n");
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
    //just a sleep function or the "main loop" in the main loop itslef lol
    while(interrupt){
        /*
        Here we can adjust the jet out
        and then on next interypt 
        */
        /*
        take in data from socket then have it copied over to the tx buffer
        */
        uint8_t SocketParams[8] = {0x00};
        //check if we have sent the data
        if(!jetOut_rdy){
            memcpy(jetOut, SocketParams, sizeof(uint8_t) * 8);
            jetOut_rdy = TRUE;
        }
        if(CAN_rdy){
            sendCAN(frame);
            CAN_rdy = FALSE;
        }
        
    }

    spiClose(SPI_init);
    gpioTerminate();
    exit(0);


   
}

