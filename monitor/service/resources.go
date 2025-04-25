package service

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"monitor/config"
	"monitor/mqtt"
	"monitor/utils"
	"os/exec"
	"regexp"
	"strings"
	"time"
)

var CPU_util int        //Percent of CPU up-time in use
var GPU_util int        //Percent of GPU up-time in use
var memory_util int     //Percent of RAM space in use
var storage_util int    //Percent of hard drive space in use
var power_usage float32 //Average in mW (milliWatts), converted to deciWatts in MQTT compression
var CPU_temp float32    //CPU temp in Celsius
var GPU_temp float32    //GPU temp in Celsius
var stats_output []byte

/*

 */
/*
Initializes necessary setup and threaded functions for querying and parsing Jetson resource information.
*/
func InitializeResourceQuery() {
	go func() {
		for {
			err := QueryResourceStatistics()
			if err == nil {
				GetCPUUtil()
				GetGPUUtil()
				GetMemoryUtil()
				GetPowerUsage()
				GetCPUTemp()
				GetGPUTemp()
			} else {
				fmt.Println("Error running tegrastats:\n", err)
			}
			PublishResources()
			time.Sleep(5 * time.Second)
		}
	}()
}

/*
This is a helper function that keeps an input value between a min and max (inclusive).
This is used in PublishResources() to keep MQTT data at most 1 byte long.
*/
func clamp(val, min, max int) int {
	if val < min {
		return min
	}
	if val > max {
		return max
	}
	return val
}

/*
This function creates a topic string, grabs the current timestamp, and compiles the binary encoding of all collected resource data.
Then, it publishes the payload to the MQTT topic.
*/
func PublishResources() {
	topic := fmt.Sprintf("gr25/gr25-main/tcm/0x02A", config.VehicleID)

	millis := time.Now().UnixMilli()
	millisBytes := make([]byte, 8)
	binary.BigEndian.PutUint64(millisBytes, uint64(millis))
	uploadKey := []byte{0x01, 0x01}

	data := make([]byte, 7)
	data[0] = byte(clamp(CPU_util, 0, 255))
	data[1] = byte(clamp(GPU_util, 0, 255))
	data[2] = byte(clamp(memory_util, 0, 255))
	data[3] = byte(clamp(storage_util, 0, 255))
	data[4] = byte(clamp(int(power_usage/10), 0, 255)) //mW converted into deciWatts for compression
	data[5] = byte(clamp(int(CPU_temp), 0, 255))
	data[6] = byte(clamp(int(GPU_temp), 0, 255))

	payload := append(millisBytes, uploadKey, data)

	token := mqtt.Client.Publish(topic, 0, false, payload)
	timeout := token.WaitTimeout(time.Second * 10)
	if !timeout {
		utils.SugarLogger.Errorln("Failed to publish ping: noreply 10s")
	} else if token.Error() != nil {
		utils.SugarLogger.Errorln("Failed to publish ping:", token.Error())
	}
}

/*
Queries the jetson nano for its hard drive utilization percentage and stores this in storage_util.
*/
func QueryStorageUtil() (string, error) {
	cmd := exec.Command("df", "-h", "/")
	var out bytes.Buffer
	cmd.Stdout = &out

	if err := cmd.Run(); err != nil {
		return "", fmt.Errorf("failed to run df: %v", err)
	}

	lines := strings.Split(out.String(), "\n")
	if len(lines) < 2 {
		return "", fmt.Errorf("unexpected df output: %v", out.String())
	}

	fields := strings.Fields(lines[1])
	if len(fields) < 5 {
		return "", fmt.Errorf("unexpected df line format: %v", lines[1])
	}

	storage_util = fields[4]

	return nil
}

/*
Runs the shell command 'tegrastats' which outputs a large aggregate of CPU, GPU, and Memory statistics.

Statistics are formatted according to:
https://docs.nvidia.com/jetson/archives/r35.1/DeveloperGuide/text/AT/JetsonLinuxDevelopmentTools/TegrastatsUtility.html
*/
func QueryResourceStatistics() error {
	tegrastats := exec.Command("tegrastats", "--interval", "1000", "--count", "1")
	output, err := tegrastats.Output()
	if err != nil {
		return err
	}

	stats_output = output

	return nil
}

/*
Parses the resource statistics for CPU utilization and saves to CPU_util.
*/
func GetCPUUtil() error {
	re := regexp.MustCompile(`CPU \[(.*?)\]`)
	CPU_block_match := re.FindStringSubmatch(string(stats_output))
	if len(CPU_block_match) < 2 {
		return fmt.Errorf("CPU utilization block not found.")
	}

	CPU_block := CPU_block_match[1]
	CPU_re := regexp.MustCompile(`(\d+)%`)
	CPU_matches := CPU_re.FindAllStringSubmatch(CPU_block, -1)

	// var CPU_percents []int
	var count int = 0
	for _, match := range CPU_matches {
		var value int
		fmt.Sscanf(match[1], "%d", &value)
		count += 1
		CPU_util = ((count * CPU_util) + value) / (count + 1)
		// CPU_percents = append(CPU_percents, value)
	}

	return nil
}

/*
Parses the resource statistics for GPU utilization and saves to GPU_util.
*/
func GetGPUUtil() error {
	re := regexp.MustCompile(`GR3D_FREQ (\d+)%`)
	GPU_util_match := re.FindStringSubmatch(string(stats_output))
	if len(GPU_util_match) < 2 {
		return fmt.Errorf("GPU utilization (GR3D_FREQ) not found.")
	}

	fmt.Sscanf(GPU_util_match[1], "%d", &GPU_util)

	return nil
}

/*
Parses the resource statistics for RAM (memory) utilization and saves to memory_util.
*/
func GetMemoryUtil() error {
	re := regexp.MustCompile(`RAM (\d+)/(\d+)MB`)
	memory_util_matches := re.FindStringSubmatch(string(stats_output))
	if len(memory_util_matches) < 3 {
		return fmt.Errorf("RAM usage not found.")
	}

	var RAM_in_use, RAM_total int
	fmt.Sscanf(memory_util_matches[1], "%d", &RAM_in_use)
	fmt.Sscanf(memory_util_matches[2], "%d", &RAM_total)

	memory_util = int((RAM_in_use / RAM_total) * 100)

	return nil
}

/*
Parses the resource statistics for power usage statistics and saves to power_usage.
*/
func GetPowerUsage() error {
	re := regexp.MustCompile(`VDDR\? \d+/(\d+)`)
	power_usage_match := re.FindStringSubmatch(string(stats_output))
	if len(power_usage_match) < 2 {
		return fmt.Errorf("Power usage not found.")
	}

	fmt.Sscanf(power_usage_match[1], "%d", &power_usage)

	return nil
}

/*
Parses the resource statistics for CPU temperature in Celsius and saves to CPU_temp.
*/
func GetCPUTemp() error {
	re := regexp.MustCompile(`CPU@(\d+\.\d+)C`)
	CPU_temp_match := re.FindStringSubmatch(string(stats_output))
	if len(CPU_temp_match) < 2 {
		return fmt.Errorf("CPU temperature not found.")
	}

	fmt.Sscanf(CPU_temp_match[1], "%d", &CPU_temp)

	return nil
}

/*
Parses the resource statistics for GPU temperature in Celsius and saves to GPU_temp.
*/
func GetGPUTemp() error {
	re := regexp.MustCompile(`CPU@(\d+\.\d+)C`)
	GPU_temp_match := re.FindStringSubmatch(string(stats_output))
	if len(GPU_temp_match) < 2 {
		return fmt.Errorf("CPU temperature not found.")
	}

	fmt.Sscanf(GPU_temp_match[1], "%d", &GPU_temp)

	return nil
}
