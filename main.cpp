#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <mqtt/async_client.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

// This code is straight up asking ChatGPT for an implementation. Guaranteed not to work but baseline ig idfk
// For anyone working on this: 
// JetsonGPIO can help us manipulate pins for LEDS like status lights, etc. 
// Refer to "TCM" and then CAN-R for last years controller code. We use CAN FD which has a variable data rate but last years code
// had a fixed rate and only sent data when all sensors' data were added to the bus
// Because of the variable rate, we have to make sure the bus isn't overloaded/oversaturated

// CAN and MQTT parameters
const char *CAN_INTERFACE = "can0";
const char *MQTT_BROKER = "tcp://<mapache shit idfk bharat clutch up rq>:1883";
const char *MQTT_CLIENT_ID = "Jetson";
const char *TOPIC = "sensor data";

// GPIO pin definitions
const int GPIO_RX = 12;  // Pin for data received indication
const int GPIO_TX = 16;  // Pin for data transmitted indication
const int GPIO_ERR = 18; // Pin for error indication

// Helper function to set GPIO pin state (on/off)
void set_gpio(int pin, bool state) {
    std::ostringstream cmd;
    cmd << "echo " << state << " > /sys/class/gpio/gpio" << pin << "/value";
    system(cmd.str().c_str());
}

// Helper function to initialize GPIO pin as output
void init_gpio(int pin) {
    std::ostringstream cmd;
    cmd << "echo " << pin << " > /sys/class/gpio/export";
    system(cmd.str().c_str());

    cmd.str("");
    cmd << "echo out > /sys/class/gpio/gpio" << pin << "/direction";
    system(cmd.str().c_str());
}

// Helper function to open the CAN socket
int open_can_socket(const char *interface) {
    int socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket_fd < 0) {
        perror("Error while opening socket");
        set_gpio(GPIO_ERR, true);
        return -1;
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name, interface);
    if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {
        perror("Error in ioctl");
        set_gpio(GPIO_ERR, true);
        return -1;
    }

    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Error in socket bind");
        set_gpio(GPIO_ERR, true);
        return -1;
    }

    std::cout << "CAN socket opened successfully on interface " << interface << std::endl;
    return socket_fd;
}

// Function to save CAN frame data locally to a CSV file
void save_to_local(const canfd_frame &frame) {
    std::ofstream file("can_data.csv", std::ios_base::app);
    if (!file) {
        std::cerr << "Error opening file for local storage." << std::endl;
        set_gpio(GPIO_ERR, true);
        return;
    }

    file << std::hex << frame.can_id << ", ";
    for (int i = 0; i < frame.len; ++i) {
        file << std::setw(2) << std::setfill('0') << static_cast<int>(frame.data[i]) << " ";
    }
    file << std::endl;

    file.close();
}

// Function to format the CAN frame data as a JSON string for MQTT
std::string format_frame_as_json(const canfd_frame &frame) {
    std::ostringstream oss;
    oss << "{ \"can_id\": " << std::hex << frame.can_id << ", \"data\": [";
    for (int i = 0; i < frame.len; ++i) {
        oss << "0x" << std::setw(2) << std::setfill('0') << static_cast<int>(frame.data[i]);
        if (i < frame.len - 1) oss << ", ";
    }
    oss << "] }";
    return oss.str();
}

// Function to publish CAN frame data to Mapache via MQTT
void publish_to_mapache(const canfd_frame &frame, mqtt::async_client &client) {
    std::string payload = format_frame_as_json(frame);
    mqtt::message_ptr message = mqtt::make_message(TOPIC, payload);
    message->set_qos(1);

    try {
        client.publish(message)->wait_for(std::chrono::milliseconds(1000));
        std::cout << "Published message to Mapache: " << payload << std::endl;
        set_gpio(GPIO_TX, true);
        usleep(50000); // Delay for LED indication
        set_gpio(GPIO_TX, false);
    } catch (const mqtt::exception &exc) {
        std::cerr << "Error publishing to Mapache: " << exc.what() << std::endl;
        set_gpio(GPIO_ERR, true);
    }
}

// Main loop to receive CAN data and handle storage and publishing
void receive_can_data(int socket_fd, mqtt::async_client &client) {
    struct canfd_frame frame;
    while (true) {
        int nbytes = read(socket_fd, &frame, sizeof(struct canfd_frame));
        if (nbytes > 0) {
            set_gpio(GPIO_RX, true);   // Blink RX LED
            save_to_local(frame);      
            publish_to_mapache(frame, client);
            set_gpio(GPIO_RX, false);
        } else if (nbytes < 0) {
            perror("CAN read error");
            set_gpio(GPIO_ERR, true);
        }
    }
}

int main() {
    // Initialize GPIOs
    init_gpio(GPIO_RX);
    init_gpio(GPIO_TX);
    init_gpio(GPIO_ERR);

    // Initialize CAN socket
    int can_socket = open_can_socket(CAN_INTERFACE);
    if (can_socket < 0) return -1;

    // Initialize MQTT client
    mqtt::async_client client(MQTT_BROKER, MQTT_CLIENT_ID);
    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);

    try {
        std::cout << "Connecting to MQTT broker..." << std::endl;
        client.connect(connOpts)->wait();
        std::cout << "Connected to MQTT broker." << std::endl;
    } catch (const mqtt::exception &exc) {
        std::cerr << "MQTT connection error: " << exc.what() << std::endl;
        set_gpio(GPIO_ERR, true);
        close(can_socket);
        return -1;
    }

    // Start receiving CAN data
    receive_can_data(can_socket, client);

    // Clean up
    client.disconnect()->wait();
    close(can_socket);
    std::cout << "Disconnected and CAN socket closed." << std::endl;
    return 0;
}