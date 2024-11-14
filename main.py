# Test code for CANFD
# Once completed someone please update main.cpp



import Jetson.GPIO as GPIO
import spidev
import os
import time
import logging

# Define GPIO 
CS_PIN = 24
SCK_PIN = 23
MOSI_PIN = 19
MISO_PIN = 21

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

# Map of CAN IDs to frame size
# ffs please update the spreadsheet https://docs.google.com/spreadsheets/d/1XfJhhAQoDnuSuwluNitPsDWtuQu-bP-VbEGPmSo5ujA/edit?gid=68138563#gid=68138563
sensor_frame_sizes = {
    "00000000": 8, # no data being received
    "0x169420": 8, # test canfd, 11/13 can 2.0 transmission test
    "0x100102": "dude",
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
    GPIO.setup(CS_PIN, GPIO.OUT,initial=GPIO.HIGH)
    # GPIO.setup(SCK_PIN, GPIO.OUT,initial=GPIO.HIGH)
    # GPIO.setup(MOSI_PIN, GPIO.OUT,initial=GPIO.HIGH)
    # GPIO.setup(MISO_PIN, GPIO.OUT,initial=GPIO.HIGH)

def init_spi():
    try:
        spi.open(SPI_BUS, SPI_DEVICE)
        spi.max_speed_hz = SPI_SPEED
        spi.mode = 0
        return True
    except Exception as e:
        print(f"Failed to open SPI device: {e}")
        return False
    
def set_can_baud_rate(spi):
    NBTCFG = [0x00,0x01,0x14,0x03] # 1mbps baud
    DBTCFG = [0x00,0x00,0x05,0x01] # 8Mbps

    spi.xfer2([0x04]+NBTCFG)
    spi.xfer2([0x10]+DBTCFG)

def set_normal_mode(spi):
    mode_command = [0x01,0x00]
    spi.xfer2(mode_command)
    print("Normal mode set")

def check_mode(spi):
    response = spi.xfer2([0x01,0x00])
    mode = response[1]
    if mode == 0x00:
        print("Normal")
    else:
        print(f"Diff mode: {hex(mode)}")

def configure_fifo_as_receive():
    # Example settings - replace with actual values from your datasheet
    fifo_control_register = 0x5C  # Example address for FIFO control
    fifo_receive_mode = 0x01      # Set FIFO to receive mode (check datasheet)

    # Write to FIFO control register to enable receive mode
    spi.xfer2([0x02, fifo_control_register, fifo_receive_mode])
    print("FIFO set to receive mode")

def get_CANID():
    frame_size = 4 # 3 bytes per CANID?
    tx = [0] * frame_size
    try:
        #GPIO.output(CS_PIN,GPIO.LOW)
        rx = spi.xfer2(tx)
        CANID = ''.join(f"{byte:02X}" for byte in rx)
        
        with open("data_check_CANFD.log", "a") as logfile:
            logfile.write(f"test_CANID: {CANID}\n")
        
        return CANID
    except Exception as e:
        print(f"Failed to communicate over SPI: {e}")
        return None

# Example fn to read reg
def ReadReg(self, reg_address):
    self.bus.open(self.spi_bus_number, self.spi_dev_number)
    tx = [reg_address | self.__READ_FLAG, 0x00]
    rx = self.bus.xfer2(tx)
    self.bus.close()
    return rx[1]

def read_can_fd_data():
    frame_size = 8
    #frame_size = sensor_frame_sizes[get_CANID()]
    #tx = [0] * frame_size
    #tx = [0x03,0x60,0x00]
    reg_address = 0x60
    tx = [reg_address | 0x80] + [0x00] * frame_size # frame size determines the amount of dummy bytes to send to get data back
    try:
        GPIO.output(CS_PIN,GPIO.LOW) # pin must be set to low to read data
        rx = spi.xfer2(tx) # each item is sent to the register and the register responds with data back that will populate rx
        print(f"rx0{rx[0]}")
        print(f"rx1{rx[1]}")
        #status_response = spi.xfer2([0x03,0x60,0x00])
        #print(f"status: {status_response}")
        # Hopefully this shit is useless
        if rx[1] & 0x01:
            fifo_user_address_register = 0x64
            user_address_response = spi.xfer2([0x03, fifo_user_address_register, 0x00, 0x00])
            print(user_address_response)
            data_address = (user_address_response[2] << 8) | user_address_response[3]
            print(data_address)
            # 3. Use the address to read the CAN message data (adjust the number of bytes as needed)
            message_response = spi.xfer2([0x03, data_address, 0x00] + [0x00] * 64)
            print("HOLY SHIT BRO IT RECEIVES SHIT")
            print("HOLY SHIT BRO IT RECEIVES SHIT")
            print("HOLY SHIT BRO IT RECEIVES SHIT")
            print("HOLY SHIT BRO IT RECEIVES SHIT")

            # 4. Parse the CAN message data
            print("CAN message data:", message_response)

        print(f"readbytes {spi.readbytes(8)}")
        
        data = ''.join(f"{byte:02X}" for byte in rx)
        print(f"Data being received {data}")
        # if data[0:3] != "000:
        #     print("HOLY SHIT BRO IT RECEIVES SHIT")
        #     print("HOLY SHIT BRO IT RECEIVES SHIT")
        #     print("HOLY SHIT BRO IT RECEIVES SHIT")
        #     print("HOLY SHIT BRO IT RECEIVES SHIT")
        #     print("HOLY SHIT BRO IT RECEIVES SHIT")
        with open("data_check_CANFD.log", "a") as logfile:
            logfile.write(f"test: {data}\n")
        GPIO.output(CS_PIN,GPIO.HIGH)
        return data
    except Exception as e:
        print(f"Failed to communicate over SPI: {e}")
        return None

def close_spi():
    spi.close()

def sort_sensor_data(data):
    can_id = data[:9]
    return sensor_files.get(can_id, "test_misc_sensor_data.log")

def log_data(data):
    with open("test_CANFD.log", "a") as logfile:
        logfile.write(f"Raw: {data}\n")

def log_actual_data(data):
    with open("test_CANFD_actual_fucking_working.log", "a") as logfile:
        logfile.write(f"Raw: {data}\n")

def main():
    init_gpio()
    if not init_spi():
        return
    set_can_baud_rate(spi)
    set_normal_mode(spi)
    check_mode(spi)
    configure_fifo_as_receive()
    try:
        while True:
            #if GPIO.input(INT_PIN) == GPIO.HIGH:
            #print("CANFD data being read idfk")
            data = read_can_fd_data()
            #print(data)
            if data:
                #print("Data actually being recorded lol")
                # if data != "0000000000000000":
                #     print("HOLY SHIT BRO IT RECEIVES SHIT")
                #     print("HOLY SHIT BRO IT RECEIVES SHIT")
                #     print("HOLY SHIT BRO IT RECEIVES SHIT")
                #     print("HOLY SHIT BRO IT RECEIVES SHIT")
                #     print("HOLY SHIT BRO IT RECEIVES SHIT")
                #     log_actual_data(data)
                log_data(data)
                # someone add send data function

            time.sleep(0.01)
    except KeyboardInterrupt:
        pass
    finally:
        GPIO.cleanup()
        close_spi()

if __name__ == "__main__":
    main()
