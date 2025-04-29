package mqtt

import (
	"fmt"
	"monitor/config"
	"monitor/utils"

	mq "github.com/eclipse/paho.mqtt.golang"
)

var Client mq.Client

var subscribedTopics = make(map[string]mq.MessageHandler)

func InitializeMQTT() {
	opts := mq.NewClientOptions()
	opts.AddBroker(fmt.Sprintf("tcp://%s:%s", config.MQTTHost, config.MQTTPort))
	opts.SetUsername(config.MQTTUser)
	opts.SetPassword(config.MQTTPassword)
	opts.SetAutoReconnect(true)
	opts.SetClientID(fmt.Sprintf("gr25-tcm-monitor-%s", config.Env))
	opts.SetOnConnectHandler(onConnect)
	opts.SetConnectionLostHandler(onConnectionLost)
	opts.SetReconnectingHandler(onReconnect)
	Client = mq.NewClient(opts)
	if token := Client.Connect(); token.Wait() && token.Error() != nil {
		utils.SugarLogger.Fatalln("[MQ] Failed to connect to MQTT", token.Error())
	}
}

func Subscribe(topic string, handler mq.MessageHandler) {
	if token := Client.Subscribe(topic, 0, handler); token.Wait() && token.Error() != nil {
		utils.SugarLogger.Errorln("[MQ] Failed to subscribe to topic:", topic, token.Error())
		return
	}
	subscribedTopics[topic] = handler
	utils.SugarLogger.Infoln("[MQ] Subscribed to topic:", topic)
}

func onConnect(client mq.Client) {
	utils.SugarLogger.Infoln("[MQ] Connected to MQTT broker")

	for topic, handler := range subscribedTopics {
		if token := Client.Subscribe(topic, 0, handler); token.Wait() && token.Error() != nil {
			utils.SugarLogger.Errorln("[MQ] Failed to resubscribe to topic:", topic, token.Error())
			continue
		}
		utils.SugarLogger.Infoln("[MQ] Resubscribed to topic:", topic)
	}
}

func onConnectionLost(client mq.Client, err error) {
	utils.SugarLogger.Errorln("[MQ] Connection lost to MQTT broker:", err)
}

func onReconnect(client mq.Client, opts *mq.ClientOptions) {
	utils.SugarLogger.Infoln("[MQ] Reconnecting to MQTT broker...")
}
