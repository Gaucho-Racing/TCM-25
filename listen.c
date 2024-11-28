#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <jetgpio.h>

// Constants
#define MCP2518FD_SPI_PORT 1
#define SPI_SPEED 10000000
#define SPI_MODE 0

#define READ_CMD 0x30
#define WRITE_CMD 0x20

#define C1FIFOSTA1 0x060 // FIFO Status Register
#define C1FIFOUA1 0x064  // FIFO User Address Register
#define RX_FIFO 0x030    // RX FIFO Base Address
#define C1INT 0x01C      // Interrupt Register

// Function prototypes
void write_register(int spi_handle, uint16_t reg, uint8_t value);
uint8_t read_register_byte(int spi_handle, uint16_t reg);
void read_can_message(int spi_handle);

// Write a single byte to a register
void write_register(int spi_handle, uint16_t reg, uint8_t value) {
    uint8_t tx[3] = { 0 };

    tx[0] = WRITE_CMD | ((reg >> 8) & 0x0F); // Write command and high 4 bits of address
    tx[1] = reg & 0xFF;                     // Low 8 bits of address
    tx[2] = value;                          // Data byte to write

    spiXfer(spi_handle, tx, NULL, sizeof(tx));
}

// Read a single byte from a register
uint8_t read_register_byte(int spi_handle, uint16_t reg) {
    uint8_t tx[3] = { READ_CMD | ((reg >> 8) & 0x0F), reg & 0xFF, 0x00 };
    uint8_t rx[3] = { 0 };

    spiXfer(spi_handle, tx, rx, sizeof(rx));
    return rx[2];
}

// Read a CAN message from RX FIFO
void read_can_message(int spi_handle) {
    uint8_t header[4]; // Message header
    uint8_t payload[64]; // Maximum payload size for CAN FD
    uint8_t dlc;
    uint32_t id;
    
    while (1) {
        uint8_t fifo_status = read_register_byte(spi_handle, C1FIFOSTA1);
        if (fifo_status & 0x01) { // RX Full flag
            // Read the first 4 bytes (header)
            for (int i = 0; i < 4; i++) {
                header[i] = read_register_byte(spi_handle, RX_FIFO + i);
            }

            // Extract message ID and DLC
            id = ((header[0] << 24) | (header[1] << 16) | (header[2] << 8) | header[3]);
            dlc = header[3] & 0x0F; // Lower nibble contains DLC

            printf("Message ID: 0x%08X, DLC: %u\n", id, dlc);

            // Read the payload
            for (int i = 0; i < dlc; i++) {
                payload[i] = read_register_byte(spi_handle, RX_FIFO + 4 + i);
            }

            printf("Payload:");
            for (int i = 0; i < dlc; i++) {
                printf(" %02X", payload[i]);
            }
            printf("\n");

            // Acknowledge the message (update FIFO user address)
            write_register(spi_handle, C1FIFOUA1, 0x00);
        }

        usleep(1000); // Polling delay
    }
}

int main() {
    int Init = gpioInitialise();
    if (Init < 0) {
        printf("GPIO initialization failed.\n");
        return Init;
    }

    int spi_handle = spiOpen(MCP2518FD_SPI_PORT, SPI_SPEED, SPI_MODE, 0, 8, 0, 1);
    if (spi_handle < 0) {
        printf("SPI initialization failed.\n");
        gpioTerminate();
        return spi_handle;
    }

    printf("Listening for CAN messages...\n");
    read_can_message(spi_handle);

    gpioTerminate();
    return 0;
}
