#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <jetgpio.h>
#include <unistd.h>


#define SPI_BUS 1
#define CS_PIN 24
#define SPI_SPEED 5000000 // 5 MHz
#define RX_FIFO_ADDRESS 0x500
#define RX_FIFO_CONTROL_ADDRESS 0x580
#define BUFFER_SIZE 64

// MCP2518FD SPI instructions
#define RESET 0x00
#define WRITE 0x02
#define READ 0x03

// MCP2518FD Register Addresses
#define REG_CiCON 0x000



int main(int argc, char *argv[]) {
  int Init;
  int SPI_init;
  int SPI_stat;
  char tx[7] = {0,};
  char rx[7] = {0,};

  Init = gpioInitialise();
  if (Init < 0)
    {
      /* jetgpio initialisation failed */
      printf("Jetgpio initialisation failed. Error code:  %d\n", Init);
      exit(Init);
    }
  else
    {
      /* jetgpio initialised okay*/
      printf("Jetgpio initialisation OK. Return code:  %d\n", Init);
    }

    SPI_init = spiOpen(1, 500000, 0, 0, 8, 1, 1);
    if (SPI_init < 0)
    {
      /* SPI initialisation failed */
      printf("SPI initialisation failed. Error code:  %d\n", SPI_init);
      exit(Init);
    }
    else
    {
      /* SPI initialised okay*/
      printf("SPI initialisation OK. Return code:  %d\n", SPI_init);
    }
}

