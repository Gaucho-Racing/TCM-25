package model

type Gr25Message struct {
	Timestamp  int    `json:"timestamp"`
	VehicleID  string `json:"vehicle_id"`
	Topic      string `json:"topic"`
	Data       []byte `json:"data" gorm:"type:bytea"` // PostgreSQL uses BYTEA for binary data
	Synced     int    `json:"synced"`
	SourceNode string `json:"source_node"`
	TargetNode string `json:"target_node"`
}

func (Gr25Message) TableName() string {
	return "gr25_message"
}
