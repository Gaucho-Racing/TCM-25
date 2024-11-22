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

  SPI_init = spiOpen(1, 5000000, 0, 0, 8, 0, 1);
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

    tx[0] = 0x02;
    tx[1] = 0x03;
    tx[2] = 0x04;

    int i = 0;
    while (i<10){

        SPI_stat = spiXfer(SPI_init, tx, rx, 4);
        if (SPI_stat < 0)
            {
            /* SPI transfer failed */
            printf("SPI transfer failed. Error code:  %d\n", SPI_stat);
            exit(SPI_stat);
            }
        else
            {
            /* SPI transfer okay*/
            printf("SPI transfer OK. Return code:  %d\n", SPI_stat);
            printf("rx[0]: %d, rx[1]: %d, rx[2]: %d\n", rx[0], rx[1], rx[2]);
            }

        printf("tx0:%x --> rx0:%x\n",tx[0], rx[0]);
        printf("tx1:%x --> rx1:%x\n",tx[1], rx[1]);
        printf("tx2:%x --> rx2:%x\n",tx[2], rx[2]);
        printf("rx3:%x\n", rx[3]);
        
        i++;
        usleep(1000000);

    }

}