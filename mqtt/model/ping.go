package model

// Since we are dealing with a local database (on-vehicle), we know that
// the ping millis will always be unique for that car.
type Ping struct {
	VehicleID string `json:"vehicle_id"`
	Ping      int    `json:"ping" gorm:"primaryKey"`
	Pong      int    `json:"pong"`
	Latency   int    `json:"latency"`
}

func (Ping) TableName() string {
	return "ping"
}
