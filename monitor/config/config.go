package config

import (
	"os"
)

var Version = "1.4.6"
var Env = os.Getenv("ENV")

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

var PingInterval = os.Getenv("PING_INTERVAL")
