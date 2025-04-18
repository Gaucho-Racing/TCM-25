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

func PublishData(nodeID, messageID string, data []byte) {
	topic := fmt.Sprintf("gr25/gr25-main/%s/%s", nodeID, messageID)
	timestamp := time.Now().UnixMilli()

	token := mqtt.Client.Publish(topic, 0, true, data)
	if token.Wait() && token.Error() != nil {
		log.Printf("MQTT publish error: %v", token.Error())
	}

	err := database.DB.Exec(`
		INSERT INTO gr25 (timestamp, topic, data, synced)
		VALUES (?, ?, ?, ?)`,
		timestamp, topic, data, 0).Error
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

		if n >= 8 {
			nodeID := fmt.Sprintf("%02x%02x%02x%02x", buffer[0], buffer[1], buffer[2], buffer[3])
			msgID := fmt.Sprintf("0x%02x%02x%02x%02x", buffer[4], buffer[5], buffer[6], buffer[7])
			payload := buffer[8:n]
			PublishData(nodeID, msgID, payload)
		}
	}
}
