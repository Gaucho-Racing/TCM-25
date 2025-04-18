package service

import (
	"encoding/binary"
	"fmt"
	"mqtt/config"
	"mqtt/database"
	"mqtt/model"
	"mqtt/mqtt"
	"mqtt/utils"
	"strconv"
	"time"

	mq "github.com/eclipse/paho.mqtt.golang"
)

func InitializePings() {
	SubscribePong()
	interval, err := strconv.Atoi(config.PingInterval)
	if err != nil {
		utils.SugarLogger.Errorln("Failed to convert ping interval, using 5000ms", err)
		interval = 5000
	}
	go func() {
		for {
			PublishPing()
			time.Sleep(time.Duration(interval) * time.Millisecond)
		}
	}()
	go func() {
		for {
			lastPing := FindLastSuccessfulPing()
			if time.Now().UnixMilli()-int64(lastPing.Ping) > int64(interval*2) {
				utils.SugarLogger.Warnf("Last successful ping was %.2fs ago", float64(time.Now().UnixMilli()-int64(lastPing.Ping))/1000)
			}
			time.Sleep(time.Duration(2345) * time.Millisecond)
		}
	}()
}

func SubscribePong() {
	topic := fmt.Sprintf("gr25/%s/tcm/pong", config.VehicleID)
	mqtt.Subscribe(topic, func(client mq.Client, msg mq.Message) {
		ping := binary.BigEndian.Uint64(msg.Payload()[:8])
		pong := binary.BigEndian.Uint64(msg.Payload()[8:])
		received := time.Now().UnixMilli()
		uploadLatency := time.Now().UnixMilli() - int64(ping)
		rtt := received - int64(ping)

		go UpdatePong(int(ping), int(pong), int(uploadLatency))
		utils.SugarLogger.Infof("[MQ] Received pong in %d ms", rtt)
	})
}

func PublishPing() {
	topic := fmt.Sprintf("gr25/%s/tcm/ping", config.VehicleID)
	millis := time.Now().UnixMilli()
	go CreatePing(int(millis))
	millisBytes := make([]byte, 8)
	binary.BigEndian.PutUint64(millisBytes, uint64(millis))
	uploadKey := []byte{0x01, 0x01}
	payload := append(millisBytes, uploadKey...)
	token := mqtt.Client.Publish(topic, 0, false, payload)
	timeout := token.WaitTimeout(time.Second * 10)
	if !timeout {
		utils.SugarLogger.Errorln("Failed to publish ping: noreply 10s")
	} else if token.Error() != nil {
		utils.SugarLogger.Errorln("Failed to publish ping:", token.Error())
	}
}

func CreatePing(ping int) {
	result := database.DB.Create(&model.Ping{
		VehicleID: config.VehicleID,
		Ping:      ping,
	})
	if result.Error != nil {
		utils.SugarLogger.Errorln("Failed to create ping:", result.Error)
	}
}

func UpdatePong(ping int, pong int, latency int) {
	result := database.DB.Model(&model.Ping{}).Where("ping = ?", ping).Update("pong", pong).Update("latency", latency)
	if result.Error != nil {
		utils.SugarLogger.Errorln("Failed to update pong:", result.Error)
	}
}

func FindLastSuccessfulPing() model.Ping {
	var ping model.Ping
	database.DB.Where("latency > 0").Order("ping DESC").First(&ping)
	return ping
}
