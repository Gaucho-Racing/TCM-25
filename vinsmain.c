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

// Helper function to send SPI commands
int spiWrite(int spiHandle, uint16_t reg, uint8_t *data, size_t len) {
    uint8_t tx[len + 3];
    tx[0] = CMD_WRITE;
    tx[1] = (reg >> 8) & 0xFF;
    tx[2] = reg & 0xFF;
    memcpy(&tx[3], data, len);

    char rx[len + 3];
    return spiXfer(spiHandle, tx, rx, len + 3);
}

int spiRead(int spiHandle, uint16_t reg, uint8_t *data, size_t len) {
    uint8_t tx[len + 3];
    tx[0] = CMD_READ;
    tx[1] = (reg >> 8) & 0xFF;
    tx[2] = reg & 0xFF;
    memset(&tx[3], 0, len);

    char rx[len + 3];
    int result = spiXfer(spiHandle, tx, rx, len + 3);
    if (result >= 0) {
        memcpy(data, &rx[3], len);
    }
    return result;
}

int main(int argc, char *argv[]) {
    int Init;
    int SPI_init;

    Init = gpioInitialise();
    if (Init < 0) {
        printf("Jetgpio initialization failed. Error code: %d\n", Init);
        exit(Init);
    }
    printf("Jetgpio initialization OK. Return code: %d\n", Init);

    SPI_init = spiOpen(1, 10000000, 0, 0, 8, 1, 1);
    if (SPI_init < 0) {
        printf("SPI port opening failed. Error code: %d\n", SPI_init);
        gpioTerminate();
        exit(SPI_init);
    }
    printf("SPI port opened OK. Return code: %d\n", SPI_init);

    // Reset MCP2518FD
    uint8_t resetCmd = CMD_RESET;
    spiXfer(SPI_init, &resetCmd, NULL, 1);
    usleep(10000); // Wait for reset to complete

    // Configure CAN (set Configuration mode and other settings)
    uint8_t configMode[4] = {0x80, 0x00, 0x00, 0x00};
    spiWrite(SPI_init, REG_CiCON, configMode, 4);

    // Configure FIFO1 as RX FIFO
    uint8_t fifocon1[4] = {0x00, 0x00, 0x00, 0x80}; // RXTSEN enabled
    spiWrite(SPI_init, REG_CiFIFOCON1, fifocon1, 4);

    // Set Normal Operation mode
    uint8_t normalMode[4] = {0x00, 0x00, 0x00, 0x00};
    spiWrite(SPI_init, REG_CiCON, normalMode, 4);

    printf("MCP2518FD configured for normal operation. Listening for CAN messages...\n");

    // Loop to receive messages
    while (1) {
        uint8_t fifoStatus[4];
        spiRead(SPI_init, REG_CiFIFOSTA1, fifoStatus, 4);

        if (fifoStatus[0] & 0x01) { // TFNRFNIF: FIFO not empty
            uint8_t fifoAddr[4];
            spiRead(SPI_init, REG_CiFIFOUA1, fifoAddr, 4);

            uint8_t rxData[16];
            spiRead(SPI_init, fifoAddr[0], rxData, 16);

            uint32_t id = (rxData[1] << 3) | (rxData[2] >> 5); // Extract Standard ID
            printf("Message ID: 0x%03X\n", id);
            printf("Data: ");
            for (int i = 0; i < 8; i++) {
                printf("0x%02X ", rxData[4 + i]);
            }
            printf("\n");

            // Increment FIFO to acknowledge read
            uint8_t uincCmd[4] = {0x01, 0x00, 0x00, 0x00};
            spiWrite(SPI_init, REG_CiFIFOCON1, uincCmd, 4);
        }

        usleep(1000); // Poll every 1 ms
    }

    // Close SPI port
    spiClose(SPI_init);

    // Terminate library
    gpioTerminate();

    printf("Program completed successfully.\n");
    return 0;
}
