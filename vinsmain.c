#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <jetgpio.h>

#define SPI_BUS 1
#define CS_PIN 24
#define SPI_SPEED 5000000 // 5 MHz
#define RX_FIFO_ADDRESS 0x500
#define RX_FIFO_CONTROL_ADDRESS 0x580
#define BUFFER_SIZE 64

// MCP2518FD SPI instructions
#define MCP2518FD_WRITE 0x02
#define MCP2518FD_READ 0x03

// MCP2518FD Register Addresses
#define REG_CiCON 0x000
#define REG_CiNBTCFG 0x004
#define REG_CiFIFOCON 0x050
#define REG_CiFIFOUA 0x058
#define REG_CiRXIF 0x020

// Function prototypes
int initialize_mcp2518fd(int spi_handle);
int switch_to_normal_mode(int spi_handle);
void configure_baud_rate(int spi_handle);
void setup_rx_fifo(int spi_handle);
void setup_catch_all_filter(int spi_handle);
void read_can_message(int spi_handle);
void debug_fifo_data(uint8_t *rx_buffer, int length);
void acknowledge_fifo_read(int spi_handle);
void verify_fifo_setup(int spi_handle);
void verify_filter_mask_setup(int spi_handle);

int main() {
    int Init, SPI_init;

    // Initialize JetGPIO
    Init = gpioInitialise();
    if (Init < 0) {
        printf("JetGPIO initialization failed. Error code: %d\n", Init);
        return -1;
    }
    printf("JetGPIO initialized successfully.\n");

    // Open SPI interface
    SPI_init = spiOpen(SPI_BUS, SPI_SPEED, 0, 0, 8, 1, 1);
    if (SPI_init < 0) {
        printf("Failed to open SPI bus. Error code: %d\n", SPI_init);
        gpioTerminate();
        return -1;
    }
    printf("SPI bus opened successfully.\n");

    // Initialize MCP2518FD
    if (initialize_mcp2518fd(SPI_init) != 0) {
        printf("Failed to initialize MCP2518FD.\n");
        spiClose(SPI_init);
        gpioTerminate();
        return -1;
    }

    // Set up RX FIFO
    setup_rx_fifo(SPI_init);

    // Verify RX FIFO setup
    verify_fifo_setup(SPI_init);

    // Set up a catch-all filter
    setup_catch_all_filter(SPI_init);

    // Verify filter and mask setup
    verify_filter_mask_setup(SPI_init);

    // Switch to Normal mode
    if (switch_to_normal_mode(SPI_init) != 0) {
        printf("Failed to switch MCP2518FD to Normal mode.\n");
        spiClose(SPI_init);
        gpioTerminate();
        return -1;
    }

    // Continuously read CAN messages
    while (1) {
        read_can_message(SPI_init);
        usleep(1000); // Optional delay
    }

    // Cleanup
    spiClose(SPI_init);
    gpioTerminate();

    return 0;
}

int initialize_mcp2518fd(int spi_handle) {
    uint8_t tx_buffer[4] = {MCP2518FD_WRITE, (REG_CiCON >> 8) & 0xFF, REG_CiCON & 0xFF, 0x40}; // Set Configuration mode
    uint8_t rx_buffer[4] = {0};

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 4) < 0) {
        printf("Failed to set Configuration mode.\n");
        return -1;
    }

    printf("MCP2518FD set to Configuration mode.\n");

    configure_baud_rate(spi_handle);
    return 0;
}

int switch_to_normal_mode(int spi_handle) {
    uint8_t tx_buffer[4] = {MCP2518FD_WRITE, (REG_CiCON >> 8) & 0xFF, REG_CiCON & 0xFF, 0x00}; // Set Normal mode
    uint8_t rx_buffer[4] = {0};

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 4) < 0) {
        printf("Failed to set Normal mode.\n");
        return -1;
    }

    printf("MCP2518FD switched to Normal mode.\n");
    return 0;
}

void configure_baud_rate(int spi_handle) {
    uint8_t tx_buffer[5] = {MCP2518FD_WRITE, (REG_CiNBTCFG >> 8) & 0xFF, REG_CiNBTCFG & 0xFF, 0x87, 0x01}; // Set 1 Mbps baud rate
    uint8_t rx_buffer[5] = {0};

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 5) < 0) {
        printf("Failed to configure baud rate.\n");
        exit(-1);
    }

    printf("Baud rate configured to 1 Mbps.\n");
}

void setup_rx_fifo(int spi_handle) {
    uint8_t tx_buffer[8] = {MCP2518FD_WRITE, (RX_FIFO_CONTROL_ADDRESS >> 8) & 0xFF, RX_FIFO_CONTROL_ADDRESS & 0xFF, 0x00, 0x00, 0x03, 0x00, 0x00}; // RX FIFO setup
    uint8_t rx_buffer[8] = {0};

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 8) < 0) {
        printf("Failed to configure RX FIFO.\n");
        return;
    }

    printf("RX FIFO configured.\n");
}

void setup_catch_all_filter(int spi_handle) {
    uint8_t tx_buffer[8] = {MCP2518FD_WRITE, (REG_CiFIFOCON >> 8) & 0xFF, REG_CiFIFOCON & 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00}; // Filter setup
    uint8_t rx_buffer[8] = {0};

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 8) < 0) {
        printf("Failed to configure catch-all filter.\n");
        return;
    }

    printf("Catch-all filter configured.\n");
}

void read_can_message(int spi_handle) {
    uint8_t tx_buffer[16] = {MCP2518FD_READ, (RX_FIFO_ADDRESS >> 8) & 0xFF, RX_FIFO_ADDRESS & 0xFF};
    uint8_t rx_buffer[16] = {0};

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 16) < 0) {
        printf("Failed to read RX FIFO.\n");
        return;
    }

    // Debug raw data
    debug_fifo_data(rx_buffer, 16);

    // Extract CAN message
    uint32_t can_id = (rx_buffer[1] << 24) | (rx_buffer[2] << 16) | (rx_buffer[3] << 8) | rx_buffer[4];
    uint8_t dlc = rx_buffer[5];
    printf("Received CAN Message: ID=0x%08X DLC=%d\n", can_id, dlc);

    // Acknowledge the read
    acknowledge_fifo_read(spi_handle);
}

void debug_fifo_data(uint8_t *rx_buffer, int length) {
    printf("Raw FIFO Data:\n");
    for (int i = 0; i < length; i++) {
        printf("0x%02X ", rx_buffer[i]);
    }
    printf("\n");
}

void acknowledge_fifo_read(int spi_handle) {
    uint8_t tx_buffer[4] = {MCP2518FD_WRITE, (RX_FIFO_CONTROL_ADDRESS >> 8) & 0xFF, RX_FIFO_CONTROL_ADDRESS & 0xFF, 0x20}; // UINC bit
    uint8_t rx_buffer[4] = {0};

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 4) < 0) {
        printf("Failed to update FIFO pointer.\n");
    } else {
        printf("FIFO pointer updated successfully.\n");
    }
}
