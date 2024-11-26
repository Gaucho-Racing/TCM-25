#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <jetgpio.h>

#define SPI_PORT 1
#define SPI_SPEED 5000000
#define SPI_MODE 0
#define SPI_BITS 8
#define GPIO_INT_PIN 1

volatile int interrupt_flag = 0;

// Interrupt handler
void gpio_interrupt_handler(int gpio, int level, uint32_t tick, void *user) {
    interrupt_flag = 1;
    printf("Interrupt triggered on GPIO %d. Level: %d\n", gpio, level);
}

int main(int argc, char *argv[]) {
    int Init, SPI_init;
    char tx[2] = {0x20, 0x02};
    char rx[2] = {0};

    // Initialize JETGPIO
    Init = gpioInitialise();
    if (Init < 0) {
        printf("Jetgpio initialisation failed. Error code: %d\n", Init);
        return Init;
    }
    printf("Jetgpio initialisation OK.\n");

    // Configure GPIO01 as input and enable interrupts
    if (gpioSetMode(GPIO_INT_PIN, JET_INPUT) < 0) {
        printf("Failed to set GPIO %d as input.\n", GPIO_INT_PIN);
        gpioTerminate();
        return -1;
    }
    if (gpioSetISRFunc(GPIO_INT_PIN, JET_EDGE_FALLING, 0, gpio_interrupt_handler, NULL) < 0) {
        printf("Failed to set interrupt for GPIO %d.\n", GPIO_INT_PIN);
        gpioTerminate();
        return -1;
    }

    // Open SPI1
    SPI_init = spiOpen(SPI_PORT, SPI_SPEED, SPI_MODE, 0, SPI_BITS, 0, 0);
    if (SPI_init < 0) {
        printf("Failed to open SPI1. Error code: %d\n", SPI_init);
        gpioTerminate();
        return SPI_init;
    }
    printf("SPI1 opened successfully.\n");

    // Main loop to send/receive data
    for (int i = 0; i < 10; i++) {
        if (interrupt_flag) {
            interrupt_flag = 0;

            // Perform SPI transfer
            if (spiXfer(SPI_init, tx, rx, 2) < 0) {
                printf("SPI transfer failed.\n");
            } else {
                printf("SPI transfer completed. TX: 0x%02X 0x%02X | RX: 0x%02X 0x%02X\n",
                       tx[0], tx[1], rx[0], rx[1]);
            }
        }

        usleep(100000); // Small delay
    }

    // Clean up
    spiClose(SPI_init);
    gpioTerminate();

    return 0;
}
