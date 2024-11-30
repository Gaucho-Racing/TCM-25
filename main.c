#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "/drv/drv_canfdspi_api.h"
#include "jetgpio.h"

#define SPI_PORT 1
#define SPI_SPEED 10000000
#define SPI_MODE 0

CANFDSPI_MODULE_ID canModule = 0; // Assuming single module ID 0

void initialize_can_module() {
    // Reset the CAN FD Controller
    if (DRV_CANFDSPI_Reset(canModule) != 0) {
        printf("CAN FD Controller Reset Failed\n");
        exit(EXIT_FAILURE);
    }

    // Configure the module to Normal mode
    if (DRV_CANFDSPI_OperationModeSelect(canModule, CAN_NORMAL_MODE) != 0) {
        printf("Failed to set CAN Controller to Normal mode\n");
        exit(EXIT_FAILURE);
    }

    // Configure a Receive FIFO
    CAN_RX_FIFO_CONFIG rxConfig = {
        .PayloadSize = CAN_DLC_64,
        .FifoSize = 8,
        .FifoAttempts = CAN_FIFO_ATTEMPTS_THREE
    };
    if (DRV_CANFDSPI_ReceiveChannelConfigure(canModule, CAN_FIFO_CH1, &rxConfig) != 0) {
        printf("Receive FIFO Configuration Failed\n");
        exit(EXIT_FAILURE);
    }

    printf("CAN FD Controller Initialized\n");
}

void read_can_messages() {
    CAN_RX_MSG rxMsg;
    uint8_t rxData[64];
    uint8_t dlc;

    while (1) {
        // Check for new messages in RX FIFO
        if (DRV_CANFDSPI_ReceiveMessageGet(canModule, CAN_FIFO_CH1, &rxMsg, rxData, &dlc) == 0) {
            printf("Message Received:\n");
            printf("  ID: 0x%X\n", rxMsg.ID);
            printf("  DLC: %d\n", dlc);
            printf("  Data: ");
            for (uint8_t i = 0; i < dlc; i++) {
                printf("%02X ", rxData[i]);
            }
            printf("\n");
        } else {
            usleep(1000); // No message, small delay
        }
    }
}

int main() {
    // Initialize GPIO and SPI
    if (gpioInitialise() < 0) {
        printf("GPIO Initialization Failed\n");
        return EXIT_FAILURE;
    }

    // Open SPI channel
    if (spiOpen(SPI_PORT, SPI_SPEED, SPI_MODE, 0, 8, 0, 1) < 0) {
        printf("SPI Initialization Failed\n");
        gpioTerminate();
        return EXIT_FAILURE;
    }

    // Initialize CAN FD Controller
    initialize_can_module();

    // Listen for CAN messages
    printf("Listening for CAN messages...\n");
    read_can_messages();

    gpioTerminate();
    return EXIT_SUCCESS;
}
