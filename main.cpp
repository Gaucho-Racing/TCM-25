#include <JetsonGPIO.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <cstring>
#include <Nodes.h>

// Define GPIO pin numbers for CS and INT
const int CS_PIN = <what the fuck is the pin number>;
const int INT_PIN = <which pins are we connecting to?>;
#define SPI_BUS 0     
#define SPI_DEVICE 0   
#define SPI_SPEED 20000000 // should be 20 Mhz

int spi_fd;

// map of CAN IDs to file names
std::unordered_map<std::string, std::string> sensorFiles = {
    {"0x100102", "ecu.log"},
    {"0x100103", "acu.log"},
    {"0x100105", "dash_panel.log"},
    {"0x100106", "steering_wheel.log"},
    {"0x100107", "inverter1.log"},
    {"0x100108", "inverter2_panel.log"},
    {"0x10010C", "sam1_panel.log"}
    {"0x10010D", "sam2_panel.log"}

    //can id and file name
};

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
    const int frame_size = 64; // Adjust frame size as needed
    uint8_t tx[frame_size] = {0}; // Transmission buffer
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

std::string sortSensorData(const std::string &data) {
    // this would return the first 4 characters of the can message as the can id
    std::string canID = data.substr(0, 9); // idfk what can id length is

    if (sensorFiles.find(canID) != sensorFiles.end()) {
        return sensorFiles[canID];
    } else {
        // trash dump idfk
        return "misc_sensor_data.log";
    }
}

void logData(const std::string &data) {
    std::ofstream logfile;
    //Implement CAN ID sorting here and save each metric to its own file
    logfile.open(sortSensorData(data), std::ios::app);

    if (decodeFunctions.find(canID) != decodeFunctions.end()) {
        // data decode
        std::string decodedData = decodeFunctions[canID](data);
        logfile << decodedData << std::endl;
    } else {
        // log raw data if error
        logfile << "Raw: " << data << std::endl;
    }

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
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
    }

    GPIO::cleanup();
    return 0;
}
