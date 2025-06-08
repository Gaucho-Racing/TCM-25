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

	database.InitializeDB()
	mqtt.InitializeMQTT()
	service.InitializePings()
	service.InitializeResourceQuery()

	for {
	}
}
