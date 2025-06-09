package service

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"mqtt/config"
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

func PublishData(canID uint32, nodeID uint8, messageID uint16, targetID uint8, data []byte) {
	source := nodeIDMap[nodeID]
	target := nodeIDMap[targetID]

	if source == "" {
		source = "unknown"
	}
	if target == "" {
		target = "unknown"
	}

	topic := fmt.Sprintf("gr25/%s/%s/0x%03x", config.VehicleID, source, messageID)
	timestamp := uint64(time.Now().UnixMicro())

	// Queue the database write
	QueueDBWrite(int(timestamp), config.VehicleID, topic, data, source, target)

	buf := new(bytes.Buffer)
	binary.Write(buf, binary.BigEndian, timestamp)
	binary.Write(buf, binary.BigEndian, config.VehicleUploadKey)
	buf.Write(data)

	go func() {
		canIDString := fmt.Sprintf("%d", canID)
		lastSent, ok := config.LastSucessfulPublish.Get(canIDString)
		shouldPublish := false
		if ok {
			// 100000 us = 100ms
			if timestamp-lastSent > 100000 {
				shouldPublish = true
			}
		} else {
			shouldPublish = true
		}
		if shouldPublish {
			token := mqtt.Client.Publish(topic, 1, true, buf.Bytes())
			if token.WaitTimeout(10 * time.Second) {
				if token.Error() != nil {
					utils.SugarLogger.Errorf("[MQTT] Failed to publish to %s: %v", topic, token.Error())
				} else {
					utils.SugarLogger.Infof("[MQTT] Published to %s", topic)
					config.LastSucessfulPublish.Set(canIDString, timestamp)
				}
			} else {
				utils.SugarLogger.Errorf("[MQTT] Failed to publish to %s: %v", topic, token.Error())
			}
		}
	}()
}

func ListenCAN(port string) {
	shouldLog := false
	if config.Env == "DEV" {
		shouldLog = true
	}

	portInt, err := strconv.Atoi(port)
	if err != nil {
		utils.SugarLogger.Fatalf("[CAN] Failed to convert port to int: %v", err)
	}
	addr := net.UDPAddr{
		Port: portInt,
		IP:   net.ParseIP("0.0.0.0"),
	}
	conn, err := net.ListenUDP("udp", &addr)
	if err != nil {
		utils.SugarLogger.Fatalf("[CAN] Failed to create UDP connection: %v", err)
	}
	defer conn.Close()

	for {
		buffer := make([]byte, 1024)
		n, remoteAddr, err := conn.ReadFromUDP(buffer)
		if err != nil {
			utils.SugarLogger.Errorf("[CAN] Error reading from UDP: %v", err)
			continue
		}
		if shouldLog {
			utils.SugarLogger.Infof("[CAN] Received %d bytes from %s", n, remoteAddr.String())
		}

		if n < 70 {
			utils.SugarLogger.Infof("[CAN] Invalid packet size: expected at least 70 bytes, got %d", n)
			continue
		}

		// Debug print the raw canID bytes
		canIDBytes := buffer[0:4]
		canIDStr := make([]string, len(canIDBytes))
		for i, b := range canIDBytes {
			canIDStr[i] = fmt.Sprintf("0x%02x", b)
		}
		if shouldLog {
			utils.SugarLogger.Infof("[CAN] Raw CAN ID bytes: %v", canIDStr)
		}

		// Splits canID bytes into hex digits/4 bits
		canID := binary.LittleEndian.Uint32(buffer[0:4])
		if shouldLog {
			utils.SugarLogger.Infof("[CAN] CAN ID: %d (0x%08x)", canID, canID)
		}
		nodeID := uint8((canID >> 20) & 0xFF)
		msgID := uint16((canID >> 8) & 0xFFF)
		targetID := uint8(canID & 0xFF)

		if shouldLog {
			utils.SugarLogger.Infof("[CAN] Msg ID: %d (0x%03x)", msgID, msgID)
			utils.SugarLogger.Infof("[CAN] Node ID: %d (0x%02x)", nodeID, nodeID)
			utils.SugarLogger.Infof("[CAN] Target ID: %d (0x%02x)", targetID, targetID)
		}

		bus := buffer[4] // unused
		length := buffer[5]
		payload := buffer[6 : length+6]

		if shouldLog {
			utils.SugarLogger.Infof("[CAN] Bus: %d", bus)
			utils.SugarLogger.Infof("[CAN] Length: %d", length)
		}
		payloadStr := make([]string, len(payload))
		for i, b := range payload {
			payloadStr[i] = fmt.Sprintf("0x%02x", b)
		}
		if shouldLog {
			utils.SugarLogger.Infof("[CAN] Payload: %v", payloadStr)
		}

		go PublishData(canID, nodeID, msgID, targetID, payload)
	}
}
