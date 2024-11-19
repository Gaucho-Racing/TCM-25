#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <jetgpio.h>

#define SPI_BUS 1
#define CS_PIN 24
#define SPI_SPEED 5000000 // 5 MHz
#define RX_FIFO_ADDRESS 0x400 // Example address for FIFO
#define BUFFER_SIZE 64

int initialize_mcp2518fd(int spi_handle);
void configure_baud_rate(int spi_handle);
void read_can_message(int spi_handle);

int main() {
    int Init;
    int SPI_init;

    // Initialize JetGPIO
    Init = gpioInitialise();
    if (Init < 0) {
        printf("JetGPIO initialization failed. Error code: %d\n", Init);
        exit(Init);
    }
    printf("JetGPIO initialized successfully.\n");

    // Open SPI interface
    SPI_init = spiOpen(SPI_BUS, SPI_SPEED, 0, 0, 8, 1, 1);
    if (SPI_init < 0) {
        printf("Failed to open SPI bus. Error code: %d\n", SPI_init);
        gpioTerminate();
        exit(SPI_init);
    }
    printf("SPI bus opened successfully.\n");

    // Initialize MCP2518FD
    if (initialize_mcp2518fd(SPI_init) != 0) {
        printf("Failed to initialize MCP2518FD.\n");
        spiClose(SPI_init);
        gpioTerminate();
        exit(-1);
    }

    // Read a single CAN message
    read_can_message(SPI_init);

    // Cleanup
    spiClose(SPI_init);
    gpioTerminate();

    return 0;
}

int initialize_mcp2518fd(int spi_handle) {
    uint8_t tx_buffer[BUFFER_SIZE] = {0};
    uint8_t rx_buffer[BUFFER_SIZE] = {0};

    // Example: Set MCP2518FD to Configuration mode (REQOP = 0b100)
    tx_buffer[0] = 0x02; // Write command to CiCON register
    tx_buffer[1] = 0x00; // Address (CiCON register base)
    tx_buffer[2] = 0x40; // Data: Configuration mode (REQOP=100)
    tx_buffer[3] = 0x00;

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 4) < 0) {
        printf("Failed to configure MCP2518FD mode.\n");
        return -1;
    }

    printf("MCP2518FD configured to Configuration mode.\n");

    // Configure baud rate
    configure_baud_rate(spi_handle);

    // Switch to Normal mode (REQOP = 0b000)
    tx_buffer[2] = 0x00; // Normal mode
    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 4) < 0) {
        printf("Failed to switch MCP2518FD to Normal mode.\n");
        return -1;
    }

    printf("MCP2518FD switched to Normal mode.\n");
    return 0;
}

void configure_baud_rate(int spi_handle) {
    uint8_t tx_buffer[BUFFER_SIZE] = {0};
    uint8_t rx_buffer[BUFFER_SIZE] = {0};

    // Set CiNBTCFG register for 1 Mbps
    // Example values:
    // BRP = 0 (Prescaler = 1)
    // TSEG1 = 6 (7 TQ)
    // TSEG2 = 1 (2 TQ)
    // SJW = 1 (2 TQ)
    tx_buffer[0] = 0x02; // Write command
    tx_buffer[1] = 0x04; // Address (CiNBTCFG register base)
    tx_buffer[2] = 0x00; // BRP
    tx_buffer[3] = 0x87; // TSEG1 = 6 (7 TQ), SJW = 1 (2 TQ)
    tx_buffer[4] = 0x01; // TSEG2 = 1 (2 TQ)

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 5) < 0) {
        printf("Failed to configure baud rate.\n");
        exit(-1);
    }
    printf("Baud rate configured to 1 Mbps.\n");
}

void read_can_message(int spi_handle) {
    uint8_t tx_buffer[BUFFER_SIZE] = {0};
    uint8_t rx_buffer[BUFFER_SIZE] = {0};

    // Read RX FIFO (example: RX FIFO 1)
    tx_buffer[0] = 0x30; // Read command (FIFO Control)
    tx_buffer[1] = (RX_FIFO_ADDRESS >> 8) & 0xFF; // High byte of FIFO address
    tx_buffer[2] = RX_FIFO_ADDRESS & 0xFF;        // Low byte of FIFO address

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 16) < 0) {
        printf("Failed to read RX FIFO.\n");
        return;
    }

    printf("Received CAN message:\n");
    for (int i = 0; i < 16; i++) {
        printf("0x%02X ", rx_buffer[i]);
    }
    printf("\n");
}
