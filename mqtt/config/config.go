package config

import (
	"log"
	"os"

	"github.com/joho/godotenv"
)

var (
	Version          string
	Env              string
	Port             string
	VehicleID        string
	DatabaseHost     string
	DatabasePort     string
	DatabaseUser     string
	DatabasePassword string
	DatabaseName     string
	MQTTHost         string
	MQTTPort         string
	MQTTUser         string
	MQTTPassword     string
	PingInterval     string
)

func init() {
	if err := godotenv.Load(); err != nil {
		log.Println("⚠️  Error loading .env file:", err)
	}

	Version = "1.1.0"
	Env = os.Getenv("ENV")
	Port = os.Getenv("PORT")
	VehicleID = os.Getenv("VEHICLE_ID")
	DatabaseHost = os.Getenv("DATABASE_HOST")
	DatabasePort = os.Getenv("DATABASE_PORT")
	DatabaseUser = os.Getenv("DATABASE_USER")
	DatabasePassword = os.Getenv("DATABASE_PASSWORD")
	DatabaseName = os.Getenv("DATABASE_NAME")
	MQTTHost = os.Getenv("MQTT_HOST")
	MQTTPort = os.Getenv("MQTT_PORT")
	MQTTUser = os.Getenv("MQTT_USER")
	MQTTPassword = os.Getenv("MQTT_PASSWORD")
	PingInterval = os.Getenv("PING_INTERVAL")
}
