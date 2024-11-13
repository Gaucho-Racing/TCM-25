import Jetson.GPIO as GPIO
import spidev
import os
import time
import logging

# Define GPIO pin numbers for CS and INT
CS_PIN = None  # Assign actual GPIO pin number for CS
INT_PIN = 32   # Connect to PIN 32

SPI_BUS = 0
SPI_DEVICE = 0
SPI_SPEED = 20000000  # 20 MHz

# Map of CAN IDs to file names
sensor_files = {
    "0x100102": "ecu.log",
    "0x100103": "acu.log",
    "0x100105": "dash_panel.log",
    "0x100106": "steering_wheel.log",
    "0x100107": "inverter1.log",
    "0x100108": "inverter2_panel.log",
    "0x10010C": "sam1_panel.log",
    "0x10010D": "sam2_panel.log"
}

spi = spidev.SpiDev()
logging.basicConfig(filename='test_CANFD.log', level=logging.DEBUG)

def init_gpio():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(CS_PIN, GPIO.OUT)
    GPIO.setup(INT_PIN, GPIO.IN)

def init_spi():
    try:
        spi.open(SPI_BUS, SPI_DEVICE)
        spi.max_speed_hz = SPI_SPEED
        spi.mode = 0
        return True
    except Exception as e:
        print(f"Failed to open SPI device: {e}")
        return False

def read_can_fd_data():
    frame_size = 64
    tx = [0] * frame_size
    try:
        rx = spi.xfer2(tx)
        data = ''.join(f"{byte:02X}" for byte in rx)
        
        with open("data_check_CANFD.log", "a") as logfile:
            logfile.write(f"test: {data}\n")
        
        return data
    except Exception as e:
        print(f"Failed to communicate over SPI: {e}")
        return None

def close_spi():
    spi.close()

def sort_sensor_data(data):
    can_id = data[:9]
    return sensor_files.get(can_id, "misc_sensor_data.log")

def log_data(data):
    with open("test_CANFD.log", "a") as logfile:
        logfile.write(f"Raw: {data}\n")

def main():
    init_gpio()
    if not init_spi():
        return

    try:
        while True:
            if GPIO.input(INT_PIN) == GPIO.HIGH:
                data = read_can_fd_data()
                if data:
                    log_data(data)
                    # Send data if needed

            time.sleep(0.01)
    except KeyboardInterrupt:
        pass
    finally:
        GPIO.cleanup()
        close_spi()

if __name__ == "__main__":
    main()
