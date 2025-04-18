package mqtt

import (
	"encoding/binary"
	"fmt"
	"log"
	"mqtt/database"
	"mqtt/mqtt"

	mq "github.com/eclipse/paho.mqtt.golang"
)

func StartSyncSubscriber(vehicleID string) {
	topic := fmt.Sprintf("gr25/%s/#", vehicleID)

	mqtt.Client.Subscribe(topic, 0, func(client mq.Client, msg mq.Message) {
		if len(msg.Payload()) < 8 {
			log.Printf("Invalid payload length: %d", len(msg.Payload()))
			return
		}

		timestamp := int64(binary.BigEndian.Uint64(msg.Payload()[:8]))
		topicStr := msg.Topic()

		// update synced = 1 where timestamp and topic match
		result := database.DB.Exec(`
			UPDATE gr25 SET synced = 1 WHERE timestamp = ? AND topic = ?`, timestamp, topicStr)

		if result.Error != nil {
			log.Printf("Failed to update synced: %v", result.Error)
		} else {
			log.Printf("[SYNC] Confirmed sync for %s at %d", topicStr, timestamp)
		}
	})
}
