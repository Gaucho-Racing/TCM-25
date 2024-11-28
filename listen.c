#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <jetgpio.h>

// MCP2518FD SPI Constants
#define MCP2518FD_SPI_PORT 1
#define SPI_SPEED 10000000 // 10 MHz
#define SPI_MODE 0

// MCP2518FD Command Definitions
#define READ_CMD 0x30
#define WRITE_CMD 0x20

// MCP2518FD Register Addresses
#define C1CON 0x002          // CAN Control Register
#define C1NBTCFG 0x004       // Nominal Bit Timing Configuration
#define C1FIFOCON1 0x05C     // FIFO Configuration
#define C1FIFOSTA1 0x060     // FIFO Status Register
#define C1FIFOUA1 0x064      // FIFO User Address Register
#define RX_FIFO 0x030        // RX FIFO Base Address
#define C1INT 0x01C          // Interrupt Register
#define FILTER_MASK 0x0F00   // Mask Register Base
#define FILTER_ID 0x0F04     // Filter Register Base

// Function Prototypes
void write_register(int spi_handle, uint16_t reg, uint8_t value);
uint8_t read_register_byte(int spi_handle, uint16_t reg);
void configure_can_listener(int spi_handle);
void read_can_messages(int spi_handle);

// Write a single byte to a register
void write_register(int spi_handle, uint16_t reg, uint8_t value) {
    uint8_t tx[3] = { WRITE_CMD | ((reg >> 8) & 0x0F), reg & 0xFF, value };
    spiWrite(spi_handle, tx, sizeof(tx));
}

// Read a single byte from a register
uint8_t read_register_byte(int spi_handle, uint16_t reg) {
    uint8_t tx[3] = { READ_CMD | ((reg >> 8) & 0x0F), reg & 0xFF, 0x00 };
    uint8_t rx[3] = { 0 };

    spiXfer(spi_handle, tx, rx, sizeof(rx));
    return rx[2];
}

// Configure MCP2518FD for listening mode
void configure_can_listener(int spi_handle) {
    // Reset MCP2518FD
    write_register(spi_handle, 0x0F, 0x00); // Reset Command
    usleep(100000); // Wait for reset

    // Set Configuration Mode
    write_register(spi_handle, C1CON, 0x04); // Set REQOP to Configuration Mode

    // Set Baud Rate (1 Mbps)
    write_register(spi_handle, C1NBTCFG + 0, 0x00); // BRP = 0
    write_register(spi_handle, C1NBTCFG + 1, 0x1C); // TSEG1 = 28
    write_register(spi_handle, C1NBTCFG + 2, 0x03); // TSEG2 = 3
    write_register(spi_handle, C1NBTCFG + 3, 0x01); // SJW = 1

    // Configure FIFO for RX
    write_register(spi_handle, C1FIFOCON1, 0x07); // FIFO size: 8 messages

    // Optional: Set up a filter to accept only CAN ID 0x123
    write_register(spi_handle, FILTER_MASK, 0xFF << 21);  // Exact match on 11-bit ID
    write_register(spi_handle, FILTER_ID, 0x123 << 21);   // Accept only ID 0x123

    // Set Normal Mode (Listen-only mode is optional)
    write_register(spi_handle, C1CON, 0x00); // Set REQOP to Normal Mode
}

// Read and print CAN messages from RX FIFO
void read_can_messages(int spi_handle) {
    uint8_t header[4];
    uint8_t payload[8]; // 8-byte payload (maximum for standard CAN)
    uint8_t dlc;
    uint32_t id;

    while (1) {
        // Check RX FIFO Status for new messages
        uint8_t fifo_status = read_register_byte(spi_handle, C1FIFOSTA1);
        if (fifo_status & 0x01) { // RX Full flag
            // Read the 4-byte header
            for (int i = 0; i < 4; i++) {
                header[i] = read_register_byte(spi_handle, RX_FIFO + i);
            }

            // Extract CAN ID and DLC
            id = (header[0] << 24) | (header[1] << 16) | (header[2] << 8) | header[3];
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

            // Acknowledge the FIFO entry
            write_register(spi_handle, C1FIFOUA1, 0x00);
        }

        usleep(1000); // Polling delay
    }
}

int main() {
    int Init = gpioInitialise();
    if (Init < 0) {
        printf("GPIO initialization failed. Error code: %d\n", Init);
        return Init;
    }

    int spi_handle = spiOpen(MCP2518FD_SPI_PORT, SPI_SPEED, SPI_MODE, 0, 8, 0, 1);
    if (spi_handle < 0) {
        printf("SPI initialization failed. Error code: %d\n", spi_handle);
        gpioTerminate();
        return spi_handle;
    }

    printf("Initializing CAN listener...\n");
    configure_can_listener(spi_handle);

    printf("Listening for CAN messages...\n");
    read_can_messages(spi_handle);

    gpioTerminate();
    return 0;
}
