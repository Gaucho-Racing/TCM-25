#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <jetgpio.h>

/* CAN FD Controller */
#define cREGADDR_CiCON 0x000
#define cREGADDR_CiNBTCFG 0x004
#define cREGADDR_CiDBTCFG 0x008
#define cREGADDR_CiTDC 0x00C

#define cREGADDR_CiTBC 0x010
#define cREGADDR_CiTSCON 0x014
#define cREGADDR_CiVEC 0x018
#define cREGADDR_CiINT 0x01C
#define cREGADDR_CiINTFLAG cREGADDR_CiINT
#define cREGADDR_CiINTENABLE (cREGADDR_CiINT + 2)

#define cREGADDR_CiRXIF 0x020
#define cREGADDR_CiTXIF 0x024
#define cREGADDR_CiRXOVIF 0x028
#define cREGADDR_CiTXATIF 0x02C

#define cREGADDR_CiTXREQ 0x030
#define cREGADDR_CiTREC 0x034
#define cREGADDR_CiBDIAG0 0x038
#define cREGADDR_CiBDIAG1 0x03C

#define cREGADDR_CiTEFCON 0x040
#define cREGADDR_CiTEFSTA 0x044
#define cREGADDR_CiTEFUA 0x048
#define cREGADDR_CiFIFOBA 0x04C

#define cREGADDR_CiFIFOCON 0x050
#define cREGADDR_CiFIFOSTA 0x054
#define cREGADDR_CiFIFOUA 0x058
#define CiFIFO_OFFSET (3 * 4)

#define cREGADDR_CiTXQCON 0x050
#define cREGADDR_CiTXQSTA 0x054
#define cREGADDR_CiTXQUA 0x058

int main(int argc, char *argv[])
{
  int Init;
  int SPI_init;
  int SPI_stat;
  char tx[4] = {0,};
  char rx[4] = {0,};

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


    int i = 0;
    while (i<10){
        tx[0] = 0x02;
        tx[1] = 0x00;
        tx[2] = 0x04;
        SPI_stat = spiXfer(SPI_init, tx, rx, 3);
        if (SPI_stat < 0)
            {
            /* SPI transfer failed */
            printf("SPI transfer failed. Error code:  %d\n", SPI_stat);
            exit(SPI_stat);
            }
        else
            {
            /* SPI transfer okay*/
            printf("SPI write transfer OK. Return code:  %d\n", SPI_stat);
            }
        sleep(1);
        tx[0] = 0x03;
        tx[1] = 0x01;
        SPI_stat = spiXfer(SPI_init, tx, rx, 3);
        if (SPI_stat < 0)
            {
            /* SPI transfer failed */
            printf("SPI transfer failed. Error code:  %d\n", SPI_stat);
            exit(SPI_stat);
            }
        else
            {
            /* SPI transfer okay*/
            printf("SPI read transfer OK. Return code:  %d\n", SPI_stat);
            printf("rx[0]: %d, rx[1]: %d, rx[2]: %d\n", rx[0], rx[1], rx[2]);
            }
        printf("rx3:%x\n", rx[2]);
        
        rx[0] = 0;
        rx[1] = 0;
        rx[2] = 0;

        i++;
        usleep(1000000);

    }

}