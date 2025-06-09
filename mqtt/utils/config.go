package utils

import (
	"mqtt/config"
	"strconv"
)

func VerifyConfig() {
	if config.VehicleID == "" {
		SugarLogger.Fatalln("VEHICLE_ID is not set")
	}
	key, err := strconv.Atoi(config.VehicleUploadKeyString)
	if err != nil {
		SugarLogger.Fatalln("VEHICLE_UPLOAD_KEY is not a number")
	}
	if key < 0 || key > 65535 {
		SugarLogger.Fatalln("VEHICLE_UPLOAD_KEY is not a valid unsigned 16-bit integer")
	}
	config.VehicleUploadKey = uint16(key)
	publishInterval, err := strconv.Atoi(config.PublishInterval)
	if err != nil {
		SugarLogger.Errorf("PUBLISH_INTERVAL is not a number, using 100ms: %v", err)
		publishInterval = 100
	}
	config.PublishIntervalInt = publishInterval

	SugarLogger.Infof("Vehicle ID: %s", config.VehicleID)
	SugarLogger.Infof("Vehicle Upload Key: %d", config.VehicleUploadKey)
}
