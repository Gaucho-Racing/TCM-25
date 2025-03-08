package utils

import "monitor/config"

func VerifyConfig() {
	if config.VehicleID == "" {
		SugarLogger.Fatalln("VEHICLE_ID must be set")
	}
	if config.Port == "" {
		config.Port = "9999"
		SugarLogger.Infof("PORT is not set, defaulting to %s", config.Port)
	}
	if config.DatabaseHost == "" {
		config.DatabaseHost = "localhost"
		SugarLogger.Infof("DATABASE_HOST is not set, defaulting to %s", config.DatabaseHost)
	}
	if config.DatabasePort == "" {
		config.DatabasePort = "5432"
		SugarLogger.Infof("DATABASE_PORT is not set, defaulting to %s", config.DatabasePort)
	}
	if config.DatabaseUser == "" {
		config.DatabaseUser = "gr25"
		SugarLogger.Infof("DATABASE_USER is not set, defaulting to %s", config.DatabaseUser)
	}
	if config.DatabasePassword == "" {
		config.DatabasePassword = "gr25"
		SugarLogger.Infof("DATABASE_PASSWORD is not set, defaulting to %s", config.DatabasePassword)
	}
	if config.PingInterval == "" {
		config.PingInterval = "5000"
		SugarLogger.Infof("PING_INTERVAL is not set, defaulting to %s", config.PingInterval)
	}
}
