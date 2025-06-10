package model

type ResourceMetrics struct {
	CPU0Freq     int `json:"cpu_0_freq"`     // 2 bytes
	CPU0Util     int `json:"cpu_0_util"`     // 1 byte
	CPU1Freq     int `json:"cpu_1_freq"`     // 2 bytes
	CPU1Util     int `json:"cpu_1_util"`     // 1 byte
	CPU2Freq     int `json:"cpu_2_freq"`     // 2 bytes
	CPU2Util     int `json:"cpu_2_util"`     // 1 byte
	CPU3Freq     int `json:"cpu_3_freq"`     // 2 bytes
	CPU3Util     int `json:"cpu_3_util"`     // 1 byte
	CPU4Freq     int `json:"cpu_4_freq"`     // 2 bytes
	CPU4Util     int `json:"cpu_4_util"`     // 1 byte
	CPU5Freq     int `json:"cpu_5_freq"`     // 2 bytes
	CPU5Util     int `json:"cpu_5_util"`     // 1 byte
	CPUTotalUtil int `json:"cpu_total_util"` // 1 byte
	RAMTotal     int `json:"ram_total"`      // 2 bytes
	RAMUsed      int `json:"ram_used"`       // 2 bytes
	RAMUtil      int `json:"ram_util"`       // 1 byte
	GPUUtil      int `json:"gpu_util"`       // 1 byte
	GPUFreq      int `json:"gpu_freq"`       // 2 bytes
	DiskTotal    int `json:"disk_total"`     // 4 bytes
	DiskUsed     int `json:"disk_used"`      // 4 bytes
	DiskUtil     int `json:"disk_util"`      // 1 byte
	CPUTemp      int `json:"cpu_temp"`       // 1 byte
	GPUTemp      int `json:"gpu_temp"`       // 1 byte
	VoltageDraw  int `json:"voltage_draw"`   // 2 bytes
	CurrentDraw  int `json:"current_draw"`   // 2 bytes
	PowerDraw    int `json:"power_draw"`     // 2 bytes
}
