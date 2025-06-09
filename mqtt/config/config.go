package config

import (
	"os"

	cmap "github.com/orcaman/concurrent-map/v2"
)

var Version = "1.4.1"
var Env = os.Getenv("ENV")
var Port = os.Getenv("PORT")

var VehicleID = os.Getenv("VEHICLE_ID")
var VehicleUploadKeyString = os.Getenv("VEHICLE_UPLOAD_KEY")
var VehicleUploadKey uint16

var DatabaseHost = os.Getenv("DATABASE_HOST")
var DatabasePort = os.Getenv("DATABASE_PORT")
var DatabaseUser = os.Getenv("DATABASE_USER")
var DatabasePassword = os.Getenv("DATABASE_PASSWORD")
var DatabaseName = os.Getenv("DATABASE_NAME")

var MQTTHost = os.Getenv("MQTT_HOST")
var MQTTPort = os.Getenv("MQTT_PORT")
var MQTTUser = os.Getenv("MQTT_USER")
var MQTTPassword = os.Getenv("MQTT_PASSWORD")

var CANPort = os.Getenv("CAN_PORT")

var LastSucessfulPublish = cmap.ConcurrentMap[string, uint64]{}
