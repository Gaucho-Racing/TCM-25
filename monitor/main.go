package main

import (
	"monitor/config"
	"monitor/database"
	"monitor/mqtt"
	"monitor/service"
	"monitor/utils"
)

func main() {
	config.PrintStartupBanner()
	utils.InitializeLogger()
	defer utils.Logger.Sync()

	utils.VerifyConfig()
	database.InitializeDB()
	mqtt.InitializeMQTT()
	service.InitializePings()
	service.InitializeResourceQuery()

	for {
	}
}
