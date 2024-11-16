# TODO:
# config mcp2518
#     reset, can control register config, fifo reg setup
# receive data from fifos


import Jetson.GPIO as GPIO
import spidev
import os
import time
import logging

CS_PIN = 24
SCK_PIN = 23
MOSI_PIN = 19
MISO_PIN = 21

SPI_BUS = 0
SPI_DEVICE = 0
SPI_SPEED = 20000000 

spi = spidev.SpiDev()
# Use xfer2 to transfer data
# Use xfer3 to read data
logging.basicConfig(filename='test_CANFD.log', level=logging.DEBUG)

def init_gpio():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(CS_PIN, GPIO.OUT,initial=GPIO.HIGH)

def init_spi():
    try:
        spi.open(SPI_BUS, SPI_DEVICE)
        spi.max_speed_hz = SPI_SPEED
        spi.mode = 0b00
        return True
    except Exception as e:
        print(f"Failed to open SPI device: {e}")
        return False

def reset_mcp2518fd(spi):
    try:
        GPIO.output(CS_PIN, GPIO.LOW)
        balls = spi.xfer2([0x00])  # reset
        time.sleep(0.01)
        GPIO.output(CS_PIN, GPIO.HIGH)  
    except Exception as e:
        print(f"fuck {e}")

def config_mode(spi):
    # request config mode
    ctrl_register = 0x00# page 26, can control register
    spi.xfer2([0x02,ctrl_register,0x80]) # write, register, command
    time.sleep(0.01) 

def check_config_mode(spi):
    ctrl_register = 0x00

    response = spi.xfer2([0x03, ctrl_register, 0x00, 0x00, 0x00, 0x00]) # read, reg, 4x dummy
    
    mode = (response[0] << 24) | (response[1] << 16) | (response[2] << 8) | response[3]
    
    # Extract REQOP bits (24-26) and check if they are 0b100 (configuration mode)
    reqop = (mode >> 24) & 0x07  # Shift down by 24 bits, mask to 3 bits
    return reqop == 0b100
init_gpio()
init_spi()
reset_mcp2518fd(spi)
config_mode(spi)

if check_config_mode(spi):
    print("w config mode")
else:
    print("FFS")

spi.close()