// SPDX-License-Identifier: GPL-2.0-only
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define DEVICE "/dev/spidev1.1" // Change if needed

static void pabort(const char *s)
{
	perror(s);
	abort();
}

int main()
{
	int fd;
	int ret;
	uint8_t tx[2] = {0x00, 0x00}; // Data to send
	uint8_t rx[2] = {0};          // Buffer to store received data
	uint32_t speed = 2000000;    // 20 MHz
	uint8_t bits = 8;             // 8 bits per word
	uint32_t mode = SPI_MODE_0;   // SPI mode 0 (CPOL = 0, CPHA = 0)

	// Open SPI device
	fd = open(DEVICE, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	// Set SPI mode
	ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	// Set bits per word
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	// Set max speed
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed");

	// Print configuration
	printf("SPI Configuration:\n");
	printf("Mode: %d\n", mode);
	printf("Bits per word: %d\n", bits);
	printf("Speed: %d Hz (%d kHz)\n", speed, speed / 1000);

	// Configure SPI transfer
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = sizeof(tx),
		.speed_hz = speed,
		.bits_per_word = bits,
		.delay_usecs = 0,
	};

	// Perform SPI transaction
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

	// Print received data
	printf("Received data:\n");
	for (int i = 0; i < sizeof(rx); i++) {
		printf("0x%02X ", rx[i]);
	}
	printf("\n");

	// Close SPI device
	close(fd);

	return 0;
}
