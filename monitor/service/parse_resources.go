//THIS CODE IS ALL CHATGPT - still need to be rewritten to fit the actual system!!

package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"

	mq "github.com/eclipse/paho.mqtt.golang"
	"gorm.io/driver/sqlite"
	"gorm.io/gorm"
)

// Define the structure of the data we will receive
type ResourceData struct {
	CPUUtil     int     `json:"cpu_util"`
	GPUUtil     int     `json:"gpu_util"`
	MemoryUtil  int     `json:"memory_util"`
	StorageUtil int     `json:"storage_util"`
	PowerUsage  float32 `json:"power_usage"`
	CPUTemp     float32 `json:"cpu_temp"`
	GPUTemp     float32 `json:"gpu_temp"`
	Timestamp   int64   `json:"timestamp"`
}

// Define the database model for storing resource data
type Resource struct {
	ID          uint    `gorm:"primaryKey"`
	CPUUtil     int     `json:"cpu_util"`
	GPUUtil     int     `json:"gpu_util"`
	MemoryUtil  int     `json:"memory_util"`
	StorageUtil int     `json:"storage_util"`
	PowerUsage  float32 `json:"power_usage"`
	CPUTemp     float32 `json:"cpu_temp"`
	GPUTemp     float32 `json:"gpu_temp"`
	Timestamp   int64   `json:"timestamp"`
}

var db *gorm.DB

// MQTT callback function for when a message is received
func onMessageReceived(client mq.Client, msg mq.Message) {
	// Decode the binary message
	var data ResourceData

	// Read the values from the binary message (assuming 4 bytes for each field)
	reader := bytes.NewReader(msg.Payload())

	// Assuming the binary message has the following structure:
	// - CPU utilization (int)
	// - GPU utilization (int)
	// - Memory utilization (int)
	// - Storage utilization (int)
	// - Power usage (float32)
	// - CPU temperature (float32)
	// - GPU temperature (float32)
	// - Timestamp (int64)

	// Decode each field from the binary data
	err := binary.Read(reader, binary.BigEndian, &data.CPUUtil)
	if err != nil {
		log.Printf("Error decoding CPUUtil: %v", err)
		return
	}

	err = binary.Read(reader, binary.BigEndian, &data.GPUUtil)
	if err != nil {
		log.Printf("Error decoding GPUUtil: %v", err)
		return
	}

	err = binary.Read(reader, binary.BigEndian, &data.MemoryUtil)
	if err != nil {
		log.Printf("Error decoding MemoryUtil: %v", err)
		return
	}

	err = binary.Read(reader, binary.BigEndian, &data.StorageUtil)
	if err != nil {
		log.Printf("Error decoding StorageUtil: %v", err)
		return
	}

	err = binary.Read(reader, binary.BigEndian, &data.PowerUsage)
	if err != nil {
		log.Printf("Error decoding PowerUsage: %v", err)
		return
	}

	err = binary.Read(reader, binary.BigEndian, &data.CPUTemp)
	if err != nil {
		log.Printf("Error decoding CPUTemp: %v", err)
		return
	}

	err = binary.Read(reader, binary.BigEndian, &data.GPUTemp)
	if err != nil {
		log.Printf("Error decoding GPUTemp: %v", err)
		return
	}

	err = binary.Read(reader, binary.BigEndian, &data.Timestamp)
	if err != nil {
		log.Printf("Error decoding Timestamp: %v", err)
		return
	}

	// Upload the data to the database
	err = uploadToDatabase(data)
	if err != nil {
		log.Printf("Error uploading data to the database: %v", err)
	}
}

// Upload decoded data to the database
func uploadToDatabase(data ResourceData) error {
	resource := Resource{
		CPUUtil:     data.CPUUtil,
		GPUUtil:     data.GPUUtil,
		MemoryUtil:  data.MemoryUtil,
		StorageUtil: data.StorageUtil,
		PowerUsage:  data.PowerUsage,
		CPUTemp:     data.CPUTemp,
		GPUTemp:     data.GPUTemp,
		Timestamp:   data.Timestamp,
	}

	result := db.Create(&resource)
	if result.Error != nil {
		return result.Error
	}

	fmt.Println("Data uploaded to database successfully")
	return nil
}

// Initialize the database connection
func initDB() {
	var err error
	db, err = gorm.Open(sqlite.Open("resources.db"), &gorm.Config{})
	if err != nil {
		log.Fatalf("Failed to connect to the database: %v", err)
	}

	// Auto migrate the Resource model
	err = db.AutoMigrate(&Resource{})
	if err != nil {
		log.Fatalf("Failed to migrate database: %v", err)
	}
}

func main() {
	// Initialize the database
	initDB()

	// MQTT client options
	opts := mq.NewClientOptions().AddBroker("tcp://localhost:1883")
	opts.SetClientID("JetsonSubscriber")
	opts.SetCleanSession(true)

	// Create MQTT client
	client := mq.NewClient(opts)

	// Connect to MQTT broker
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		log.Fatalf("Failed to connect to MQTT broker: %v", token.Error())
	}

	// Subscribe to the topic
	topic := "gr25/jetson/tcm/resource_data" // Update with your topic
	if token := client.Subscribe(topic, 0, onMessageReceived); token.Wait() && token.Error() != nil {
		log.Fatalf("Failed to subscribe to topic: %v", token.Error())
	}

	// Block forever
	select {}
}
