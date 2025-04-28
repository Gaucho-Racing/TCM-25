package model

type Gr25 struct {
	Timestamp int64  `json:"timestamp" gorm:"primaryKey;column:timestamp"` // milliseconds since epoch
	Topic     string `json:"topic" gorm:"type:text"`
	Data      []byte `json:"data" gorm:"type:bytea"` // PostgreSQL uses BYTEA for binary data
	Synced    int    `json:"synced"`
	Source    string `json:"source"`
	Target    string `json:"target"`
}

func (Gr25) TableName() string {
	return "gr25"
}
