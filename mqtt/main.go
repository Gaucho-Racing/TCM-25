package main

import (
	"mqtt/config"
	"mqtt/database"
	"mqtt/mqtt"
	"mqtt/service"
	"mqtt/utils"
)

func main() {
	config.PrintStartupBanner()
	utils.InitializeLogger()
	defer utils.Logger.Sync()
	
	utils.VerifyConfig()
	database.InitializeDB()
	mqtt.InitializeMQTT()
	service.ListenCAN(config.CANPort)
}
