package model

type ResourceMetrics struct {
	CPU0Freq     int `json:"cpu_0_freq"`
	CPU0Util     int `json:"cpu_0_util"`
	CPU1Freq     int `json:"cpu_1_freq"`
	CPU1Util     int `json:"cpu_1_util"`
	CPU2Freq     int `json:"cpu_2_freq"`
	CPU2Util     int `json:"cpu_2_util"`
	CPU3Freq     int `json:"cpu_3_freq"`
	CPU3Util     int `json:"cpu_3_util"`
	CPU4Freq     int `json:"cpu_4_freq"`
	CPU4Util     int `json:"cpu_4_util"`
	CPU5Freq     int `json:"cpu_5_freq"`
	CPU5Util     int `json:"cpu_5_util"`
	CPUTotalUtil int `json:"cpu_total_util"`
	RAMTotal     int `json:"ram_total"`
	RAMUsed      int `json:"ram_used"`
	RAMUtil      int `json:"ram_util"`
	GPUUtil      int `json:"gpu_util"`
	GPUFreq      int `json:"gpu_freq"`
	DiskTotal    int `json:"disk_total"`
	DiskUsed     int `json:"disk_used"`
	DiskUtil     int `json:"disk_util"`
	CPUTemp      int `json:"cpu_temp"`
	GPUTemp      int `json:"gpu_temp"`
	VoltageDraw  int `json:"voltage_draw"`
	CurrentDraw  int `json:"current_draw"`
	PowerDraw    int `json:"power_draw"`
}
