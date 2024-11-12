# Jetson Orin Nano TCM
This is the code to process incoming CAN data from our sensors and then forward it to mapache. 

# The Setup
MCP2516FD is a FD CAN controller that will be receiving all flow from FD CAN sensor nodes. Interfaced through SPI, and processing data through a library to be sent to both Mapache via Singlestore database (MQTT) and the Orin Nano locally as a black box solution. 

# Por favor
If anyone knows what they're doing please reach out to Vin or Steven in the sensors channel and help us out. We haven't eaten in 10 days and Bharat won't let us out of his basement until we finish this. 

we might be cooked.

![alt text](https://github.com/Gaucho-Racing/TCM-Jelqing/image.png?raw=true)