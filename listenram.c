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
#define C1CON 0x0002          // CAN Control Register
#define C1FIFOCON1 0x005C     // FIFO Configuration Register
#define C1FIFOSTA1 0x0060     // FIFO Status Register
#define C1FIFOUA1 0x0064      // FIFO User Address Register
#define RAM_BASE 0x400        // RAM Base Address
#define RX_FIFO_SIZE 8        // Default RX FIFO Size

// Function Prototypes
void write_register(int spi_handle, uint16_t reg, uint8_t value);
uint8_t read_register_byte(int spi_handle, uint16_t reg);
void configure_can_controller(int spi_handle);
void read_message_from_ram(int spi_handle);

// Write a single byte to a register
void write_register(int spi_handle, uint16_t reg, uint8_t value) {
    uint8_t tx[3] = { WRITE_CMD | ((reg >> 8) & 0x0F), reg & 0xFF, value };
    spiXfer(spi_handle, tx, NULL, sizeof(tx));
}

// Read a single byte from a register
uint8_t read_register_byte(int spi_handle, uint16_t reg) {
    uint8_t tx[3] = { READ_CMD | ((reg >> 8) & 0x0F), reg & 0xFF, 0x00 };
    uint8_t rx[3] = { 0 };

    spiXfer(spi_handle, tx, rx, sizeof(rx));
    return rx[2];
}

// Configure MCP2518FD as a CAN controller
void configure_can_controller(int spi_handle) {
    // Set Configuration Mode
    write_register(spi_handle, C1CON, 0x04); // Configuration Mode
    usleep(100000); // Allow time to settle

    // Configure RX FIFO
    write_register(spi_handle, C1FIFOCON1, 0x07); // Enable FIFO, set RX size

    // Enable Normal Mode
    write_register(spi_handle, C1CON, 0x00); // Normal mode
}

// Read a CAN message stored in RAM
void read_message_from_ram(int spi_handle) {
    uint8_t header[4], payload[RX_FIFO_SIZE];
    uint8_t dlc;
    uint32_t id;

    // Read FIFO User Address to get the message location
    uint16_t ram_address = (read_register_byte(spi_handle, C1FIFOUA1) << 8) |
                           read_register_byte(spi_handle, C1FIFOUA1 + 1);

    printf("Message located at RAM address: 0x%04X\n", ram_address);

    // Read the header
    for (int i = 0; i < 4; i++) {
        header[i] = read_register_byte(spi_handle, RAM_BASE + ram_address + i);
    }

    // Extract CAN ID and DLC
    id = (header[0] << 24) | (header[1] << 16) | (header[2] << 8) | header[3];
    dlc = header[3] & 0x0F;

    printf("Message ID: 0x%08X, DLC: %u\n", id, dlc);

    // Read the payload
    for (int i = 0; i < dlc; i++) {
        payload[i] = read_register_byte(spi_handle, RAM_BASE + ram_address + 4 + i);
    }

    printf("Payload:");
    for (int i = 0; i < dlc; i++) {
        printf(" %02X", payload[i]);
    }
    printf("\n");
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

    printf("Configuring CAN controller...\n");
    configure_can_controller(spi_handle);

    printf("Reading a message from RAM...\n");
    read_message_from_ram(spi_handle);

    gpioTerminate();
    return 0;
}