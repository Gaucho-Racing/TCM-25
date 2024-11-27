#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <jetgpio.h>



/* CAN FD Controller */

int main(int argc, char *argv[])
{
  int Init;
  int SPI_init;
  int SPI_stat;
  char tx[2] = {0,};
  char rx[2] = {0,};

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

  SPI_init = spiOpen(1, 500000, 0, 10, 8, 0, 1);

  tx[0] = 0x00;
  tx[1] = 0x00;
  SPI_stat = spiXfer(SPI_init, tx, rx, 2);
  
  tx[0] = 0x30;
  tx[1] = 0x00;
  
  SPI_stat = spiXfer(SPI_init, tx, NULL, 2);
  SPI_stat &= spiXfer(SPI_init, NULL, rx, 1);

    if (SPI_stat < 0)
      {
	/* Spi transfer failed */
	printf("Spi port transfer failed. Error code:  %d\n", SPI_stat);
	exit(Init);
      }
    else
      {
	/* Spi transfer okay*/
	printf("Spi port transfer OK. Return code:  %d\n", SPI_stat);
      }

  printf("tx0:%x\n", tx[0]);
  printf("tx1:%x\n", tx[1]);
  printf("rx0:%x\n", rx[0]);
  printf("rx1:%x\n", rx[1]);

}