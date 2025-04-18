package service

import (
	"encoding/binary"
	"fmt"
	"math"
	"monitor/config"
	"monitor/mqtt"
	"os/exec"
	"regexp"
	"time"
)

var CPU_util int        //Percent of CPU up-time in use
var GPU_util int        //Percent of GPU up-time in use
var memory_util int     //Percent of RAM space in use
var storage_util int    //Percent of hard drive space in use
var power_usage float32 //Average in mW (milliWatts)
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
			time.Sleep(1 * time.Second)
		}
	}()
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

func PublishJetsonResources() {
	topic := fmt.Sprintf("gr25/%s/jetson/resources", config.VehicleID)

	// Create 24-byte buffer
	payload := make([]byte, 24)

	// Encode timestamp (8 bytes)
	timestamp := time.Now().UnixMilli()
	binary.BigEndian.PutUint64(payload[0:8], uint64(timestamp))

	// Encode resource usage metrics (4 bytes: 1 each)
	payload[8] = uint8(CPU_util)
	payload[9] = uint8(GPU_util)
	payload[10] = uint8(memory_util)
	payload[11] = uint8(storage_util)

	// Encode float32 values (12 bytes: 4 each)
	binary.BigEndian.PutUint32(payload[12:16], math.Float32bits(power_usage))
	binary.BigEndian.PutUint32(payload[16:20], math.Float32bits(CPU_temp))
	binary.BigEndian.PutUint32(payload[20:24], math.Float32bits(GPU_temp))

	// Publish to MQTT
	token := mqtt.Client.Publish(topic, 0, false, payload)
	if token.Wait() && token.Error() != nil {
		fmt.Println("Failed to publish Jetson stats:", token.Error())
	} else {
		fmt.Println("Published Jetson stats successfully.")
	}
}
