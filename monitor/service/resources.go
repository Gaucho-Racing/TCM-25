package service

import (
	"fmt"
	"os/exec"
	"regexp"
	"strings"
	"time"
)

var CPU_util int //Percent of CPU up-time in use
var GPU_util int //Percent of GPU up-time in use
var memory_util int //Percent of RAM space in use
var storage_util int //Percent of hard drive space in use
var power_usage float32 //Average in mW (milliWatts)
var CPU_temp float32 //CPU temp in Celsius
var GPU_temp float32 //GPU temp in Celsius
var stats_output

/*

*/
/*
Initializes necessary setup and threaded functions for querying and parsing Jetson resource information.
*/
func InitializeResourceQuery() {

}

/*
Runs the shell command 'tegrastats' which outputs a large aggregate of CPU, GPU, and Memory statistics.

Statistics formatting:
https://docs.nvidia.com/jetson/archives/r35.1/DeveloperGuide/text/AT/JetsonLinuxDevelopmentTools/TegrastatsUtility.html
*/
func QueryResourceStatistics(error) {
	tegrastats := exec.Command("tegrastats", "--interval", "1000", "--count", "1")
	stats_output, err := tegrastats.Output()
	if err != nil {
		return err
	}
}

/*
Parses the resource statistics for CPU utilization and saves to CPU_util.
*/
func GetCPUUtil(error) {
	re := regexp.MustCompile(`CPU \[(.*?)\]`)
	CPU_block_match := re.FindStringSubmatch(string(stats_output))
	if len(CPU_block_match) < 2 {
		return fmt.Errorf("CPU utilization block not found.")
	}

	CPU_block = CPU_block_match[1]
	CPU_re := regexp.MustCompile(`(\d+)%`)
	CPU_matches := GPU_re.FindAllStringSubmatch(CPU_block, -1)

	// var CPU_percents []int
	var count int = 0
	for _, match := range CPU_matches {
		var value int
		fmt.Sscanf(match[1], "%d", &value)
		count += 1
		CPU_util := ((count * CPU_util) + value) / (count + 1)
		// CPU_percents = append(CPU_percents, value)
	}

	return nil
}

/*
Parses the resource statistics for GPU utilization and saves to GPU_util.
*/
func GetGPUUtil(error) {
	re := regexp.MustCompile(`GR3D_FREQ (\d+)%`)
	GPU_util_match := re.FindStringSubmatch(string(stats_output))
	if len(matches) < 2 {
		return fmt.Errorf("GPU utilization (GR3D_FREQ) not found.")
	}

	fmt.Sscanf(matches[1], "%d", &GPU_util)

	return nil
}

/*
Parses the resource statistics for RAM (memory) utilization and saves to memory_util.
*/
func GetMemoryUtil(error) {
	re := regexp.MustCompile(`RAM (\d+)/(\d+)MB`)
	memory_util_matches := re.FindStringSubmatch(string(stats_output))
	if len(memory_util_matches) < 3 {
		return fmt.Errorf("RAM usage not found.")
	}

	var RAM_in_use, RAM_total int
	fmt.Sscanf(matches[1], "%d", &usedRAM)
	fmt.Sscanf(matches[2], "%d", &totalRAM)

	memory_util := int((RAM_in_use / RAM_total) * 100)

	return nil
}

/*
Parses the resource statistics for power usage statistics and saves to power_usage.
*/
func GetPowerUsage(error) {
	re := regexp.MustCompile(`VDDR\? \d+/(\d+)`)
	power_usage_match := re.FindStringSubmatch(string(stats_output))
	if len(power_usage_match) < 2 {
		return fmt.Errorf("Power usage not found.")
	}

	fmt.Sscanf(matches[1], "%d", &power_usage)

	return nil
}

/*
Parses the resource statistics for CPU temperature in Celsius and saves to CPU_temp.
*/
func GetCPUTemp(error) {
	re := regexp.MustCompile(`CPU@(\d+\.\d+)C`)
	CPU_temp_match := re.FindStringSubmatch(string(stats_output))
	if len(CPU_temp_match) < 2 {
		return fmt.Errorf("CPU temperature not found.")
	}

	fmt.Sscanf(matches[1], "%d", &CPU_temp)

	return nil
}

/*
Parses the resource statistics for GPU temperature in Celsius and saves to GPU_temp.
*/
func GetGPUTemp(error) {
	re := regexp.MustCompile(`CPU@(\d+\.\d+)C`)
	GPU_temp_match := re.FindStringSubmatch(string(stats_output))
	if len(GPU_temp_match) < 2 {
		return fmt.Errorf("CPU temperature not found.")
	}

	fmt.Sscanf(matches[1], "%d", &GPU_temp)

	return nil
}