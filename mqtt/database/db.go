package database

import (
	"fmt"
	"mqtt/config"
	"mqtt/model"
	"mqtt/utils"
	"time"

	cmap "github.com/orcaman/concurrent-map/v2"
	"gorm.io/driver/postgres"
	"gorm.io/gorm"
)

var DB *gorm.DB
var dbRetries = 0

func InitializeDB() error {
	dsn := fmt.Sprintf(
		"host=%s user=%s password=%s dbname=%s port=%s sslmode=disable TimeZone=UTC",
		config.DatabaseHost,
		config.DatabaseUser,
		config.DatabasePassword,
		config.DatabaseName,
		config.DatabasePort,
	)

	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{})
	if err != nil {
		if dbRetries < 5 {
			dbRetries++
			utils.SugarLogger.Errorln("failed to connect database, retrying in 5s... ")
			time.Sleep(time.Second * 5)
			return InitializeDB()
		}
		return fmt.Errorf("failed to connect database after 5 attempts")
	}

	utils.SugarLogger.Infoln("[DB] Connected to database")

	if err := db.AutoMigrate(&model.Gr25Message{}); err != nil {
		utils.SugarLogger.Errorln("[DB] AutoMigration failed:", err)
		return err
	}

	utils.SugarLogger.Infoln("[DB] AutoMigration complete")
	DB = db
	return nil
}

func InitializeMap() {
	config.LastSucessfulPublish = cmap.New[uint64]()
}
