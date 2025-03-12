#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SERVER "127.0.0.1"
#define PORT 8888
#define PACKET_SIZE 70
#define TOTAL_PACKETS 1000000  // Send 1 million packets

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char message[PACKET_SIZE];
    memset(message, 'A', PACKET_SIZE);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Increase send buffer size
    int bufsize = 1048576;  // 1 MB
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER, &server_addr.sin_addr);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    for (int i = 0; i < TOTAL_PACKETS; i++) {
        sendto(sockfd, message, PACKET_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    }

    gettimeofday(&end, NULL);
    double time_taken = (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_usec - start.tv_usec); // Convert to microseconds
    double throughput = (TOTAL_PACKETS * PACKET_SIZE * 8) / (time_taken / 1e6);  // bits per second

    printf("Sent %d packets in %.2f ms\n", TOTAL_PACKETS, time_taken / 1000);
    printf("Achieved Throughput: %.2f Mbps\n", throughput / 1e6);

    close(sockfd);
    return 0;
}

