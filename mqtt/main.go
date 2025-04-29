package main

import (
	"mqtt/api"
	"mqtt/config"
	"mqtt/database"
	"mqtt/mqtt"
	"mqtt/publisher"
	"mqtt/utils"
)

func main() {
	config.PrintStartupBanner()
	utils.InitializeLogger()
	defer utils.Logger.Sync()

	database.InitializeDB()
	mqtt.InitializeMQTT()
	//mqtt.StartSyncSubscriber(vehicleID)
	publisher.StartUDPServer(5000)

	router := api.SetupRouter()
	api.InitializeRoutes(router)
	err := router.Run(":" + config.Port)
	if err != nil {
		utils.SugarLogger.Fatalln(err)
	}
}
