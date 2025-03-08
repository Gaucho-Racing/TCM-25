package mqtt

import (
	"fmt"
	"monitor/config"
	"monitor/utils"

	mq "github.com/eclipse/paho.mqtt.golang"
)

var Client mq.Client

func InitializeMQTT() {
	opts := mq.NewClientOptions()
	opts.AddBroker(fmt.Sprintf("tcp://%s:%s", config.MQTTHost, config.MQTTPort))
	opts.SetUsername(config.MQTTUser)
	opts.SetPassword(config.MQTTPassword)
	opts.SetAutoReconnect(true)
	opts.SetClientID(fmt.Sprintf("gr25-tcm-monitor"))
	opts.SetOnConnectHandler(onConnect)
	opts.SetConnectionLostHandler(onConnectionLost)
	opts.SetReconnectingHandler(onReconnect)
	Client = mq.NewClient(opts)
	if token := Client.Connect(); token.Wait() && token.Error() != nil {
		utils.SugarLogger.Fatalln("[MQ] Failed to connect to MQTT", token.Error())
	}
}

func onConnect(client mq.Client) {
	utils.SugarLogger.Infoln("[MQ] Connected to MQTT broker")
}

func onConnectionLost(client mq.Client, err error) {
	utils.SugarLogger.Errorln("[MQ] Connection lost to MQTT broker:", err)
}

func onReconnect(client mq.Client, opts *mq.ClientOptions) {
	utils.SugarLogger.Infoln("[MQ] Reconnecting to MQTT broker...")
}
