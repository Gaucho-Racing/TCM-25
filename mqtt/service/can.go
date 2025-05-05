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
	0x00: "DTI Inverter", // ???
	0x01: "Debugger",
	0x02: "ECU",
	0x03: "ACU",
	0x04: "TCM",
	0x05: "Dash Panel",
	0x06: "Steering Wheel",
	0x07: "", // no name
	0x08: "GR Inverter 1",
	0x09: "GR Inverter 2",
	0x0A: "GR Inverter 3",
	0x0B: "GR Inverter 4",
	0x0C: "Charging SDC",
	0x0D: "Fan Controller 1",
	0x0E: "Fan Controller 2",
	0x0F: "Fan Controller 3",
	0x10: "Fan Controller 4",
	0x11: "Fan Controller 5",
	0x12: "Fan Controller 6",
	0x13: "Fan Controller 7",
	0x14: "Fan Controller 8",
	0x15: "SAM1",
	0x16: "SAM2",
	0x17: "SAM3",
	0x18: "SAM4",
	0x19: "SAM5",
	0x1A: "SAM6",
	0x1B: "SAM7",
	0x1C: "SAM8",
	0x1D: "SAM9",
	0x1E: "SAM10",
	0x1F: "SAM11",
	0x20: "SAM12",
	0x21: "SAM13",
	0x22: "SAM14",
	0x23: "SAM15",
	0x24: "SAM16",
	0x25: "SAM17",
	0x26: "SAM18",
	0x27: "SAM19",
	0x28: "SAM20",
	0x29: "LV DC-DC",
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
