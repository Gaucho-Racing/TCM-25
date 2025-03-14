package main

import (
	"monitor/api"
	"monitor/config"
	"monitor/database"
	"monitor/mqtt"
	"monitor/service"
	"monitor/utils"
)

func main() {
	config.PrintStartupBanner()
	utils.InitializeLogger()
	utils.VerifyConfig()
	defer utils.Logger.Sync()

	database.InitializeDB()
	mqtt.InitializeMQTT()
	service.InitializePings()

	router := api.SetupRouter()
	api.InitializeRoutes(router)
	err := router.Run(":" + config.Port)
	if err != nil {
		utils.SugarLogger.Fatalln(err)
	}
}
