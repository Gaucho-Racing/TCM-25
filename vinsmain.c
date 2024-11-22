#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <jetgpio.h>

int main(int argc, char *argv[])
{
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

  /* Port SPI2 pins: 37, 22, 13 & 18
     to perform a simple loop test, pins 22: SPI2_MISO & 37:SPI2_MOSI should be connected
     with a short jumper cable, when the cable is disconnected the output on screen will show
     all the rx values as zeros 
     spiOpen() parameters go as follows: spiOpen(port number, speed in Hz, mode, cs pin delay in us, 
     bits per word, least significant bit first, cs change)
  */

  SPI_init = spiOpen(1, 5000000, 0, 0, 8, 1, 1);
  if (SPI_init < 0)
    {
      /* Port SPI2 opening failed */
      printf("Port SPI2 opening failed. Error code:  %d\n", SPI_init);
      exit(Init);
    }
  else
    {
      /* Port SPI2 opened  okay*/
      printf("Port SPI2 opened OK. Return code:  %d\n", SPI_init);
    }

}