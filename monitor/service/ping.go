package service

import (
	"encoding/binary"
	"fmt"
	"monitor/config"
	"monitor/database"
	"monitor/model"
	"monitor/mqtt"
	"monitor/utils"
	"time"

	mq "github.com/eclipse/paho.mqtt.golang"
)

func SubscribePong() {
	topic := fmt.Sprintf("gr25/%s/tcm/pong", config.VehicleID)
	token := mqtt.Client.Subscribe(topic, 0, func(client mq.Client, msg mq.Message) {
		ping := binary.BigEndian.Uint64(msg.Payload()[:8])
		pong := binary.BigEndian.Uint64(msg.Payload()[8:])
		received := time.Now().UnixMilli()
		uploadLatency := time.Now().UnixMilli() - int64(ping)
		rtt := received - int64(ping)

		go UpdatePong(int(ping), int(pong), int(uploadLatency))
		utils.SugarLogger.Infof("[MQ] Received pong in %d ms", rtt)
	})
	token.Wait()
	if token.Error() != nil {
		utils.SugarLogger.Errorln("Failed to subscribe to pong:", token.Error())
	}
}

func PublishPing() {
	topic := fmt.Sprintf("gr25/%s/tcm/ping", config.VehicleID)
	millis := time.Now().UnixMilli()
	go CreatePing(int(millis))
	millisBytes := make([]byte, 8)
	binary.BigEndian.PutUint64(millisBytes, uint64(millis))
	uploadKey := []byte{0x01, 0x01}
	payload := append(uploadKey, millisBytes...)
	token := mqtt.Client.Publish(topic, 0, false, payload)
	timeout := token.WaitTimeout(time.Second * 10)
	if timeout {
		utils.SugarLogger.Errorln("Failed to publish ping: noreply 10s")
	} else if token.Error() != nil {
		utils.SugarLogger.Errorln("Failed to publish ping:", token.Error())
	}
}

func CreatePing(ping int) {
	result := database.DB.Create(&model.Ping{
		Ping: ping,
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
