#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <jetgpio.h>

// SPI Command Definitions for MCP2518FD
#define CMD_RESET 0xC0
#define CMD_WRITE 0x02
#define CMD_READ 0x03

// MCP2518FD Registers (example addresses, consult the datasheet for details)
#define REG_CiCON 0x000
#define REG_CiNBTCFG 0x004
#define REG_CiFIFOCON1 0x05C
#define REG_CiFIFOSTA1 0x060
#define REG_CiFIFOUA1 0x064

int main(int argc, char *argv[])
{
    int Init;
    int SPI_init;
    int SPI_stat;
    char tx[16] = {0,}; // Adjusted size for larger SPI transactions
    char rx[16] = {0,};

    Init = gpioInitialise();
    if (Init < 0)
    {
        printf("Jetgpio initialisation failed. Error code: %d\n", Init);
        exit(Init);
    }
    else
    {
        printf("Jetgpio initialisation OK. Return code: %d\n", Init);
    }

    SPI_init = spiOpen(1, 10000000, 0, 0, 8, 1, 1);
    if (SPI_init < 0)
    {
        printf("Port SPI2 opening failed. Error code: %d\n", SPI_init);
        gpioTerminate();
        exit(SPI_init);
    }
    else
    {
        printf("Port SPI2 opened OK. Return code: %d\n", SPI_init);
    }

    // Reset MCP2518FD
    tx[0] = CMD_RESET;
    SPI_stat = spiXfer(SPI_init, tx, rx, 1);
    if (SPI_stat < 0)
    {
        printf("SPI transfer failed during reset. Error code: %d\n", SPI_stat);
        spiClose(SPI_init);
        gpioTerminate();
        exit(SPI_stat);
    }
    else
    {
        printf("MCP2518FD reset command sent.\n");
    }
    usleep(10000); // Wait for reset to complete

    // Example: Write to a register (CiCON)
    tx[0] = CMD_WRITE;
    tx[1] = (REG_CiCON >> 8) & 0xFF; // High byte of register address
    tx[2] = REG_CiCON & 0xFF;        // Low byte of register address
    tx[3] = 0x80;                    // Set Configuration Mode
    tx[4] = 0x00;                    // Clear other fields
    tx[5] = 0x00;
    tx[6] = 0x00;
    SPI_stat = spiXfer(SPI_init, tx, rx, 7);
    if (SPI_stat < 0)
    {
        printf("SPI transfer failed during register write. Error code: %d\n", SPI_stat);
        spiClose(SPI_init);
        gpioTerminate();
        exit(SPI_stat);
    }
    else
    {
        printf("Register CiCON configured for Configuration mode.\n");
    }

    // Example: Read from a register (CiCON)
    tx[0] = CMD_READ;
    tx[1] = (REG_CiCON >> 8) & 0xFF;
    tx[2] = REG_CiCON & 0xFF;
    memset(&tx[3], 0, 4); // Clear remaining bytes for read
    SPI_stat = spiXfer(SPI_init, tx, rx, 7);
    if (SPI_stat < 0)
    {
        printf("SPI transfer failed during register read. Error code: %d\n", SPI_stat);
        spiClose(SPI_init);
        gpioTerminate();
        exit(SPI_stat);
    }
    else
    {
        printf("Read CiCON Register: 0x%02X%02X%02X%02X\n", rx[3], rx[4], rx[5], rx[6]);
    }

    // Close SPI port
    spiClose(SPI_init);

    // Terminate library
    gpioTerminate();

    printf("Program completed successfully.\n");
    exit(0);
}
