package main

import (
	"fmt"
	"time"

	mq "github.com/eclipse/paho.mqtt.golang"
)


var Client mq.Client

const username = "a"
const password = "69420"
const broker = "tcp://127.0.0.1:1883"
// db and table name?
var uploadKey = [2]byte{69, 69}

func getTimestamp() int64 { // int?
	return time.Now().Unix() / int64(time.Millisecond)
}

// func connectDB() {
// 		to add? 
// }

func connectMQTT() {
	opts := mq.NewClientOptions()
	opts.SetUsername(username)
	opts.SetPassword(password)
	opts.SetAutoReconnect(true)
	opts.SetCleanSession(false)
	opts.SetKeepAlive(1)
	opts.AddBroker(broker)

	Client = mq.NewClient(opts)
	if token := Client.Connect(); token.Wait() && token.Error() != nil {
		fmt.Println("error") // change to log/proper error msg
	}

}

func disconnect() {
	Client.Disconnect(250) // catch if connected first?
	//disconnect pg db?
}

func publishData(nodeID string, messageID string, arr []byte) { 
=	topic := "gr25/gr25-main/" + nodeID + "/" + messageID

	timestamp := getTimestamp()

	size := 10 + len(arr)
	m := make([]byte, size)

	for i := 0; i < 8; i++ {
		m[7-i] = byte(timestamp >> (i * 8) & 0xFF)
	}
	m[8] = uploadKey[0]
	m[9] = uploadKey[1]

	for i := 10; i < size; i++ {
		m[i] = arr[i-10]
	}

	Client.Publish(topic, 1, true, m)
	// add error catching and reconnecting(idek if even needed) 
	// add database storing 
}

// test function

func main() {
	fmt.Println("Hello, World!")
	connectMQTT()
	disconnect()
}
