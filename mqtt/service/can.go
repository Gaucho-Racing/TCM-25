package service

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
	"mqtt/database"
	"mqtt/mqtt"
	"mqtt/utils"
	"net"
	"strconv"
	"time"
)

var nodeIDMap = map[byte]string{
	0x00: "dti_inverter", // ???
	0x01: "debugger",
	0x02: "ecu",
	0x03: "acu",
	0x04: "tcm",
	0x05: "dash_panel",
	0x06: "steering_wheel",
	0x07: "", // no name
	0x08: "gr_inverter_1",
	0x09: "gr_inverter_2",
	0x0A: "gr_inverter_3",
	0x0B: "gr_inverter_4",
	0x0C: "charging_sdc",
	0x0D: "fan_controller_1",
	0x0E: "fan_controller_1",
	0x0F: "fan_controller_1",
	0x10: "fan_controller_1",
	0x11: "fan_controller_1",
	0x12: "fan_controller_1",
	0x13: "fan_controller_1",
	0x14: "fan_controller_1",
	0x15: "sam1",
	0x16: "sam2",
	0x17: "sam3",
	0x18: "sam4",
	0x19: "sam5",
	0x1A: "sam6",
	0x1B: "sam7",
	0x1C: "sam8",
	0x1D: "sam9",
	0x1E: "sam10",
	0x1F: "sam11",
	0x20: "sam12",
	0x21: "sam13",
	0x22: "sam14",
	0x23: "sam15",
	0x24: "sam16",
	0x25: "sam17",
	0x26: "sam18",
	0x27: "sam19",
	0x28: "sam20",
	0x29: "lv_dc_dc",
}

func PublishData(nodeID byte, messageID string, targetID byte, data []byte) {
	source := nodeIDMap[nodeID]
	target := nodeIDMap[targetID]
	topic := fmt.Sprintf("gr25/gr25-main/%s/%s", source, messageID)
	timestamp := time.Now().UnixMilli()

	err := database.DB.Exec(`
		INSERT INTO gr25 (timestamp, topic, data, synced, source_node, target_node)
		VALUES (?, ?, ?, ?, ?, ?)`,
		timestamp, topic, data, 0, source, target).Error
	if err != nil {
		log.Printf("DB insert error: %v", err)
	}

	buf := new(bytes.Buffer)
	binary.Write(buf, binary.BigEndian, timestamp)
	buf.Write([]byte{0x00, 0x00})
	buf.Write(data)

	token := mqtt.Client.Publish(topic, 0, true, buf.Bytes())
	if token.Wait() && token.Error() != nil {
		log.Printf("MQTT re-publish error: %v", token.Error())
	} else {
		fmt.Printf("Published to %s\n", topic)
	}
}

func ListenCAN(port string) {
	portInt, err := strconv.Atoi(port)
	if err != nil {
		utils.SugarLogger.Fatalf("Failed to convert port to int: %v", err)
	}
	addr := net.UDPAddr{
		Port: portInt,
		IP:   net.ParseIP("0.0.0.0"),
	}
	conn, err := net.ListenUDP("udp", &addr)
	if err != nil {
		utils.SugarLogger.Fatalf("Failed to start UDP server: %v", err)
	}
	defer conn.Close()

	buffer := make([]byte, 1024)
	for {
		n, remoteAddr, err := conn.ReadFromUDP(buffer)
		if err != nil {
			utils.SugarLogger.Errorf("Error reading from UDP: %v", err)
			continue
		}
		utils.SugarLogger.Infof("Received %d bytes from %s", n, remoteAddr.String())

		if n == 70 {
			// Splits canID bytes into hex digits/4 bits
			// canID := fmt.Sprintf("%x%x%x%x%x%x%x%x",
			// 	(buffer[0]&0xF0)>>4, buffer[0]&0x0F,
			// 	(buffer[1]&0xF0)>>4, buffer[1]&0x0F,
			// 	(buffer[2]&0xF0)>>4, buffer[2]&0x0F,
			// 	(buffer[3]&0xF0)>>4, buffer[3]&0x0F,
			// )
			grID := (buffer[0] & 0x0F) | ((buffer[1] & 0xF0) >> 4) // 0th hex digit skipped
			msgID := fmt.Sprintf("0x%x%x%x", buffer[1]&0x0F, (buffer[2]&0xF0)>>4, buffer[2]&0x0F)
			targetID := ((buffer[3] & 0xF0) >> 4) | (buffer[3] & 0x0F)
			// bus := buffer[4]
			// length := buffer[5]
			payload := buffer[6:n]

			PublishData(grID, msgID, targetID, payload)
		} else {
			utils.SugarLogger.Infof("Invalid packet size: expected 70 bytes, got %d", n)
		}

	}
}
