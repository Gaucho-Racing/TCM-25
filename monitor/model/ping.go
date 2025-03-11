package model

type Ping struct {
	Ping    int `json:"ping"`
	Pong    int `json:"pong"`
	Latency int `json:"latency"`
}

func (Ping) TableName() string {
	return "ping"
}
