#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

// Using the same CAN struct format as the main program
struct CAN {
    union {
        uint16_t buffer[35];
        struct {
            uint32_t ID;
            uint8_t bus;
            uint8_t length;
            uint8_t data[64];
        } split;
    } combined;
};

// Function to create a mock CAN message
void createMockCANMessage(struct CAN *frame) {
    // Generate CAN ID components
    // uint8_t gr_id = rand() % 0xFF;        // 1 byte for GR ID (sending node)
    // uint16_t msg_id = rand() % 0xFFFF;    // 2 bytes for message ID
    // uint8_t target_id = rand() % 0xFF;    // 1 byte for target ID

    uint8_t gr_id = 0x02;
    uint16_t msg_id = 0x003;
    uint8_t target_id = 0xFF;
    
    // Combine components into final CAN ID
    // Format: GR_ID (bits 31-24) | MSG_ID (bits 23-8) | TARGET_ID (bits 7-0)
    frame->combined.split.ID = (gr_id << 24) | (msg_id << 8) | target_id;
    
    // Random bus number (0 or 1)
    frame->combined.split.bus = rand() % 2;
    
    // Random length between 1 and 64 bytes
    frame->combined.split.length = (rand() % 64) + 1;
    
    // Fill data with random values
    for (int i = 0; i < frame->combined.split.length; i++) {
        frame->combined.split.data[i] = rand() % 256;
    }
}

// Function to print CAN message details
void printCANMessage(struct CAN *frame) {
    printf("CAN Message:\n");
    printf("ID: 0x%X\n", frame->combined.split.ID);
    printf("Bus: %d\n", frame->combined.split.bus);
    printf("Length: %d\n", frame->combined.split.length);
    printf("Data: ");
    for (int i = 0; i < frame->combined.split.length; i++) {
        printf("%02X ", frame->combined.split.data[i]);
    }
    printf("\n\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IP address> <port>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct sockaddr_in servaddr;
    struct CAN mockFrame;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Set server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    // Initialize random seed
    srand(time(NULL));

    printf("Starting mock CAN message sender...\n");
    printf("Sending to %s:%s\n", argv[1], argv[2]);

    while (1) {
        // Create and send a mock message
        createMockCANMessage(&mockFrame);
        printCANMessage(&mockFrame);

        if (sendto(sockfd, &mockFrame, sizeof(struct CAN), 0,
                   (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            perror("Send failed");
            exit(1);
        }

        // Wait for 1 second before sending next message
        usleep(100);
    }

    close(sockfd);
    return 0;
}