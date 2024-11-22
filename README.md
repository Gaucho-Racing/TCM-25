# Jetson Orin Nano TCM
This is the code to process incoming CAN data from our sensors and then forward it to mapache. 

# The Setup
MCP2516FD is a FD CAN controller that will be receiving all flow from FD CAN sensor nodes. Interfaced through SPI, and processing data through a library to be sent to both Mapache via Singlestore database (MQTT) and the Orin Nano locally as a black box solution. 

# Usage
```
git clone https://github.com/Gaucho-Racing/TCM-Jelqing.git

gcc -Wall -o main main.c -ljetgpio
sudo ./main
```

# Acknowledgements
![alt text](https://github.com/Gaucho-Racing/TCM-Jelqing/blob/main/image.png?raw=true)
![alt text](https://github.com/Gaucho-Racing/TCM-Jelqing/blob/main/IMG_1329.png?raw=true)




