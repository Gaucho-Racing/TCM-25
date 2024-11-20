#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <jetgpio.h>

#include <unistd.h>

#define SPI_BUS 1
#define CS_PIN 24
#define SPI_SPEED 5000000 // 5 MHz
#define RX_FIFO_ADDRESS 0x400 // Example address for FIFO
#define RX_FIFO_CONTROL_ADDRESS 0x480 // CiFIFOCON1 base address
#define BUFFER_SIZE 64

int initialize_mcp2518fd(int spi_handle);
void configure_baud_rate(int spi_handle);
int switch_to_normal_mode(int spi_handle);
void read_can_message(int spi_handle);
void setup_catch_all_filter(int spi_handle);
void setup_rx_fifo(int spi_handle);
void verify_rx_fifo_config(int spi_handle);
void verify_filter_config(int spi_handle);
void debug_raw_fifo_data(uint8_t *rx_buffer, int length);

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

    // Set up RX FIFO at 0x400
    setup_rx_fifo(SPI_init);

    // Verify RX FIFO configuration
    verify_rx_fifo_config(SPI_init);

    // Set up a catch-all filter
    setup_catch_all_filter(SPI_init);

    // Verify filter configuration
    verify_filter_config(SPI_init);

    // Switch to Normal mode
    if (switch_to_normal_mode(SPI_init) != 0) {
        printf("Failed to switch MCP2518FD to Normal mode.\n");
        spiClose(SPI_init);
        gpioTerminate();
        exit(-1);
    }

    // Continuously read CAN messages
    while (1) {
        read_can_message(SPI_init);
        usleep(1000); // Optional delay to prevent excessive polling
    }

    // Cleanup
    spiClose(SPI_init);
    gpioTerminate();

    return 0;
}

int initialize_mcp2518fd(int spi_handle) {
    uint8_t tx_buffer[BUFFER_SIZE] = {0};
    uint8_t rx_buffer[BUFFER_SIZE] = {0};

    // Set MCP2518FD to Configuration mode (REQOP = 0b100)
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

    return 0;
}

void configure_baud_rate(int spi_handle) {
    uint8_t tx_buffer[BUFFER_SIZE] = {0};
    uint8_t rx_buffer[BUFFER_SIZE] = {0};

    // Set CiNBTCFG register for 1 Mbps
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

int switch_to_normal_mode(int spi_handle) {
    uint8_t tx_buffer[4] = {0x02, 0x00, 0x00, 0x00}; // Write to CiCON to set REQOP=000
    uint8_t rx_buffer[4] = {0};

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 4) < 0) {
        printf("Failed to switch to Normal mode.\n");
        return -1;
    }

    printf("MCP2518FD switched to Normal mode.\n");
    return 0;
}

void setup_rx_fifo(int spi_handle) {
    uint8_t tx_buffer[8] = {0};
    uint8_t rx_buffer[8] = {0};

    // Configure FIFO Control Register (CiFIFOCON1)
    tx_buffer[0] = 0x02; // SPI Write Command
    tx_buffer[1] = (RX_FIFO_CONTROL_ADDRESS >> 8) & 0xFF; // High byte of FIFO Control address
    tx_buffer[2] = RX_FIFO_CONTROL_ADDRESS & 0xFF;        // Low byte of FIFO Control address

    tx_buffer[3] = 0x00; // Configure as RX FIFO (TXEN = 0)
    tx_buffer[4] = 0x00; // Reserved
    tx_buffer[5] = 0x0F; // FSIZE = 16 messages (n - 1 for FSIZE)
    tx_buffer[6] = 0x08; // PLSIZE = 8 bytes per payload
    tx_buffer[7] = 0x00; // Reserved

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 8) < 0) {
        printf("Failed to configure RX FIFO.\n");
        return;
    }

    printf("RX FIFO configured at address 0x400.\n");
}

void setup_catch_all_filter(int spi_handle) {
    uint8_t tx_buffer[8] = {0};
    uint8_t rx_buffer[8] = {0};

    // Configure Filter Object (CiFLTOBJ0)
    tx_buffer[0] = 0x02;
    tx_buffer[1] = 0x1C;
    tx_buffer[2] = 0x00;
    tx_buffer[3] = 0x00; // Match all CAN IDs
    tx_buffer[4] = 0x00;
    tx_buffer[5] = 0x00;
    tx_buffer[6] = 0x00;
    tx_buffer[7] = 0x00;

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 8) < 0) {
        printf("Failed to configure filter object.\n");
        return;
    }

    // Configure Filter Mask (CiMASK0)
    tx_buffer[1] = 0x1C;
    tx_buffer[2] = 0x04;
    tx_buffer[3] = 0x00; // Ignore all bits
    tx_buffer[4] = 0x00;
    tx_buffer[5] = 0x00;
    tx_buffer[6] = 0x00;

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 8) < 0) {
        printf("Failed to configure filter mask.\n");
        return;
    }

    // Enable Filter and Assign to RX FIFO (CiFLTCON0)
    tx_buffer[1] = 0x1C;
    tx_buffer[2] = 0x08;
    tx_buffer[3] = (1 << 5) | 0x01;

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 5) < 0) {
        printf("Failed to enable filter.\n");
        return;
    }

    printf("Catch-all filter configured successfully.\n");
}

void read_can_message(int spi_handle) {
    uint8_t tx_buffer[16] = {0};
    uint8_t rx_buffer[16] = {0};

    // Get FIFO user address (CiFIFOUA)
    tx_buffer[0] = 0x03;
    tx_buffer[1] = (RX_FIFO_ADDRESS >> 8) & 0xFF;
    tx_buffer[2] = RX_FIFO_ADDRESS & 0xFF;

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 16) < 0) {
        printf("Failed to read RX FIFO.\n");
        return;
    }

    // Debug raw FIFO data
    debug_raw_fifo_data(rx_buffer, 16);

    // Parse message data
    uint32_t can_id = (rx_buffer[1] << 24) | (rx_buffer[2] << 16) | (rx_buffer[3] << 8) | rx_buffer[4];
    uint8_t dlc = rx_buffer[5];
    uint8_t data[8];
    memcpy(data, &rx_buffer[6], dlc);

    // Print CAN message
    printf("Received CAN Message:\n");
    printf("CAN ID: 0x%08X\n", can_id);
    printf("DLC: %d\n", dlc);
    printf("Data: ");
    for (int i = 0; i < dlc; i++) {
        printf("0x%02X ", data[i]);
    }
    printf("\n");

    // Acknowledge the read (update FIFO pointer)
    tx_buffer[0] = 0x02;
    tx_buffer[1] = (RX_FIFO_CONTROL_ADDRESS >> 8) & 0xFF;
    tx_buffer[2] = RX_FIFO_CONTROL_ADDRESS & 0xFF;
    tx_buffer[3] = 0x20;

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 4) < 0) {
        printf("Failed to update FIFO pointer.\n");
        return;
    }
}

void verify_rx_fifo_config(int spi_handle) {
    uint8_t tx_buffer[4] = {0x03, 0x04, 0x80, 0x00};
    uint8_t rx_buffer[8] = {0};

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 8) < 0) {
        printf("Failed to read RX FIFO configuration.\n");
        return;
    }

    printf("RX FIFO Configuration:\n");
    for (int i = 0; i < 8; i++) {
        printf("0x%02X ", rx_buffer[i]);
    }
    printf("\n");
}

void verify_filter_config(int spi_handle) {
    uint8_t tx_buffer[4] = {0x03, 0x1C, 0x00, 0x00};
    uint8_t rx_buffer[8] = {0};

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 8) < 0) {
        printf("Failed to read filter object.\n");
        return;
    }

    printf("Filter Object Configuration:\n");
    for (int i = 0; i < 8; i++) {
        printf("0x%02X ", rx_buffer[i]);
    }
    printf("\n");

    tx_buffer[2] = 0x04;

    if (spiXfer(spi_handle, (char *)tx_buffer, (char *)rx_buffer, 8) < 0) {
        printf("Failed to read filter mask.\n");
        return;
    }

    printf("Filter Mask Configuration:\n");
    for (int i = 0; i < 8; i++) {
        printf("0x%02X ", rx_buffer[i]);
    }
    printf("\n");
}

void debug_raw_fifo_data(uint8_t *rx_buffer, int length) {
    printf("Raw RX FIFO Data:\n");
    for (int i = 0; i < length; i++) {
        printf("0x%02X ", rx_buffer[i]);
    }
    printf("\n");
}
