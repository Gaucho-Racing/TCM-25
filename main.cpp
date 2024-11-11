#include <JetsonGPIO.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <cstring>

// Define GPIO pin numbers for CS and INT
const int CS_PIN = <CS_PIN_NUMBER>;
const int INT_PIN = <INT_PIN_NUMBER>;
#define SPI_BUS 0       // Adjust to your SPI bus (usually 0 on Jetson Nano)
#define SPI_DEVICE 0    // Adjust to your device (usually 0 for spidev0.0)
#define SPI_SPEED 1000000 // 1 MHz, adjust based on CAN FD controller specs

int spi_fd;

void initGPIO() {
    GPIO::setmode(GPIO::BCM);
    GPIO::setup(CS_PIN, GPIO::OUT);
    GPIO::setup(INT_PIN, GPIO::IN);
}

bool initSPI() {
    std::string spi_device = "/dev/spidev" + std::to_string(SPI_BUS) + "." + std::to_string(SPI_DEVICE);
    spi_fd = open(spi_device.c_str(), O_RDWR);
    if (spi_fd < 0) {
        std::cerr << "Failed to open SPI device: " << strerror(errno) << std::endl;
        return false;
    }

    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = SPI_SPEED;

    // Set SPI mode
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        std::cerr << "Failed to set SPI mode: " << strerror(errno) << std::endl;
        return false;
    }

    // Set bits per word
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        std::cerr << "Failed to set SPI bits per word: " << strerror(errno) << std::endl;
        return false;
    }

    // Set max speed (in Hz)
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        std::cerr << "Failed to set SPI speed: " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

bool readCANFDData(std::string &data) {
    const int frame_size = 16; // Adjust frame size based on the CAN FD controller's data frame size
    uint8_t tx[frame_size] = {0}; // Transmission buffer (might be filled with read command bytes)
    uint8_t rx[frame_size] = {0}; // Reception buffer

    // SPI transaction structure
    struct spi_ioc_transfer spi = {};
    spi.tx_buf = reinterpret_cast<unsigned long>(tx);
    spi.rx_buf = reinterpret_cast<unsigned long>(rx);
    spi.len = frame_size;
    spi.speed_hz = SPI_SPEED;
    spi.bits_per_word = 8;
    spi.cs_change = 0;

    // Perform SPI transaction
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi) < 0) {
        std::cerr << "Failed to communicate over SPI: " << strerror(errno) << std::endl;
        return false;
    }

    // Convert received bytes to a hex string
    data.clear();
    for (int i = 0; i < frame_size; ++i) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02X", rx[i]);
        data += hex;
    }

    return true;
}

void closeSPI() {
    if (spi_fd >= 0) {
        close(spi_fd);
    }
}

void sendData(const std::string &data) {
    return 0;
}

void logData(const std::string &data) {
    std::ofstream logfile;
    logfile.open("/path/to/save/folder/canfd_data.txt", std::ios::app);
    logfile << data << std::endl;
    logfile.close();
}

int main() {
    initGPIO();

    while (true) {
        std::string data;
        if (GPIO::input(INT_PIN) == GPIO::HIGH) { // Check interrupt signal
            if (readCANFDData(data)) {
                logData(data);
                sendData(data);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Adjust based on system latency
    }

    GPIO::cleanup();
    return 0;
}
