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
#define C1FIFOSTA1 0x060     // FIFO Status Register Base
#define C1FIFOUA1 0x064      // FIFO User Address Register
#define RX_FIFO_BASE 0x060   // RX FIFO Base Address
#define RX_FIFO_SIZE 5       // RX FIFO Register Count (0x060 to 0x064)
#define C1INT 0x01C          // Interrupt Register

// Function Prototypes
void write_register(int spi_handle, uint16_t reg, uint8_t value);
uint8_t read_register_byte(int spi_handle, uint16_t reg);
void configure_canfd(int spi_handle);
void read_fifo(int spi_handle);

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
    uint8_t tx[3] = { 0 };
    uint8_t rx[3] = { 0 };

    tx[0] = READ_CMD | ((reg >> 8) & 0x0F); // Read command and high 4 bits of address
    tx[1] = reg & 0xFF;                    // Low 8 bits of address

    spiXfer(spi_handle, tx, rx, sizeof(rx));

    return rx[2];
}

// Configure MCP2518FD for CAN FD operation
void configure_canfd(int spi_handle) {
    // Reset MCP2518FD
    write_register(spi_handle, 0x0F, 0x00);
    usleep(100000); // Wait for reset

    // Set Configuration Mode
    write_register(spi_handle, C1CON, 0x04); // Set REQOP to Configuration Mode

    // Set Nominal Bit Timing Configuration (1 Mbps)
    write_register(spi_handle, C1NBTCFG + 0, 0x00); // BRP
    write_register(spi_handle, C1NBTCFG + 1, 0x1C); // TSEG1
    write_register(spi_handle, C1NBTCFG + 2, 0x03); // TSEG2
    write_register(spi_handle, C1NBTCFG + 3, 0x01); // SJW

    // Configure FIFO as RX
    write_register(spi_handle, C1FIFOCON1, 0x07); // FIFO size 8 messages

    // Configure FIFO registers 0x060-0x064 for RX FIFO
    for (int i = 0; i < RX_FIFO_SIZE; i++) {
        write_register(spi_handle, RX_FIFO_BASE + i, 0x00); // Initialize RX FIFO registers
    }

    // Set Normal Mode
    write_register(spi_handle, C1CON, 0x00); // Set REQOP to Normal Mode
}

// Read CAN messages from FIFO
void read_fifo(int spi_handle) {
    uint8_t fifo_data[RX_FIFO_SIZE];

    while (1) {
        // Check for received message
        uint32_t int_status = read_register_byte(spi_handle, C1INT);
        if (int_status & 0x01) { // RX interrupt
            printf("Received CAN message from FIFO:\n");
            for (int i = 0; i < RX_FIFO_SIZE; i++) {
                fifo_data[i] = read_register_byte(spi_handle, RX_FIFO_BASE + i);
                printf("Register 0x%03X: 0x%02X\n", RX_FIFO_BASE + i, fifo_data[i]);
            }
            printf("\n");
        }
        usleep(1000); // Polling delay
    }
}

int main(int argc, char *argv[]) {
    int Init;
    int SPI_init;

    Init = gpioInitialise();
    if (Init < 0) {
        printf("Jetgpio initialization failed. Error code: %d\n", Init);
        return Init;
    }
    printf("Jetgpio initialization OK. Return code: %d\n", Init);

    SPI_init = spiOpen(MCP2518FD_SPI_PORT, SPI_SPEED, SPI_MODE, 0, 8, 0, 1);
    if (SPI_init < 0) {
        printf("SPI initialization failed. Error code: %d\n", SPI_init);
        return SPI_init;
    }
    printf("SPI initialization OK. Return code: %d\n", SPI_init);

    configure_canfd(SPI_init);
    read_fifo(SPI_init);

    gpioTerminate();
    return 0;
}
