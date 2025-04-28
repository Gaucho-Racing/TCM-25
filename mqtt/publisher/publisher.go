package publisher

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
	"mqtt/database"
	"mqtt/mqtt"
	"net"
	"time"
)

func PublishData(nodeID string, messageID string, targetID string, data []byte) {
	topic := fmt.Sprintf("gr25/gr25-main/%s/%s", nodeID, messageID)
	timestamp := time.Now().UnixMilli()
	source := nodeID
	target := targetID

	token := mqtt.Client.Publish(topic, 0, true, data)
	if token.Wait() && token.Error() != nil {
		log.Printf("MQTT publish error: %v", token.Error())
	}

	err := database.DB.Exec(`
		INSERT INTO gr25 (timestamp, topic, data, synced, source, target)
		VALUES (?, ?, ?, ?, ?, ?)`,
		timestamp, topic, data, 0, source, target).Error
	if err != nil {
		log.Printf("DB insert error: %v", err)
	}

	buf := new(bytes.Buffer)
	binary.Write(buf, binary.BigEndian, timestamp)
	buf.Write([]byte{0x00, 0x00})
	buf.Write(data)

	token = mqtt.Client.Publish(topic, 0, true, buf.Bytes())
	if token.Wait() && token.Error() != nil {
		log.Printf("MQTT re-publish error: %v", token.Error())
	} else {
		fmt.Printf("Published to %s\n", topic)
	}
}

func StartUDPServer(port int) {
	addr := net.UDPAddr{
		Port: port,
		IP:   net.ParseIP("0.0.0.0"),
	}
	conn, err := net.ListenUDP("udp", &addr)
	if err != nil {
		log.Fatalf("Failed to start UDP server: %v", err)
	}
	defer conn.Close()

	buffer := make([]byte, 1024)
	for {
		n, remoteAddr, err := conn.ReadFromUDP(buffer)
		if err != nil {
			log.Printf("Error reading from UDP: %v", err)
			continue
		}
		log.Printf("Received %d bytes from %s", n, remoteAddr.String())

		if n == 70 {
			// Splits canID bytes into hex digits/4 bits
			// canID := fmt.Sprintf("%x%x%x%x%x%x%x%x",
			// 	(buffer[0]&0xF0)>>4, buffer[0]&0x0F,
			// 	(buffer[1]&0xF0)>>4, buffer[1]&0x0F,
			// 	(buffer[2]&0xF0)>>4, buffer[2]&0x0F,
			// 	(buffer[3]&0xF0)>>4, buffer[3]&0x0F,
			// )
			grID := fmt.Sprintf("%x%x", buffer[0]&0x0F, (buffer[1]&0xF0)>>4) // 0th hex digit skipped
			msgID := fmt.Sprintf("%x%x%x", buffer[1]&0x0F, (buffer[2]&0xF0)>>4, buffer[2]&0x0F)
			targetID := fmt.Sprintf("%x%x", (buffer[3]&0xF0)>>4, buffer[3]&0x0F)
			// bus := buffer[4]
			// length := buffer[5]
			payload := buffer[6:n]

			PublishData(grID, msgID, targetID, payload)
		} else {
			log.Printf("Invalid packet size: expected 70 bytes, got %d", n)
		}

	}
}
