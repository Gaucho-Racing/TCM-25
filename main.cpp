#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <JetsonGPIO.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CAN_INTERFACE "can0" 

#define PIN_CS 8  // Pin for the can controller

void setupGPIO() {
    if (JetsonGPIO::gpioInitialise() < 0) {
        std::cerr << "Unable to initialize GPIO" << std::endl;
        exit(1);
    }

    JetsonGPIO::gpioSetMode(PIN_CS, JetsonGPIO::OUTPUT);
    JetsonGPIO::gpioWrite(PIN_CS, 0);  
}

void initializeCAN() {
    int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) {
        std::cerr << "Error creating CAN socket." << std::endl;
        exit(1);
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, CAN_INTERFACE, sizeof(ifr.ifr_name) - 1);

    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        std::cerr << "Error getting CAN interface index." << std::endl;
        close(sock);
        exit(1);
    }

    struct sockaddr_can addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error binding to CAN interface." << std::endl;
        close(sock);
        exit(1);
    }

    std::cout << "CAN interface initialized successfully!" << std::endl;
}

void saveCANDataToFile(const can_frame& frame) {
    std::ofstream outfile;
    outfile.open("/home/jetson/can_data.txt", std::ios_base::app);  
    if (outfile.is_open()) {
        outfile << "ID: " << frame.can_id
                << " DLC: " << (int)frame.can_dlc
                << " Data: ";
        for (int i = 0; i < frame.can_dlc; ++i) {
            outfile << std::hex << (int)frame.data[i] << " ";
        }
        outfile << std::endl;
        outfile.close();
    } else {
        std::cerr << "Error opening file for writing." << std::endl;
    }
}

void receiveCANData(int sock) {
    struct can_frame frame;
    while (true) {
        int nbytes = read(sock, &frame, sizeof(struct can_frame));
        if (nbytes < 0) {
            std::cerr << "Error reading from CAN bus." << std::endl;
            break;
        } else if (nbytes < sizeof(struct can_frame)) {
            std::cerr << "Incomplete CAN frame received." << std::endl;
        } else {
            std::cout << "Received CAN frame:" << std::endl;
            std::cout << "ID: " << frame.can_id
                      << " DLC: " << (int)frame.can_dlc
                      << " Data: ";
            for (int i = 0; i < frame.can_dlc; ++i) {
                std::cout << std::hex << (int)frame.data[i] << " ";
            }
            std::cout << std::endl;

            saveCANDataToFile(frame);
        }
    }
}

int main() {
    setupGPIO();

    initializeCAN();

    int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    receiveCANData(sock);

    close(sock);
    JetsonGPIO::gpioTerminate();

    return 0;
}
