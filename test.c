#include <stdint.h>
#include <stdio.h>
#include <jetgpio.h>

int main() {
    int spi_handle;

    // Initialize JetGPIO and open SPI port
    gpioInitialise();
    spi_handle = spiOpen(1, 5000000, 0, 0, 8, 0, 1);

    // Define read command for C1CON
    uint8_t read_command[3] = {0x03, 0x00, 0x03};
    uint8_t response[4]; // Buffer for the 4-byte response

    // Perform SPI transfer (send command and receive response)
    spiXfer(spi_handle, (char *)read_command, (char *)response, 7); // Send 3 bytes, expect 4 back

    // Print the response
    printf("Response: 0x%02X 0x%02X 0x%02X 0x%02X\n",
           response[0], response[1], response[2], response[3]);

    // Clean up
    spiClose(spi_handle);
    gpioTerminate();

    return 0;
}
