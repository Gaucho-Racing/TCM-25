#include "stm32f4xx_hal.h"
#include "spi.h"
#include "can_fd.h" // include can-utils?


#define CAN_ID 0x101
#define DATA_LENGTH 8

void setup_CANFD() {
    HAL_SPI_Init(&hspi1); 

    CANFD_Init(&hspi1, CAN_FD_MODE, CAN_SPEED_500K, DATA_SPEED_2M);
}

void send_pressure_data(uint16_t pressure_value) {
    uint8_t can_data[DATA_LENGTH];
    can_data[0] = (pressure_value >> 8) & 0xFF; 
    can_data[1] = pressure_value & 0xFF;  

    CANFD_Frame frame;
    frame.can_id = CAN_ID;
    frame.data_length = DATA_LENGTH;
    frame.data = can_data;

    // Send CAN FD frame
    // LF FUNCTION FOR THIS
}

int main() {
    // Setup CAN FD
    setup_CANFD();

    while (1) {
        //uint16_t pressure_value = HOW TF DO WE READ THE PRESSURE VALUE?
        send_pressure_data(pressure_value);

        HAL_Delay(100); 
    }
}
