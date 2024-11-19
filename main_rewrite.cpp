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
#include "drv_canfdspi_api.h"
#include <jetgpio.h>

// I NEED SOMEONE TO CHECK MY WORK
// we need to config our mcp2518fd first( use https://www.microchip.com/en-us/tools-resources/configure/mplab-harmony/configurator)
//    to do this we reset and then config registers
//    we also need to config the filter register
// then we set to normal mode and we should be good to go 

const int CS_PIN = 24; 
const int INT_PIN = 32;
#define SPI_BUS 0     
#define SPI_DEVICE 0   
#define SPI_SPEED 20000000 // MCP2518FD should be 20 Mhz

constexpr int MAX_DATA_BYTES = 64;

int spi_fd;

void initGPIO() {
    GPIO::setmode(GPIO::BCM);
    GPIO::setup(CS_PIN, GPIO::OUT);
    GPIO::setup(INT_PIN, GPIO::IN); 
}

void configureCANFilter() {
    // Disable Filter 0 before configuration
    DRV_CANFDSPI_FilterDisable(DRV_CANFDSPI_INDEX_0, CAN_FILTER0);

    // Configure Filter Object 0: Match SID = 0x300, standard frames only
    CAN_FILTEROBJ_ID fObj;
    fObj.SID = 0x300;  // Standard ID 0x300
    fObj.SID11 = 0;    // Not used for standard IDs
    fObj.EID = 0;      // Extended ID not used
    fObj.EXIDE = 0;    // Only accept standard frames

    DRV_CANFDSPI_FilterObjectConfigure(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, &fObj);

    // Configure Mask Object 0: Match IDE bit, ignore rest
    CAN_MASKOBJ_ID mObj;
    mObj.MSID = 0x7F0; // Mask for SID (0 means "don't care")
    mObj.MSID11 = 0;   // Not used for standard IDs
    mObj.MEID = 0;     // Mask extended ID
    mObj.MIDE = 1;     // Match IDE bit

    DRV_CANFDSPI_FilterMaskConfigure(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, &mObj);

    // Link Filter 0 to RX FIFO 2 and enable it
    bool filterEnable = true;
    DRV_CANFDSPI_FilterToFifoLink(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, CAN_FIFO_CH2, filterEnable);

    std::cout << "CAN Filter 0 configured to accept SID 0x300 with matching IDE bit." << std::endl;
}

void configureInterrupts(){
    DRV_CANFDSPI_ModuleEventClear(DRV_CANFDSPI_Index_0(), CAN_ALL_EVENTS);


    DRV_CANFDSPI_TransmitChannelEventEnable(DRV_CANFDSPI_INDEX_0, CAN_TXQUEUE_CH0,CAN_TX_FIFO_NOT_FULL_EVENT);
    DRV_CANFDSPI_ReceiveChannelEventEnable(DRV_CANFDSPI_INDEX_0, CAN_FIFO_CH2,CAN_RX_FIFO_NOT_EMPTY_EVENT);
    DRV_CANFDSPI_ModuleEventEnable(DRV_CANFDSPI_INDEX_0, CAN_TX_EVENT | CAN_RX_EVENT);
}

void configureMCP2518FD() {
    // Reset device
    DRV_CANFDSPI_Reset(DRV_CANFDSPI_INDEX_0);

    // Oscillator Configuration: Divide by 10
    CAN_OSC_CTRL oscCtrl;
    DRV_CANFDSPI_OscillatorControlObjectReset(&oscCtrl);
    oscCtrl.ClkOutDivide = OSC_CLKO_DIV10;
    DRV_CANFDSPI_OscillatorControlSet(DRV_CANFDSPI_INDEX_0, &oscCtrl);

    // GPIO Configuration: Use INT0 and nINT1
    DRV_CANFDSPI_GpioModeConfigure(DRV_CANFDSPI_INDEX_0, GPIO_MODE_INT, GPIO_MODE_INT);

    // CAN Configuration: ISO CRC, TEF, TXQ enabled
    CAN_CONFIG canConfig;
    DRV_CANFDSPI_ConfigureObjectReset(&canConfig);
    canConfig.IsoCrcEnable = 1;
    canConfig.StoreInTEF = 1;
    canConfig.TXQEnable = 1;
    DRV_CANFDSPI_Configure(DRV_CANFDSPI_INDEX_0, &canConfig);

    // Bit Time Configuration: 500K/2M, 80% sample point
    DRV_CANFDSPI_BitTimeConfigure(DRV_CANFDSPI_INDEX_0, CAN_500K_2M, CAN_SSP_MODE_AUTO, CAN_SYSCLK_40M);

    // TEF Configuration: 12 messages, timestamp enabled
    CAN_TEF_CONFIG tefConfig;
    tefConfig.FifoSize = 11;
    tefConfig.TimeStampEnable = 1;
    DRV_CANFDSPI_TefConfigure(DRV_CANFDSPI_INDEX_0, &tefConfig);

    // TXQ Configuration: 8 messages, 32-byte payload, high priority
    CAN_TX_QUEUE_CONFIG txqConfig;
    DRV_CANFDSPI_TransmitQueueConfigureObjectReset(&txqConfig);
    txqConfig.TxPriority = 1;
    txqConfig.FifoSize = 7;
    txqConfig.PayLoadSize = CAN_PLSIZE_32;
    DRV_CANFDSPI_TransmitQueueConfigure(DRV_CANFDSPI_INDEX_0, &txqConfig);

    // FIFO 1: Transmit FIFO, 5 messages, 64-byte payload, low priority
    CAN_TX_FIFO_CONFIG txfConfig;
    DRV_CANFDSPI_TransmitChannelConfigureObjectReset(&txfConfig);
    txfConfig.FifoSize = 4;
    txfConfig.PayLoadSize = CAN_PLSIZE_64;
    txfConfig.TxPriority = 0;
    DRV_CANFDSPI_TransmitChannelConfigure(DRV_CANFDSPI_INDEX_0, CAN_FIFO_CH1, &txfConfig);

    // FIFO 2: Receive FIFO, 16 messages, 64-byte payload, timestamp enabled
    CAN_RX_FIFO_CONFIG rxfConfig;
    rxfConfig.FifoSize = 15;
    rxfConfig.PayLoadSize = CAN_PLSIZE_64;
    rxfConfig.RxTimeStampEnable = 1;
    DRV_CANFDSPI_ReceiveChannelConfigure(DRV_CANFDSPI_INDEX_0, CAN_FIFO_CH2, &rxfConfig);

    std::cout << "MCP2518FD configured!" << std::endl;
    configureCANFilter();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
    configureInterrupts();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
    // Enable ECC
    DRV_CANFDSPI_EccEnable(DRV_CANFDSPI_INDEX_0);
    // Initialize RAM
    DRV_CANFDSPI_RamInit(DRV_CANFDSPI_INDEX_0,0xff);
    // select normal mode
    DRV_CANFDSPI_OperationModeSelect(DRV_CANFDSPI_INDEX_0, CAN_NORMAL_MODE);
}

bool checkNewMessage(){
    CAN_RX_MSGOBJ rxObj;
    uint8_t rxd[MAX_DATA_BYTES];
    uint32_t ts;

    if (APP_RX_INT()){
        DRV_CANFDSPI_ReceiveMessageGet(DRV_CANFDSPI_INDEX_0, CAN_FIFO_CH2, &rxObj, rxd, MAX_DATA_BYTES);
        if (rxObj.bF.id.SID == 0x300 && rxObj.bF.ctrl.IDE == 0){
            Nop(); Nop();
            ts = rxObj.bF.timeStamp;
            return true;
        }
    }
    return false;
}

void readCANMessage() {
    CAN_RX_MSGOBJ rxObj;

    uint8_t rxd[MAX_DATA_BYTES];
    CAN_RX_FIFO_EVENT rxFlags;

    DRV_CANFDSPI_ReceiveChannelEventGet(DRV_CANFDSPI_INDEX_0, CAN_FIFO_CH2, &rxFlags);

    if (rxFlags & CAN_RX_FIFO_NOT_EMPTY_EVENT) {
        DRV_CANFDSPI_ReceiveMessageGet(DRV_CANFDSPI_INDEX_0, CAN_FIFO_CH2, &rxObj, rxd, MAX_DATA_BYTES);
        std::cout << "Received Data: ";
        for (int i = 0; i < MAX_DATA_BYTES; i++) {
            std::cout << std::hex << "0x" << (int)rxd[i] << " ";
        }
        std::cout << std::dec << std::endl; 
    }
}

int main() {
    initGPIO();
    int Init;
    int SPI_init;
    int SPI_stat;
    char tx[7] = {0,};
    char rx[7] = {0,};
    if (Init < 0)
    {
      /* jetgpio initialisation failed */
      printf("Jetgpio initialisation failed. Error code:  %d\n", Init);
      exit(Init);
    }
    else
    {
      /* jetgpio initialised okay*/
      printf("Jetgpio initialisation OK. Return code:  %d\n", Init);
    }

    SPI_init = spiOpen(1, 5000000, 0, 0, 8, 1, 1);
    if (SPI_init < 0)
        {
        /* Port SPI2 opening failed */
        printf("Port SPI2 opening failed. Error code:  %d\n", SPI_init);
        exit(Init);
        }
    else
        {
        /* Port SPI2 opened  okay*/
        printf("Port SPI2 opened OK. Return code:  %d\n", SPI_init);
        }
    configureMCP2518FD();
    while (true) {
        std::string data;
        // data for the below is on pg 61 of family data sheet
        // read 10.4 and 10.5 to check how to read combined status reg
        // CiRXIF for receive interrupt. reflected in CiRXIF.RFIF<m> flag
        // CiRXOVIF checks for overflow
        // ^ bit 0 is txq, bit 1 is fifo 1, bit 2 is fifo 2, etc to bit 31 is fifo 31
        // if receive fifo half full int. or fifo full int. 
        //     continue reading from fifo until no new message
        // if fifo overrun int
        //    what the fuck do i do here
        if (checkNewMessage()) { // regular interrupt(not full)
            readCANMessage();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
    }
    spiClose(SPI_init);
    gpioTerminate();
    GPIO::cleanup();
    return 0;
}
